#ifndef Visindigo_General_LogMetaData_H
#define Visindigo_General_LogMetaData_H
#include "VICompileMacro.h"
#include <QtCore/qtypes.h>
#include <QtCore/qstring.h>
namespace Visindigo::General {
	class VisindigoAPI LogMetaData final {
	private:
		QString FunctionName;
		qint32 LineNumber;
	public:
		LogMetaData();
		LogMetaData(const QString& funcName, qint32 lineNumber);
		~LogMetaData();
		QString getFunctionName() const;
		qint32 getLineNumber() const;
		void setFunctionName(const QString& funcName);
		void setLineNumber(qint32 lineNumber);
		bool isValid() const;
		QString toString() const;
	};
}

#endif // Visindigo_General_LogMetaData_H