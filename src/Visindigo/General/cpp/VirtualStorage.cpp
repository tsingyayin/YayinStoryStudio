#include "General/VirtualStorage.h"
#include <QtSql/qsqldatabase.h>
#include <QtSql/qsqlquery.h>
#include <QtSql/qsqlerror.h>
#include <QtCore/qcryptographichash.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qmap.h>
#include "General/Log.h"

namespace Visindigo::Utility {

	class VirtualStoragePrivate {
		friend class VirtualStorage;
	private:
		QString dbPath;
		QSqlDatabase db;

		static const QString connectionPrefix;
		static const QChar   vpthDelimiter;

		QString computeMD5(const QByteArray& data) {
			return QString::fromUtf8(
				QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex()
			);
		}

		QString formatVID(const QString& md5, qint32 rpIndex) {
			return md5 + QLatin1Char('-')
				+ QString::number(rpIndex, 16).rightJustified(4, QLatin1Char('0'));
		}

		QPair<QString, qint32> parseVID(const QString& vid) {
			qint32 dashPos = vid.lastIndexOf(QLatin1Char('-'));
			if (dashPos < 0) {
				return { QString(), -1 };
			}
			QString md5 = vid.left(dashPos);
			bool ok = false;
			qint32 rpIndex = vid.mid(dashPos + 1).toInt(&ok, 16);
			if (!ok) {
				return { QString(), -1 };
			}
			return { md5, rpIndex };
		}

		bool openDatabase() {
			if (db.isOpen()) {
				return true;
			}
			db = QSqlDatabase::addDatabase("QSQLITE", connectionPrefix + dbPath);
			db.setDatabaseName(dbPath);
			if (!db.open()) {
				vgError << "VirtualStorage: Failed to open database: "
					<< db.lastError().text();
				return false;
			}
			return initTables();
		}

		bool initTables() {
			QSqlQuery q(db);
			q.exec("PRAGMA encoding = \"UTF-8\"");

			bool ok = q.exec(
				"CREATE TABLE IF NOT EXISTS vpth_map ("
				"  idx INTEGER PRIMARY KEY AUTOINCREMENT,"
				"  vpth TEXT NOT NULL UNIQUE,"
				"  md5  TEXT NOT NULL,"
				"  rp_index INTEGER NOT NULL"
				")"
			);
			if (!ok) {
				vgError << "VirtualStorage: Failed to create vpth_map: "
					<< q.lastError().text();
				return false;
			}

			ok = q.exec(
				"CREATE TABLE IF NOT EXISTS vfile_data ("
				"  idx INTEGER PRIMARY KEY AUTOINCREMENT,"
				"  md5  TEXT NOT NULL,"
				"  rp_index INTEGER NOT NULL,"
				"  vpth TEXT NOT NULL,"
				"  size INTEGER NOT NULL,"
				"  data BLOB NOT NULL"
				")"
			);
			if (!ok) {
				vgError << "VirtualStorage: Failed to create vfile_data: "
					<< q.lastError().text();
				return false;
			}

			ok = q.exec(
				"CREATE INDEX IF NOT EXISTS idx_vfile_data_md5_rp "
				"ON vfile_data(md5, rp_index)"
			);
			if (!ok) {
				vgError << "VirtualStorage: Failed to create index: "
					<< q.lastError().text();
				return false;
			}

			return true;
		}

		/*
			读取 file_data 中指定条目的 vpth 列表（以换行符分隔）。
		*/
		QStringList readVpthList(const QString& md5, qint32 rpIndex) {
			QSqlQuery q(db);
			q.prepare("SELECT vpth FROM vfile_data WHERE md5 = ? AND rp_index = ?");
			q.addBindValue(md5);
			q.addBindValue(rpIndex);
			if (q.exec() && q.next()) {
				return q.value(0).toString().split(vpthDelimiter, Qt::SkipEmptyParts);
			}
			return QStringList();
		}

