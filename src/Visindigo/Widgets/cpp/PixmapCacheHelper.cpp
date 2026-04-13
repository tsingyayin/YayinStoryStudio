#include "Widgets/PixmapCacheHelper.h"
#include <QtCore/qmap.h>
#include <QtCore/qrect.h>
#include <QtCore/qdatetime.h>
#include <QtGui/qpixmap.h>
#include <QtCore/qqueue.h>
#include <QtGui/qpainter.h>
#include "Utility/JsonConfig.h"
#include "Utility/FileUtility.h"
namespace Visindigo::Widgets {
	struct PixmapTileData {
		QPixmap Pixmap;
		qint32 TileWidth = 0;
		qint32 RowCount = 0;
		QQueue<qint32> FreeTileIndexes;
	};
	struct PixmapCacheData {
		QDateTime CacheTime;
		quint16 PixmapIndex = 0;
		qint32 PixmapTileIndex = 0;
		qint32 Width = 0;
		qint32 Height = 0;
	};
	class PixmapCacheHelperPrivate {
		friend class PixmapCacheHelper;
	protected:
		QList<PixmapTileData> CachePixmaps;
		QMap<QString, PixmapCacheData> CacheMap;
		QString CachePath;
		Visindigo::Utility::JsonConfig CacheMetaData;
		static PixmapCacheHelper* Instance;
	protected:
		void loadCache() {
			if (CachePath.isEmpty()) {
				return; // save in memory, not in disk
			}
			CacheMap.clear();
			CachePixmaps.clear();
			CacheMetaData.parse(Utility::FileUtility::readAll(CachePath + "/metaData.json"));
			for (auto index : CacheMetaData.keys("Tiles")) {
				PixmapTileData tileData;
				QString fileName = CacheMetaData.getString("Tiles." % index % ".fileName");
				tileData.Pixmap = QPixmap(CachePath + "/" + fileName);
				tileData.TileWidth = CacheMetaData.getInt("Tiles." % index % ".tileWidth");
				tileData.RowCount = tileData.Pixmap.width() / tileData.TileWidth;
				QStringList freeIndexesStr = CacheMetaData.getString("Tiles." % index % ".freeIndexes").split(";");
				for (auto indexes : freeIndexesStr) {
					tileData.FreeTileIndexes.append(indexes.toInt());
				}
				CachePixmaps.append(tileData);
			}
			for (auto index : CacheMetaData.keys("Files")) {
				PixmapCacheData cacheData;
				QString fileName = CacheMetaData.getString("Files." % index % ".fileName");
				QStringList dataStr = CacheMetaData.getString("Files." % index % ".data").split(";");
				if (dataStr.size() != 5) {
					continue;
				}
				bool ok = false;
				cacheData.CacheTime = QDateTime::fromSecsSinceEpoch(dataStr[0].toInt(&ok));
				cacheData.PixmapIndex = dataStr[1].toInt(&ok);
				cacheData.PixmapTileIndex = dataStr[2].toInt(&ok);
				cacheData.Width = dataStr[3].toInt(&ok);
				cacheData.Height = dataStr[4].toInt(&ok);
				if (not ok) { continue; }
				CacheMap[fileName % "_" % QString::number(cacheData.Width) % "_" % QString::number(cacheData.Height)] = cacheData;
			}
		}
		void saveConfig() {
			if (CachePath.isEmpty()) {
				return; // save in memory, not in disk
			}
			CacheMetaData = Visindigo::Utility::JsonConfig();
			for (int i = 0; i < CachePixmaps.size(); i++) {
				CacheMetaData.setString("Tiles." % QString::number(i) % ".fileName", "tile_" % QString::number(i) % ".png");
				CacheMetaData.setInt("Tiles." % QString::number(i) % ".tileWidth", CachePixmaps[i].TileWidth);
				QStringList freeIndexesStr;
				for (auto index : CachePixmaps[i].FreeTileIndexes) {
					freeIndexesStr.append(QString::number(index));
				}
				CacheMetaData.setString("Tiles." % QString::number(i) % ".freeIndexes", freeIndexesStr.join(";"));
				CachePixmaps[i].Pixmap.save(CachePath + "/tile_" % QString::number(i) % ".png");
			}
			int fileIndex = 0;
			for (auto it = CacheMap.begin(); it != CacheMap.end(); it++) {
				QString fileName = it.key();
				PixmapCacheData cacheData = it.value();
				QStringList dataStr;
				dataStr.append(QString::number(cacheData.CacheTime.toSecsSinceEpoch()));
				dataStr.append(QString::number(cacheData.PixmapIndex));
				dataStr.append(QString::number(cacheData.PixmapTileIndex));
				dataStr.append(QString::number(cacheData.Width));
				dataStr.append(QString::number(cacheData.Height));
				CacheMetaData.setString("Files." % QString::number(fileIndex) % ".fileName", fileName);
				CacheMetaData.setString("Files." % QString::number(fileIndex) % ".data", dataStr.join(";"));
				fileIndex++;
			}
			Utility::FileUtility::saveAll(CachePath + "/metaData.json", CacheMetaData.toString());
		}
		void removeCachePixmap(int pixmapIndex) {
			if (pixmapIndex >= CachePixmaps.size()) {
				return;
			}
			QString fileName = "tile_" % QString::number(pixmapIndex) % ".png";
			Utility::FileUtility::deleteFile(CachePath + "/" + fileName);
			CachePixmaps.removeAt(pixmapIndex);
			for (auto it = CacheMap.begin(); it != CacheMap.end();) {
				if (it.value().PixmapIndex == pixmapIndex) {
					it = CacheMap.erase(it);
				}
				else {
					if (it.value().PixmapIndex > pixmapIndex) {
						it.value().PixmapIndex--;
					}
					it++;
				}
			}
		}
		void clearCache() {
			for (int i = 0; i < CachePixmaps.size(); i++) {
				QString fileName = "tile_" % QString::number(i) % ".png";
				Utility::FileUtility::deleteFile(CachePath + "/" + fileName);
			}
			CachePixmaps.clear();
			CacheMap.clear();
			CacheMetaData = Visindigo::Utility::JsonConfig();
			Utility::FileUtility::deleteFile(CachePath + "/metaData.json");
		}
		QPixmap getPixmap(const QString& path, const QSize& size = QSize(), bool keepAspectRatio = true) {
			if (size == QSize()) {
				return QPixmap(path);
			}
			else {
				QString cacheIndexName = path % "_" % QString::number(size.width()) % "_" % QString::number(size.height());
				if (CacheMap.contains(cacheIndexName)) {
					QDateTime lastModified = Utility::FileUtility::getFileModifyTime(path);
					PixmapCacheData cacheData = CacheMap[cacheIndexName];
					int pixmapIndex = cacheData.PixmapIndex;
					if (pixmapIndex >= CachePixmaps.size()) {
						CacheMap.remove(cacheIndexName);
						goto doCache;
					}
					if (lastModified > cacheData.CacheTime) {
						CacheMap.remove(cacheIndexName);
						CachePixmaps[pixmapIndex].FreeTileIndexes.append(cacheData.PixmapTileIndex);
						goto doCache;
					}
					if (cacheData.Width > CachePixmaps[pixmapIndex].TileWidth || cacheData.Height > CachePixmaps[pixmapIndex].TileWidth) {
						CacheMap.remove(cacheIndexName);
						goto doCache;
					}
					int ax = cacheData.PixmapTileIndex % CachePixmaps[pixmapIndex].RowCount * CachePixmaps[pixmapIndex].TileWidth;
					int ay = cacheData.PixmapTileIndex / CachePixmaps[pixmapIndex].RowCount * CachePixmaps[pixmapIndex].TileWidth;
					QPixmap cachePixmap = CachePixmaps[pixmapIndex].Pixmap.copy(ax, ay, cacheData.Width, cacheData.Height);
					return cachePixmap;			
				}
			doCache:
				QPixmap rawPixmap(path);
				QPixmap scaled = rawPixmap.scaled(size, keepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
				quint32 maxLength = std::max(scaled.width(), scaled.height());
				quint8 largestP2 = 0;
				for (int i = 0; i < 32; i++) {
					if ((1 << i) > maxLength) {
						largestP2 = i;
						break;
					}
				}
				quint32 p2Width = 1 << largestP2;
				quint32 p2WidthL = p2Width >> 1;
				bool ok = false;
				for (int i = 0; i < CachePixmaps.size(); i++) {
					if (CachePixmaps[i].TileWidth <= p2Width 
						&& CachePixmaps[i].TileWidth > p2WidthL
						&& CachePixmaps[i].FreeTileIndexes.size() != 0) {
						int tileIndex = CachePixmaps[i].FreeTileIndexes.dequeue();
						int ax = tileIndex % CachePixmaps[i].RowCount * CachePixmaps[i].TileWidth;
						int ay = tileIndex / CachePixmaps[i].RowCount * CachePixmaps[i].TileWidth;
						QPainter painter(&CachePixmaps[i].Pixmap);
						painter.drawPixmap(ax, ay, scaled);
						painter.end();
						PixmapCacheData cacheData;
						cacheData.CacheTime = QDateTime::currentDateTime();
						cacheData.PixmapIndex = i;
						cacheData.PixmapTileIndex = tileIndex;
						cacheData.Width = scaled.width();
						cacheData.Height = scaled.height();
						CacheMap[cacheIndexName] = cacheData;
						ok = true;
						break;
					}
				}
				if (not ok) {
					int width = 0;
					if (p2Width < 512) {
						width = p2Width * 10;
					}
					else if (p2Width < 2048) {
						width = p2Width * 5;
					}
					else {
						width = p2Width * 2;
					}
					QPixmap tilePixmap(width, width);
					tilePixmap.fill(Qt::transparent);
					QPainter painter(&tilePixmap);
					painter.drawPixmap(0, 0, scaled);
					painter.end();
					PixmapTileData tileData;
					tileData.Pixmap = tilePixmap;
					tileData.TileWidth = p2Width;
					tileData.RowCount = width / p2Width;
					for (int i = 0; i < tileData.RowCount * tileData.RowCount; i++) {
						if (i != 0)[[likely]] {
							tileData.FreeTileIndexes.append(i);
						}
					}
					CachePixmaps.append(tileData);
					PixmapCacheData cacheData;
					cacheData.CacheTime = QDateTime::currentDateTime();
					cacheData.PixmapIndex = CachePixmaps.size() - 1;
					cacheData.PixmapTileIndex = 0;
					cacheData.Width = scaled.width();
					cacheData.Height = scaled.height();
					CacheMap[cacheIndexName] = cacheData;
				}
				return scaled;
			}
		}
	};

