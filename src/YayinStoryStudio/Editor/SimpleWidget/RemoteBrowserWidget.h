#ifndef YayinStoryStudio_Editor_SimpleWidget_RemoteBrowserWidget_h
#define YayinStoryStudio_Editor_SimpleWidget_RemoteBrowserWidget_h
#include <General/YSSURI.h>
#include <QtCore/qstringlist.h>
#include <QtWidgets/qwidget.h>
// Forward declarations
namespace YSS::Editor {
	class RemoteBrowserWidgetPrivate;
	class RemoteBrowserWidget :public QWidget {
		Q_OBJECT;
	signals:
		void currentUriChanged(const YSSCore::General::YSSURI& uri);
		void fileActivated(const YSSCore::General::YSSURI& uri);
		void selectionChanged();
	public:
		RemoteBrowserWidget(QWidget* parent = nullptr);
		virtual ~RemoteBrowserWidget();
		void setFileSuffixFilters(const QStringList& suffixes);
		YSSCore::General::YSSURI getCurrentUri() const;
		QString getCurrentPath() const;
		YSSCore::General::YSSURI getSelectedFileUri() const;
		void navigateTo(const QString& path);
		void refresh();
		void refreshAuthentics();
	private:
		RemoteBrowserWidgetPrivate* d;
	};
}
#endif // YayinStoryStudio_Editor_SimpleWidget_RemoteBrowserWidget_h