		/*
			向 vfile_data 条目的 vpth 列表中添加一个虚拟路径。
		*/
		void addVpthToData(const QString& md5, qint32 rpIndex, const QString& vpth) {
			QStringList list = readVpthList(md5, rpIndex);
			if (list.isEmpty()) {
				// 防御性编程：调用方保证此时条目已存在
				return;
			}
			if (list.contains(vpth)) {
				return;
			}
			list.append(vpth);
			QSqlQuery q(db);
			q.prepare("UPDATE vfile_data SET vpth = ? WHERE md5 = ? AND rp_index = ?");
			q.addBindValue(list.join(vpthDelimiter));
			q.addBindValue(md5);
			q.addBindValue(rpIndex);
			q.exec();
		}

		/*
			从 vfile_data 条目的 vpth 列表中移除一个虚拟路径。
			若移除后列表为空，则删除该数据条目。
		*/
		void removeVpthFromData(const QString& md5, qint32 rpIndex, const QString& vpth) {
			QStringList list = readVpthList(md5, rpIndex);
			if (list.isEmpty()) {
				return;
			}
			list.removeAll(vpth);
			if (list.isEmpty()) {
				QSqlQuery q(db);
				q.prepare("DELETE FROM vfile_data WHERE md5 = ? AND rp_index = ?");
				q.addBindValue(md5);
				q.addBindValue(rpIndex);
				q.exec();
			} else {
				QSqlQuery q(db);
				q.prepare("UPDATE vfile_data SET vpth = ? WHERE md5 = ? AND rp_index = ?");
				q.addBindValue(list.join(vpthDelimiter));
				q.addBindValue(md5);
				q.addBindValue(rpIndex);
				q.exec();
			}
		}

		/*
			在 vfile_data 中查找与给定 data 内容完全相同的条目。
			返回匹配的 rp_index，若需新建则返回下一个可用序号。
			若 data 与某条目的 data 完全相同，isDuplicate 置为 true。
		*/
		qint32 resolveRpIndex(const QString& md5, const QByteArray& data, bool& isDuplicate) {
			isDuplicate = false;
			QSqlQuery q(db);
			q.prepare("SELECT rp_index, data FROM vfile_data WHERE md5 = ? ORDER BY rp_index ASC");
			q.addBindValue(md5);
			if (!q.exec()) {
				return 0;
			}

			qint32 maxRp = -1;
			while (q.next()) {
				qint32 rp = q.value(0).toInt();
				if (rp > maxRp) {
					maxRp = rp;
				}
				QByteArray existingData = q.value(1).toByteArray();
				if (existingData == data) {
					isDuplicate = true;
					return rp;
				}
			}
			return maxRp + 1;
		}

		/*
			插入 vfile_data 新条目（仅用于全新数据，不处理 vpth 列表追加）。
		*/
		bool insertFileData(const QString& md5, qint32 rpIndex, const QString& vpth, const QByteArray& data) {
			QSqlQuery q(db);
			q.prepare("INSERT INTO vfile_data (md5, rp_index, vpth, size, data) VALUES (?, ?, ?, ?, ?)");
			q.addBindValue(md5);
			q.addBindValue(rpIndex);
			q.addBindValue(vpth);
			q.addBindValue((qint64)data.size());
			q.addBindValue(data);
			if (!q.exec()) {
				vgError << "VirtualStorage: Failed to insert vfile_data: "
					<< q.lastError().text();
				return false;
			}
			return true;
		}
	};

	const QString VirtualStoragePrivate::connectionPrefix = "VisStorage_";
	const QChar   VirtualStoragePrivate::vpthDelimiter     = QLatin1Char('\n');

	/*!
		\class Visindigo::Utility::VirtualStorage
		\brief 基于SQLite的虚拟文件存储，以MD5去重并按虚拟路径索引。
		\inmodule Visindigo
		\since Visindigo 0.16.0

	*/

