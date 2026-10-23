#ifndef Plugin_ASERStudio_AStorySyntax_AStoryXControllerParseData_h
#define Plugin_ASERStudio_AStorySyntax_AStoryXControllerParseData_h
#include "ASERStudioCompileMacro.h"
#include "AStorySyntax/AStoryXController.h"
#include "AStorySyntax/AStoryXDiagnosticData.h"
#include <VIMacro.h>
#include "QtCore/qstringlist.h"

namespace ASERStudio::AStorySyntax {
	class ASERAPI AStoryXParameterPrivate;
	class ASERAPI AStoryXParameter {
		friend class AStoryXController;
		friend class AStoryXControllerPrivate;
		friend class AStoryXPreprocessor;
		friend class AStoryXPreprocessorPrivate;
		friend class AStoryXRule;
		friend class AStoryXRulePrivate;
		friend class AStoryXDocument;
		friend class AStoryXDocumentPrivate;
	public:
		AStoryXParameter();
		VIMoveable(AStoryXParameter);
		VICopyable(AStoryXParameter);
		~AStoryXParameter();
	public:
		const QString& getName() const;
		const QString& getPrefix() const;
		const QString& getSeparator() const;
		const QString& getContent() const;
		const AStoryXValueMeta& getValue() const;
		qint32 getIndex() const;
		bool isValid() const;
		QString toString() const;
	protected:
		AStoryXParameterPrivate* d;
	};

	class AStoryXControllerParseDataPrivate;
	class ASERAPI AStoryXControllerParseData {
		friend class AStoryXController;
		friend class AStoryXControllerPrivate;
		friend class AStoryXPreprocessor;
		friend class AStoryXPreprocessorPrivate;
		friend class AStoryXRule;
		friend class AStoryXRulePrivate;
		friend class AStoryXDocument;
		friend class AStoryXDocumentPrivate;
	public:
		AStoryXControllerParseData();
		VIMoveable(AStoryXControllerParseData);
		VICopyable(AStoryXControllerParseData);
		~AStoryXControllerParseData();
	public:
		const QString& getStartSign() const;
		AStoryXController::ControllerType getControllerType() const;
		const AStoryXParameter& getRequiredParameter() const;
		const QList<AStoryXParameter>& getOptionalParameters() const;
		QString getCursorInWhichParameter(qint32 cursorPosition) const;
		AStoryXParameter getCursorParameter(qint32 cursorPosition) const;
		const QStringList& referenceVariables() const;
		const QStringList& blockParameters() const;
		bool isDiagnosticAvailable() const;
		const QList<AStoryXDiagnosticData>& getDiagnostics() const;
		QString toString() const;
		bool isValid() const;
	protected:
		AStoryXControllerParseDataPrivate* d;
	};
}

#endif // Plugin_ASERStudio_AStorySyntax_AStoryXControllerParseData_h
