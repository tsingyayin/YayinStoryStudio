#ifndef ASERStudio_AStorySyntax_AStoryXController_h
#define ASERStudio_AStorySyntax_AStoryXController_h
#include "ASERStudioCompileMacro.h"
#include <Utility/JsonConfig.h>
#include <VIMacro.h>
class QString;
namespace ASERStudio::AStorySyntax {
	class AStoryXControllerParseData;
	class AStoryXValueMeta;
	class AStoryXControllerPrivate;
	class ASERAPI AStoryXController{
		Q_GADGET
	public:
		enum ControllerType {
			Unknown = -1,
			Background,
			Music,
			SoundEffect,
			Mask,
			Character,
			Choice,
			Delay,
			VisualEffect,
			Interlude,
			ColorMask,
			Shake,
			FreeText,
			Cut,
			Filter,
			AnimatedText,
			Sign,
			Variable,
			Judge,
			Branch,
			Dialog,
			Preprocessor,
			Comment
		};
		Q_ENUM(ControllerType);
	public:
		AStoryXController();
		VIMoveable(AStoryXController);
		VICopyable(AStoryXController);
		~AStoryXController();
	public:
		bool parseRule(const Visindigo::Utility::JsonConfig& ruleConfig, const Visindigo::Utility::JsonConfig& metaConfig);
		AStoryXControllerParseData parseAStoryX(const QString& str, qint32 cursorPosition = -1, bool diagnostic = false, qint32 lineIndex = -1);
		ControllerType getControllerType();
		QString getControllerTypeString();
		QString getStartSign();
		QString getHeader();
		QString getRequiredParameterName();
		QString getRequiredParameterSeparater();
		QStringList getOptionalParameterNames();
		QStringList getOptionalParameterPrefixes();
		QMap<QString, AStoryXValueMeta> getOptionalParameterValues();
		AStoryXValueMeta getRequiredParameterValue();
		bool isMonotonicity();
		bool isAdvanced();
		bool isValid();
		QString toString();
	public:
		static QString controllerTypeToString(ControllerType type);
	private:
		AStoryXControllerPrivate* d;
	};
}
#endif // ASERStudio_AStorySyntax_AStoryXController_h
