#include "Editor/MainEditor/FileOperationCommands.h"
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qdatetime.h>

namespace YSS::Editor {
static bool copyPathRecursive(const QString& src, const QString& dst) {
	const QFileInfo srcInfo(src);
	if (srcInfo.isDir()) {
		QDir().mkpath(dst);
		const auto entries = QDir(src).entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
		for (const QFileInfo& entry : entries) {
			if (!copyPathRecursive(entry.absoluteFilePath(), dst + "/" + entry.fileName()))
				return false;
		}
		return true;
	}
	return QFile::copy(src, dst);
}

FileCopyCommand::FileCopyCommand(const QString& src, const QString& dst)
	: QUndoCommand(QStringLiteral("Copy '%1'").arg(QFileInfo(src).fileName()))
	, Src(src), Dst(dst) {}

void FileCopyCommand::undo() {
	if (QFileInfo(Dst).isDir())
		QDir(Dst).removeRecursively();
	else
		QFile::remove(Dst);
}

void FileCopyCommand::redo() {
	copyPathRecursive(Src, Dst);
}

FileMoveCommand::FileMoveCommand(const QString& src, const QString& dst)
	: QUndoCommand(QStringLiteral("Move '%1'").arg(QFileInfo(src).fileName()))
	, Src(src), Dst(dst) {}

void FileMoveCommand::undo() {
	QFile::rename(Dst, Src);
}

void FileMoveCommand::redo() {
	QFile::rename(Src, Dst);
}

FileDeleteCommand::FileDeleteCommand(const QString& path)
	: QUndoCommand(QStringLiteral("Delete '%1'").arg(QFileInfo(path).fileName()))
	, Path(path)
{
	// NOTICE: 
	// we will implement a in-project rubbish bin in the future with back-up function.
	// this rename is just for feature testing, and will be removed in the future.
	Backup = QFileInfo(path).absolutePath() + "/.~"
		+ QFileInfo(path).fileName() + "_"
		+ QString::number(QDateTime::currentMSecsSinceEpoch());
}

void FileDeleteCommand::undo() {
	if (!Backup.isEmpty())
		QFile::rename(Backup, Path);
}

void FileDeleteCommand::redo() {
	QFile::rename(Path, Backup);
}

} // namespace YSS::Editor
