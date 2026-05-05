#ifndef Visindigo_Widgets_PixmapCacheHelper_h
#define Visindigo_Widgets_PixmapCacheHelper_h
#include "VICompileMacro.h"
#include <QtCore/qmetaobject.h>
#include <QtCore/qsize.h>

namespace Visindigo::Widgets
{
	class PixmapCacheHelperPrivate;
	class VisindigoAPI PixmapCacheHelper
	{
	private:
		PixmapCacheHelper();
	public:
		static PixmapCacheHelper* getInstance();
		~PixmapCacheHelper();
		QPixmap getPixmap(const QString& path, const QSize& size = QSize(), bool keepAspectRatio = true);
		void setCachePath(const QString& path);
		QString getCachePath() const;
		void save();
		void clearCache();
	private:
		PixmapCacheHelperPrivate* d;
	};
}
#endif // Visindigo_Widgets_PixmapCacheHelper_h