#ifndef ASERStudio_AStorySyntax_AStoryXDiagnosticData_h
#define ASERStudio_AStorySyntax_AStoryXDiagnosticData_h
#include "ASERStudioCompileMacro.h"
#include "AStorySyntax/AStoryXController.h"
#include <VIMacro.h>
#include "QtCore/qstringlist.h"

namespace ASERStudio::AStorySyntax {
	class AStoryXDiagnosticDataPrivate;
	class ASERAPI AStoryXDiagnosticData {
	public:
		enum DiagnosticType {
			Undefined = 0x0000,
			UnknownError = 0x1000,
			RuleNotSelected,
			NoSuchRule,
			NoSuchController,
			MissingRequiredParameter,
			ParameterTypeMismatch,
			ParameterOutOfRange,
			ParameterFormatError,
			UnexpectedParameter,
			UnclosedSymbol,
			VariableNotDefined,
			UseTabInsteadSpace,
			UseSpaceInsteadTab,
			UseEnglishPunctuation,
			UseChinesePunctuation,
			NoSuchBlock,
			BlockNameConflict,
			InstrumentRedundant,
			NoSuchPreprocessor,

			UnknownWarning = 0x2000,
			NoSuchResource,
		};
	public:
		AStoryXDiagnosticData();
		AStoryXDiagnosticData(const QString& message, qint32 line, qint32 column, DiagnosticType type = DiagnosticType::Undefined, const QString& fixAdvice = "");
		VIMoveable(AStoryXDiagnosticData);
		VICopyable(AStoryXDiagnosticData);
		~AStoryXDiagnosticData();
	public:
		bool operator==(const AStoryXDiagnosticData& other) const;
	public:
		QString getMessage() const;
		qint32 getLine() const;
		qint32 getColumn() const;
		QString getFixAdvice() const;
		AStoryXDiagnosticData::DiagnosticType getType() const;
	private:
		AStoryXDiagnosticDataPrivate* d;
	};
}

#endif 