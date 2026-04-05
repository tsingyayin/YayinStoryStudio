#ifndef ASERStudio_AStorySyntax_AStoryXRule_h
#define ASERStudio_AStorySyntax_AStoryXRule_h
#include "ASERStudioCompileMacro.h"
#include "AStorySyntax/AStoryXControllerParseData.h"
#include "AStorySyntax/AStoryXController.h"
#include <QtCore/qstringlist.h>
#include <VIMacro.h>
namespace ASERStudio::AStorySyntax{
	class AStoryXValueMeta;
	class AStoryXRulePrivate;
	class ASERAPI AStoryXRule {
	public:
		AStoryXRule(const QString& version = "");
		~AStoryXRule();
		VIMoveable(AStoryXRule);
		VICopyable(AStoryXRule);
	public:
		QString getVersion() const;
		bool parseJson(const QString& str);
		void setName(const QString& name);
		QString getName() const;
		QString getHeader(AStoryXController::ControllerType type) const;
		QString getStartSign(AStoryXController::ControllerType type) const;
		QString getRequiredParameterName(AStoryXController::ControllerType type) const;
		QString getRequiredParameterSeparater(AStoryXController::ControllerType type) const;
		QStringList getOptionalParameterNames(AStoryXController::ControllerType type) const;
		QStringList getOptionalParameterPrefixes(AStoryXController::ControllerType type) const;
		QMap<QString, AStoryXValueMeta> getOptionalParameterValues(AStoryXController::ControllerType type) const;
		QList<AStoryXController> getAvailableControllers() const;
		QStringList getSupportedPreprocessors() const;
		AStoryXValueMeta getRequiredParameterValue(AStoryXController::ControllerType type) const;
		bool isMonotonicity(AStoryXController::ControllerType type) const;
		bool isAdvanced(AStoryXController::ControllerType type) const;
		AStoryXControllerParseData parseAStoryX(const QString& str, qint32 cursorPosition = -1, bool diagnostic = false, qint32 lineIndex = -1);
		Visindigo::Utility::JsonConfig getAStoryXControllerMetaData() const;
		bool isValid() const;

	public:
		static void registerRule(const AStoryXRule& rule);
		static AStoryXRule* getRule(const QString& name);
		static bool hasRule(const QString& name);
		static QStringList getAvailableRuleNames();
	private:
		AStoryXRulePrivate* d;
	};
}
#endif // ASERStudio_AStorySyntax_AStoryXRule_h