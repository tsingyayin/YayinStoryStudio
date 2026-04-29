#ifndef ASERStudio_YSS_ResourceMoniter_h
#define ASERStudio_YSS_ResourceMoniter_h
#include "ASERStudioCompileMacro.h"
#include <QtCore/qobject.h>
namespace ASERStudio::YSS {
	class ResourceMoniterPrivate;
	class ASERAPI ResourceMoniter :public QObject{
	private:
		ResourceMoniter();
	public:
		enum class ResourceType {
			Background = 0x01,
			Character = 0x02,
			Music = 0x04,
			SoundEffect = 0x08,
			All = Background | Character | Music | SoundEffect
		};
		Q_DECLARE_FLAGS(ResourceTypes, ResourceType)
	public:
		static QString getASERStandardConfigPath();
	public:
		static ResourceMoniter* getInstance();
		~ResourceMoniter();
		QStringList getBackground();
		QStringList getCharacters();
		QStringList getCharaDiff(const QString& chara);
		QStringList getMusic();
		QStringList getSoundEffect();
		void changeProjectPath(const QString& path);
		void refresh(ResourceTypes types = ResourceType::All);
	private:
		ResourceMoniterPrivate* d;
	};
	Q_DECLARE_OPERATORS_FOR_FLAGS(ResourceMoniter::ResourceTypes)
}
#define ASERRM ASERStudio::YSS::ResourceMoniter::getInstance()
#endif ASERStudio_YSS_ResourceMoniter_h