	PixmapCacheHelper* PixmapCacheHelperPrivate::Instance = nullptr;

	/*!
		\class Visindigo::Widgets::PixmapCacheHelper
		\brief PixmapCacheHelper提供了一个在读取时自动创建QPixmap缓存的工具类。
		\since VIsindigo 0.13.0
		\inmodule Visindigo

		PixmapCacheHelper提供了一个在读取时自动创建QPixmap缓存的工具类。当你需要读取一个
		QPixmap并对其进行缩放时，这个类就很有用，它会将缩放结果缓存起来，以便下次读取时直接使用缓存结果，
		从而提高性能。对于同一张图片的不同缩放结果，PixmapCacheHelper会分别缓存它们，以便在需要时快速获取。

		显然，如果你需要频繁以固定缩放比读取同一张图片（譬如文件浏览器中的缩略图），PixmapCacheHelper会非常有用。

		你可以通过setCachePath方法设置缓存路径，如果不设置，则默认在内存中缓存，不会保存到磁盘。

		为了提高IO性能，PixmapCacheHelper并不松散存储每个缩放位图，而是将多个位图拼合为一个大位图进行存储。每个
		大位图的宽高都是2的幂次方，且不小于所需存储的最大缩放位图的宽高。每个大位图被划分为多个固定大小的瓦片，
		每个瓦片可以存储一个缩放位图。当需要存储一个缩放位图时，PixmapCacheHelper会在现有的大位图中寻找一个空闲的瓦片来存储它，
		如果没有合适的空闲瓦片，则会创建一个新的大位图来存储它。

		\warning 这类暂时不建议使用，目前仍然处于技术预览状态。
	*/