	/*!
		\since Visindigo 0.16.0
		\a dbPath SQLite 数据库文件路径。
		
		构造函数，创建一个虚拟存储对象。若数据库文件不存在则自动创建并初始化表结构。
	*/
	VirtualStorage::VirtualStorage(const QString& dbPath) {
		d = new VirtualStoragePrivate();
		d->dbPath = dbPath;
		d->openDatabase();
	}

	VirtualStorage::~VirtualStorage() {
		{
			QString connName = VirtualStoragePrivate::connectionPrefix + d->dbPath;
			if (QSqlDatabase::contains(connName)) {
				QSqlDatabase::database(connName).close();
			}
		}
		QSqlDatabase::removeDatabase(
			VirtualStoragePrivate::connectionPrefix + d->dbPath
		);
		delete d;
	}

	/*!
		保存（或覆盖）虚拟路径 \a path 对应的文件数据。
		若数据内容与已有文件完全相同，将仅建立新的虚拟路径映射而不再存储一份数据。
		若该虚拟路径先前已绑定其他内容，则自动解除旧的绑定并适时清理不再引用的数据条目。
	*/
	void VirtualStorage::saveFile(const QString& path, const QByteArray& data) {
		if (!d->db.isOpen()) {
			return;
		}

		QString newMD5 = d->computeMD5(data);

		QSqlQuery q(d->db);
		bool hadOld = false;
		QString oldMD5;
		qint32 oldRp = -1;

		q.prepare("SELECT md5, rp_index FROM vpth_map WHERE vpth = ?");
		q.addBindValue(path);
		if (q.exec() && q.next()) {
			hadOld = true;
			oldMD5 = q.value(0).toString();
			oldRp = q.value(1).toInt();
		}

		if (hadOld && oldMD5 == newMD5) {
			QSqlQuery ck(d->db);
			ck.prepare("SELECT data FROM vfile_data WHERE md5 = ? AND rp_index = ?");
			ck.addBindValue(oldMD5);
			ck.addBindValue(oldRp);
			if (ck.exec() && ck.next() && ck.value(0).toByteArray() == data) {
				return;
			}
			d->db.transaction();
			q.prepare("DELETE FROM vpth_map WHERE vpth = ?");
			q.addBindValue(path);
			q.exec();
			d->removeVpthFromData(oldMD5, oldRp, path);
			d->db.commit();
			hadOld = false;
		}

		bool isDup = false;
		qint32 newRp = d->resolveRpIndex(newMD5, data, isDup);

		if (hadOld && isDup && oldMD5 == newMD5 && oldRp == newRp) {
			return;
		}

		d->db.transaction();

		if (hadOld) {
			q.prepare("DELETE FROM vpth_map WHERE vpth = ?");
			q.addBindValue(path);
			q.exec();

			if (!(oldMD5 == newMD5 && oldRp == newRp)) {
				d->removeVpthFromData(oldMD5, oldRp, path);
			}
		}

		q.prepare("INSERT INTO vpth_map (vpth, md5, rp_index) VALUES (?, ?, ?)");
		q.addBindValue(path);
		q.addBindValue(newMD5);
		q.addBindValue(newRp);
		if (!q.exec()) {
			vgError << "VirtualStorage: Failed to insert vpth_map: "
				<< q.lastError().text();
			d->db.rollback();
			return;
		}

		if (isDup) {
			d->addVpthToData(newMD5, newRp, path);
		} else {
			d->insertFileData(newMD5, newRp, path, data);
		}

		d->db.commit();
	}

