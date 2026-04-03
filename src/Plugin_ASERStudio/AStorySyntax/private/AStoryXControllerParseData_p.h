#ifndef ASERStudio_AStorySyntax_AStoryXControllerParseDataPrivate_h
#define ASERStudio_AStorySyntax_AStoryXControllerParseDataPrivate_h
#include <QtCore/qstring.h>
#include "AStorySyntax/AStoryXController.h"
#include "AStorySyntax/AStoryXDiagnosticData.h"
#include "AStorySyntax/AStoryXValueMeta.h"
namespace ASERStudio::AStorySyntax {

	class AStoryXParameterPrivate {
		friend class AStoryXParameter;
		friend class AStoryXController;
		friend class AStoryXControllerPrivate;
		friend class AStoryXPreprocessor;
		friend class AStoryXPreprocessorPrivate;
		friend class AStoryXRule;
		friend class AStoryXRulePrivate;
		friend class AStoryXDocument;
		friend class AStoryXDocumentPrivate;
	protected:
		QString Name;
		QString Prefix;
		QString Separater;
		QString Content;
		AStoryXValueMeta Value;
		qint32 Index;
		bool Valid = false;
		void setParameter(const QString& name,
			const QString& prefix,
			const QString& content,
			const AStoryXValueMeta& value,
			qint32 index,
			const QString& separater = ""
		) {
			Name = name;
			Prefix = prefix;
			Separater = separater;
			Content = content;
			Value = value;
			Index = index;
			Valid = true;
		}
	};

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
		QString StartSign;
		AStoryXParameter RequiredParameter;
		QList<AStoryXParameter> OptionalParameters;
		AStoryXController::ControllerType ControllerType = AStoryXController::ControllerType::Unknown;
		QString cursorInWhichParameter;
		QStringList referenceVariables;
		bool DiagnosticAvailable;
		QList<AStoryXDiagnosticData> Diagnostics;
	};
}

#endif // ASERStudio_AStorySyntax_AStoryXControllerParseDataPrivate_h