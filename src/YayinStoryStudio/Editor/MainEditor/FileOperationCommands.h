#ifndef YayinStoryStudio_Editor_MainEditor_FileOperationCommands_h
#define YayinStoryStudio_Editor_MainEditor_FileOperationCommands_h
#include <QtGui/qundostack.h>
#include <QtCore/qstring.h>
#include <Utility/FileOperation.h>

namespace YSS::Editor {

class FileOperationCommand : public QUndoCommand {
public:
	FileOperationCommand(const QString& text, const QString& target)
		: QUndoCommand(text), Target(target) {}
	Visindigo::Utility::FileOperation::ErrorCode getErrorCode() const { return LastError; }
	QString getTarget() const { return Target; }
protected:
	QString Target;
	Visindigo::Utility::FileOperation::ErrorCode LastError = Visindigo::Utility::FileOperation::Success;
};

class FileCopyCommand : public FileOperationCommand {
public:
	FileCopyCommand(const QString& src, const QString& dst);
	void undo() override;
	void redo() override;
private:
	QString Src;
    QString Dst;
};

class FileMoveCommand : public FileOperationCommand {
public:
	FileMoveCommand(const QString& src, const QString& dst);
	void undo() override;
	void redo() override;
private:
	QString Src;
    QString Dst;
};

class FileDeleteCommand : public FileOperationCommand {
public:
	FileDeleteCommand(const QString& path);
	void undo() override;
	void redo() override;
private:
	QString Path;
};

} // namespace YSS::Editor

Q_DECLARE_METATYPE(QUndoCommand*)
Q_DECLARE_METATYPE(YSS::Editor::FileOperationCommand*)
#endif // YayinStoryStudio_Editor_MainEditor_FileOperationCommands_h
