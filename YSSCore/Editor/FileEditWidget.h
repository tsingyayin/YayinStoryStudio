#pragma once
#include <QtCore/qobject.h>
#include <QtWidgets/qframe.h>
#include "../Macro.h"

class QWidget;
class QString;
namespace YSSCore::Editor {
	class FileEditWidgetPrivate;
	class YSSCoreAPI FileEditWidget :public QFrame {
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
		bool closeFile();
	protected:
		virtual bool onOpen(const QString& path) = 0;
		virtual bool onClose() = 0;
		virtual bool onSave(const QString& path = "") = 0;
		virtual bool onReload() = 0;
	protected:
		virtual void closeEvent(QCloseEvent* event) override;
	};
}