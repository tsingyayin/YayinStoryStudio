#pragma once
#include <QtWidgets/qframe.h>
#include "../Macro.h"
// Forward declarations
class QWidget;
class QString;
namespace Visindigo::Editor {
	class FileEditWidgetPrivate;
}
// Main
namespace Visindigo::Editor {
	class VisindigoAPI FileEditWidget :public QFrame {
		Q_OBJECT;
		VImpl(FileEditWidget);
	signals:
		void fileChanged(const QString& filePath);
		void fileSaved(const QString& filePath);
	public:
		FileEditWidget(QWidget* parent = nullptr);
		virtual ~FileEditWidget();
		QString getFilePath() const;
		QString getFileName() const;
		bool isFileChanged() const;
		void setFileChanged();
		void cancelFileChanged();
		bool openFile(const QString& path);
		bool saveFile(const QString& path = "");
		bool reloadFile();
		bool closeFile();
		bool copy();
		bool cut();
		bool paste();
		bool undo();
		bool redo();
		bool selectAll();
	protected:
		virtual bool onOpen(const QString& path) = 0;
		virtual bool onClose() = 0;
		virtual bool onSave(const QString& path = "") = 0;
		virtual bool onReload() = 0;
		virtual bool onCopy() = 0;
		virtual bool onCut() = 0;
		virtual bool onPaste() = 0;
		virtual bool onUndo() = 0;
		virtual bool onRedo() = 0;
		virtual bool onSelectAll() = 0;
	protected:
		virtual void closeEvent(QCloseEvent* event) override;
	};
}