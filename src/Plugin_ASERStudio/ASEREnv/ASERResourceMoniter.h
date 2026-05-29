#ifndef ASERStudio_ASEREnv_ASERResourceMoniter_h
#define ASERStudio_ASEREnv_ASERResourceMoniter_h
#include "ASERStudioCompileMacro.h"
#include <QtCore/qobject.h>
namespace ASERStudio::ASEREnv {
	class ASERResourceMoniterPrivate;
	class ASERAPI ASERResourceMoniter :public QObject {
	private:
		ASERResourceMoniter();
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
		static ASERResourceMoniter* getInstance();
		~ASERResourceMoniter();
		QStringList getBackground();
		QStringList getCharacters();
		QStringList getCharaDiff(const QString& chara);
		QStringList getMusic();
		QStringList getSoundEffect();
		void changeProjectPath(const QString& path);
		void refresh(ResourceTypes types = ResourceType::All);
	private:
		ASERResourceMoniterPrivate* d;
	};
	Q_DECLARE_OPERATORS_FOR_FLAGS(ASERResourceMoniter::ResourceTypes)
}
#define ASERRM ASERStudio::ASEREnv::ASERResourceMoniter::getInstance()
#endif ASERStudio_ASEREnv_ASERResourceMoniter_h