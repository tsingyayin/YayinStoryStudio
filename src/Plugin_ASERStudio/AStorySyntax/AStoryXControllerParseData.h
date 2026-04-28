#ifndef ASERStudio_AStorySyntax_AStoryXControllerParseData_h
#define ASERStudio_AStorySyntax_AStoryXControllerParseData_h
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
		QString getName() const;
		QString getPrefix() const;
		QString getSeparator() const;
		QString getContent() const;
		AStoryXValueMeta getValue() const;
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
		QString getStartSign() const;
		AStoryXController::ControllerType getControllerType() const;
		AStoryXParameter getRequiredParameter() const;
		QList<AStoryXParameter> getOptionalParameters() const;
		QString getCursorInWhichParameter(qint32 cursorPosition) const;
		AStoryXParameter getCursorParameter(qint32 cursorPosition) const;
		QStringList referenceVariables() const;
		QStringList blockParameters() const;
		bool isDiagnosticAvailable() const;
		QList<AStoryXDiagnosticData> getDiagnostics() const;
		QString toString() const;
		bool isValid() const;
	protected:
		AStoryXControllerParseDataPrivate* d;
	};
}

#endif