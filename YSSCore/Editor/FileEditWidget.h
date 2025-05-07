#pragma once
#include <QObject>
#include <QString>
#include <QFrame>
#include "../Macro.h"

class QWidget;
namespace YSSCore::Editor {
	class YSSCoreAPI FileEditWidget :public QFrame {
		Q_OBJECT;
	public:
		FileEditWidget(QWidget* parent = nullptr);
		virtual QString getFilePath() const = 0;
		virtual QString getFileName() const = 0;
		virtual bool openFile(const QString& path) = 0;
		virtual bool onClose() = 0;
		virtual bool onSave(const QString& path = "") = 0;
		virtual bool onSaveAs(const QString& path = "") = 0;
		virtual bool onReload() = 0;
	};
}