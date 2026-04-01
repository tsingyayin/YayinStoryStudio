#ifndef ASERStudio_AStorySyntax_AStoryXControllerParseDataPrivate_h
#define ASERStudio_AStorySyntax_AStoryXControllerParseDataPrivate_h
#include <QtCore/qstring.h>
#include "AStorySyntax/AStoryXController.h"
#include "AStorySyntax/AStoryXDiagnosticData.h"

namespace ASERStudio::AStorySyntax {
	class AStoryXControllerParseDataPrivate {
		friend class AStoryXControllerParseData;
		friend class AStoryXControllerPrivate;
		friend class AStoryXController;
		friend class AStoryXPreprocessor;
		friend class AStoryXPreprocessorPrivate;
		friend class AStoryXRule;
		friend class AStoryXRulePrivate;
		friend class AStoryXDocument;
		friend class AStoryXDocumentPrivate;
	protected:
		QString RequiredParameter;
		QStringList OptionalParameters;
		QStringList OptionalParameterNames;
		AStoryXController::ControllerType ControllerType = AStoryXController::ControllerType::Unknown;
		qint32 RequiredParameterStringIndex;
		QList<qint32> OptionalParameterStringIndex;
		QString cursorInWhichParameter;
		QStringList referenceVariables;
		bool DiagnosticAvailable;
		QList<AStoryXDiagnosticData> Diagnostics;
	};
}

#endif // ASERStudio_AStorySyntax_AStoryXControllerParseDataPrivate_h