#pragma once
#include <QtWidgets/qframe.h>
#include "../Macro.h"
// Forward declarations
class QWidget;
class QString;
namespace YSSCore::Editor {
	class FileEditWidgetPrivate;
}
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI FileEditWidget :public QFrame {
		Q_OBJECT;
		VImpl(FileEditWidget);
	signals:
		void fileChanged(const QString& filePath);
		void fileChangeCanceled(const QString& filePath);
		void fileSaved(const QString& filePath);
		void fileRenamed(const QString& rawPath, const QString& changedPath);
		void fileClosed(const QString& filePath);
	public:
		FileEditWidget(QWidget* parent = nullptr);
		virtual ~FileEditWidget();
		QString getFilePath() const;
		QString getFileName() const;
		bool isFileChanged() const;
		bool isVirtualFile() const;
		QString getFileExt() const;
		QString getVirtualFileParam() const;
		void setFileChanged();
		void cancelFileChanged();
	public:
		bool openFile(const QString& path);
		bool saveFile(const QString& path = "", bool deleteWhenSaveAs = false);
		bool reloadFile();
		void closeFile();
	public:
		bool copy();
		bool cut();
		bool paste();
		bool undo();
		bool redo();
		bool selectAll();
		bool cursorToPosition(qint32 lineNumber, qint32 column);
	protected:
		virtual bool onOpen(const QString& path);
		virtual bool onClose();
		virtual bool onSave(const QString& path);
		virtual bool onReload();
	protected:
		virtual bool onCopy();
		virtual bool onCut();
		virtual bool onPaste();
		virtual bool onUndo();
		virtual bool onRedo();
		virtual bool onSelectAll();
		virtual bool onCursorToPosition(qint32 lineNumber, qint32 column);
	protected:
		virtual bool onVirtualOpen(const QString& ext, const QString& fileName, const QString& param);
		virtual bool onVirtualClose();
		virtual bool onVirtualSave();
		virtual bool onVirtualReload();
	public:
		virtual void closeEvent(QCloseEvent* event) override;
	};
}