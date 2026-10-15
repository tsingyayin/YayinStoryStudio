#include <QtCore/qfileinfo.h>
#include "Editor/MainEditor/FileOperationCommands.h"

namespace YSS::Editor {
using Visindigo::Utility::FileOperation;

FileCopyCommand::FileCopyCommand(const QString& src, const QString& dst)
	: FileOperationCommand(QStringLiteral("Copy '%1'").arg(QFileInfo(src).fileName()), src)
	, Src(src), Dst(dst) {}

void FileCopyCommand::undo() {
	if (QFileInfo(Dst).isDir()) {
		FileOperation::deleteDir(Dst, false);
	}
	else {
		FileOperation::deleteFile(Dst, false);
	}
}

void FileCopyCommand::redo() {
	if (QFileInfo(Src).isDir()) {
		LastError = FileOperation::copyDir(Src, Dst, false);
	}
	else {
		LastError = FileOperation::copyFile(Src, Dst, false);
	}
}

FileMoveCommand::FileMoveCommand(const QString& src, const QString& dst)
	: FileOperationCommand(QStringLiteral("Move '%1'").arg(QFileInfo(src).fileName()), src)
	, Src(src), Dst(dst) {}

void FileMoveCommand::undo() {
	if (QFileInfo(Dst).isDir()) {
		FileOperation::moveDir(Dst, Src, false);
	}
	else {
		FileOperation::moveFile(Dst, Src, false);
	}
}

void FileMoveCommand::redo() {
	if (QFileInfo(Src).isDir()) {
		LastError = FileOperation::moveDir(Src, Dst, false);
	}
	else {
		LastError = FileOperation::moveFile(Src, Dst, false);
	}
}

FileDeleteCommand::FileDeleteCommand(const QString& path)
	: FileOperationCommand(QStringLiteral("Delete '%1'").arg(QFileInfo(path).fileName()), path)
	, Path(path) {}

void FileDeleteCommand::undo() {
	// 删除走的是回收站，回收站里的位置无法由路径反推，因此这里无法复原（本命令也不会被压栈）。
}

void FileDeleteCommand::redo() {
	if (QFileInfo(Path).isDir()) {
		LastError = FileOperation::deleteDir(Path, true);
	}
	else {
		LastError = FileOperation::deleteFile(Path, true);
	}
}

} // namespace YSS::Editor
