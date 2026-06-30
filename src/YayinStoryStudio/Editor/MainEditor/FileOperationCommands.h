#pragma once
#include <QtGui/qundostack.h>
#include <QtCore/qstring.h>

namespace YSS::Editor {

class FileCopyCommand : public QUndoCommand {
public:
	FileCopyCommand(const QString& src, const QString& dst);
	void undo() override;
	void redo() override;
private:
	QString Src;
    QString Dst;
};

class FileMoveCommand : public QUndoCommand {
public:
	FileMoveCommand(const QString& src, const QString& dst);
	void undo() override;
	void redo() override;
private:
	QString Src;
    QString Dst;
};

class FileDeleteCommand : public QUndoCommand {
public:
	FileDeleteCommand(const QString& path);
	void undo() override;
	void redo() override;
private:
	QString Path;
    QString Backup;
};

} // namespace YSS::Editor

Q_DECLARE_METATYPE(QUndoCommand*)