	/*!
		\since VIsindigo 0.13.0
		构造函数
	*/
	PixmapCacheHelper::PixmapCacheHelper() {
		this->d = new PixmapCacheHelperPrivate();
	}

	/*!
		\since VIsindigo 0.13.0
		获取PixmapCacheHelper实例
	*/
	PixmapCacheHelper* PixmapCacheHelper::getInstance() {
		if (PixmapCacheHelperPrivate::Instance == nullptr) {
			PixmapCacheHelperPrivate::Instance = new PixmapCacheHelper();
		}
		return PixmapCacheHelperPrivate::Instance;
	}

	/*!
		\since VIsindigo 0.13.0
		析构函数
	*/
	PixmapCacheHelper::~PixmapCacheHelper() {
		delete d;
	}

	/*!
		\since VIsindigo 0.13.0
		\a path是图片路径，\a size是缩放后的大小，如果为默认值则不进行缩放，\a keepAspectRatio表示是否保持宽高比，默认为true。
		如果图片不进行缩放，就不创建缓存。等同于直接调用QPixmap构造函数读取图片。
		获取Pixmap
	*/
	QPixmap PixmapCacheHelper::getPixmap(const QString& path, const QSize& size, bool keepAspectRatio) {
		return d->getPixmap(path, size, keepAspectRatio);
	}

	/*!
		\since VIsindigo 0.13.0
		设置缓存路径，如果不设置则默认在内存中缓存，不会保存到磁盘。
		设置了缓存路径后，会尝试从该路径加载缓存，如果加载失败则会清空缓存并重新创建。
	*/
	void PixmapCacheHelper::setCachePath(const QString& path) {
		d->CachePath = path;
		d->loadCache();
	}

	/*!
		\since VIsindigo 0.13.0
		获取缓存路径，如果不设置则默认在内存中缓存，不会保存到磁盘。
	*/
	QString PixmapCacheHelper::getCachePath() const {
		return d->CachePath;
	}

	/*!
		\since VIsindigo 0.13.0
		保存缓存到磁盘，如果没有设置缓存路径则不进行保存。
	*/
	void PixmapCacheHelper::save() {
		d->saveConfig();
	}

	/*!
		\since VIsindigo 0.13.0
		清空缓存，删除所有缓存文件，如果没有设置缓存路径则只清空内存中的缓存。
	*/
	void PixmapCacheHelper::clearCache() {
		d->clearCache();
	}
}