	/*!
		读取虚拟路径 \a path 对应的文件数据。若路径不存在则返回空 QByteArray。
	*/
	QByteArray VirtualStorage::readFile(const QString& path) {
		if (!d->db.isOpen()) {
			return QByteArray();
		}

		QSqlQuery q(d->db);
		q.prepare(
			"SELECT d.data "
			"FROM vpth_map m "
			"JOIN vfile_data d ON m.md5 = d.md5 AND m.rp_index = d.rp_index "
			"WHERE m.vpth = ?"
		);
		q.addBindValue(path);
		if (q.exec() && q.next()) {
			return q.value(0).toByteArray();
		}
		return QByteArray();
	}

	/*!
		通过 VID 读取文件数据。VID 格式为 "MD5-XXXX"，其中 XXXX 为四位十六进制碰撞序号。
	*/
	QByteArray VirtualStorage::readFileByVID(const QString& vid) {
		if (!d->db.isOpen()) {
			return QByteArray();
		}

		auto parsed = d->parseVID(vid);
		if (parsed.second < 0) {
			return QByteArray();
		}

		QSqlQuery q(d->db);
		q.prepare("SELECT data FROM vfile_data WHERE md5 = ? AND rp_index = ?");
		q.addBindValue(parsed.first);
		q.addBindValue(parsed.second);
		if (q.exec() && q.next()) {
			return q.value(0).toByteArray();
		}
		return QByteArray();
	}

	/*!
		返回虚拟路径 \a path 对应文件的数据大小（字节）。若路径不存在则返回 -1。
	*/
	qint32 VirtualStorage::getFileSize(const QString& path) {
		if (!d->db.isOpen()) {
			return -1;
		}

		QSqlQuery q(d->db);
		q.prepare(
			"SELECT d.size "
			"FROM vpth_map m "
			"JOIN vfile_data d ON m.md5 = d.md5 AND m.rp_index = d.rp_index "
			"WHERE m.vpth = ?"
		);
		q.addBindValue(path);
		if (q.exec() && q.next()) {
			return (qint32)q.value(0).toLongLong();
		}
		return -1;
	}

	/*!
		返回与虚拟路径 \a path 所指向文件内容完全相同的虚拟路径数量（含自身）。
	*/
	qint32 VirtualStorage::getFileRepeatCount(const QString& path) {
		if (!d->db.isOpen()) {
			return 0;
		}

		QSqlQuery q(d->db);
		q.prepare(
			"SELECT COUNT(*) "
			"FROM vpth_map m "
			"JOIN vpth_map m2 ON m2.md5 = m.md5 AND m2.rp_index = m.rp_index "
			"WHERE m.vpth = ?"
		);
		q.addBindValue(path);
		if (q.exec() && q.next()) {
			return q.value(0).toInt();
		}
		return 0;
	}

	/*!
		返回虚拟路径 \a path 对应文件的 VID（MD5-四位十六进制碰撞序号）。
	*/
	QString VirtualStorage::getFileVID(const QString& path) {
		if (!d->db.isOpen()) {
			return QString();
		}

		QSqlQuery q(d->db);
		q.prepare("SELECT md5, rp_index FROM vpth_map WHERE vpth = ?");
		q.addBindValue(path);
		if (q.exec() && q.next()) {
			return d->formatVID(
				q.value(0).toString(),
				q.value(1).toInt()
			);
		}
		return QString();
	}

	/*!
		删除虚拟路径 \a path 的映射。若该数据条目不再被任何虚拟路径引用，则同时删除数据。
		返回被移除路径的 VID；若路径不存在则返回空字符串。
	*/
	QString VirtualStorage::removeFile(const QString& path) {
		if (!d->db.isOpen()) {
			return QString();
		}

		QString vid = getFileVID(path);
		if (vid.isEmpty()) {
			return QString();
		}

		auto parsed = d->parseVID(vid);

		d->db.transaction();

		QSqlQuery q(d->db);
		q.prepare("DELETE FROM vpth_map WHERE vpth = ?");
		q.addBindValue(path);
		q.exec();

		d->removeVpthFromData(parsed.first, parsed.second, path);

		d->db.commit();
		return vid;
	}

