#pragma once
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtWidgets/QFrame>
#include <QtGui/QIcon>
#include "../Macro.h"

namespace YSSCore::Editor {
	class YSSCoreAPI FileTemplateInitWidget :public QFrame {
		Q_OBJECT;
	signals:
		void filePrepared(QString filePath);
		void closed();
	public:
		FileTemplateInitWidget(QWidget* parent = nullptr);
	};

	class EditorPlugin;
	class FileTemplateProviderPrivate;

	class YSSCoreAPI FileTemplateProvider {
		friend class FileTemplateProviderPrivate;
	public:
		FileTemplateProvider(EditorPlugin* plugin);
		virtual ~FileTemplateProvider();
		QIcon getTemplateIcon();
		void setTemplateIcon(const QIcon& icon);
		QString getTemplateID();
		void setTemplateID(const QString& id);
		QString getTemplateName();
		void setTemplateName(const QString& name);
		QString getTemplateDescription();
		void setTemplateDescription(const QString& description);
		QStringList getTemplateTags();
		void setTemplateTags(const QStringList& tags);
		virtual FileTemplateInitWidget* fileInitWidget() = 0;
	private:
		FileTemplateProviderPrivate* d;
	};
}