#ifndef ASERStudio_AStorySyntax_AStoryXControllerParseData_h
#define ASERStudio_AStorySyntax_AStoryXControllerParseData_h
#include "ASERStudioCompileMacro.h"
#include "AStorySyntax/AStoryXController.h"
#include "AStorySyntax/AStoryXDiagnosticData.h"
#include <VIMacro.h>
#include "QtCore/qstringlist.h"

namespace ASERStudio::AStorySyntax {
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
		AStoryXController::ControllerType getControllerType() const;
		qint32 getRequiredParameterStringIndex() const;
		QList<qint32> getOptionalParameterStringIndexes() const;
		QString getCursorInWhichParameter(qint32 cursorPosition = -1) const;
		QString getRequiredParameter() const;
		QStringList getOptionalParameters() const;
		QStringList getOptionalParameterNames() const;
		QStringList referenceVariables() const;
		qint32 getOptionalParameterCount() const;
		bool isDiagnosticAvailable() const;
		QList<AStoryXDiagnosticData> getDiagnostics() const;
		QString toString() const;
		bool isValid() const;
	protected:
		AStoryXControllerParseDataPrivate* d;
	};
}

#endif