	/*!
		删除指定 VID 对应的数据条目及其所有虚拟路径映射。
		返回被删除的 VID；若 VID 无效或不存在则返回空字符串。
	*/
	QString VirtualStorage::removeFileByVID(const QString& vid) {
		if (!d->db.isOpen()) {
			return QString();
		}

		auto parsed = d->parseVID(vid);
		if (parsed.second < 0) {
			return QString();
		}

		{
			QSqlQuery ck(d->db);
			ck.prepare("SELECT COUNT(*) FROM vpth_map WHERE md5 = ? AND rp_index = ?");
			ck.addBindValue(parsed.first);
			ck.addBindValue(parsed.second);
			if (!ck.exec() || !ck.next() || ck.value(0).toInt() == 0) {
				return QString();
			}
		}

		d->db.transaction();

		QSqlQuery q(d->db);
		q.prepare("DELETE FROM vpth_map WHERE md5 = ? AND rp_index = ?");
		q.addBindValue(parsed.first);
		q.addBindValue(parsed.second);
		q.exec();

		q.prepare("DELETE FROM vfile_data WHERE md5 = ? AND rp_index = ?");
		q.addBindValue(parsed.first);
		q.addBindValue(parsed.second);
		q.exec();

		d->db.commit();
		return vid;
	}

	/*!
		列出虚拟路径 \a folderPath 下的子文件夹和文件。
		若 \a recursive 为 true，则递归列出所有子孙节点。

		返回的 JsonConfig 结构为：
		{
		    "sub_folders": { "名称": { ... }, ... },
		    "files":       { "名称": "VID", ... }
		}
	*/
	JsonConfig VirtualStorage::listFolder(const QString& folderPath, bool recursive) {
		JsonConfig result;

		if (!d->db.isOpen()) {
			return result;
		}

		// 正规化前缀
		QString prefix = folderPath;
		while (prefix.startsWith(QLatin1Char('/'))) {
			prefix.remove(0, 1);
		}
		if (!prefix.isEmpty() && !prefix.endsWith(QLatin1Char('/'))) {
			prefix += QLatin1Char('/');
		}

		QSqlQuery q(d->db);
		if (prefix.isEmpty()) {
			q.prepare("SELECT vpth FROM vpth_map ORDER BY vpth ASC");
		} else {
			q.prepare("SELECT vpth FROM vpth_map WHERE vpth LIKE ? ORDER BY vpth ASC");
			q.addBindValue(prefix + QString::fromUtf8("%"));
		}

		if (!q.exec()) {
			return result;
		}

		QStringList directSubFolders;
		QMap<QString, QString> directFiles; // name -> VID

		while (q.next()) {
			QString vpth = q.value(0).toString();
			QString relative = vpth.mid(prefix.length());
			if (relative.isEmpty()) {
				continue;
			}

			int slashPos = relative.indexOf(QLatin1Char('/'));
			if (slashPos < 0) {
				QString vid = getFileVID(vpth);
				if (!vid.isEmpty() && !directFiles.contains(relative)) {
					directFiles.insert(relative, vid);
				}
			} else {
				QString subName = relative.left(slashPos);
				if (!directSubFolders.contains(subName)) {
					directSubFolders.append(subName);
				}
			}
		}

		// 构建 files
		{
			JsonConfig filesObj;
			for (auto it = directFiles.constBegin(); it != directFiles.constEnd(); ++it) {
				filesObj.setString(it.key(), it.value());
			}
			result.setObject("files", filesObj);
		}

		// 构建 sub_folders
		{
			JsonConfig foldersObj;
			for (const QString& subName : directSubFolders) {
				if (recursive) {
					foldersObj.setObject(subName,
						listFolder(prefix + subName, true));
				} else {
					foldersObj.setObject(subName, JsonConfig());
				}
			}
			result.setObject("sub_folders", foldersObj);
		}

		return result;
	}

}