#ifndef Visindigo_General_LogMetaData_H
#define Visindigo_General_LogMetaData_H
#include "../Macro.h"
#include <QtCore/qtypes.h>
class QString;
namespace Visindigo::General {
	class LogMetaDataPrivate;
	class VisindigoAPI LogMetaData final {
		VIMoveable(LogMetaData);
		VICopyable(LogMetaData);
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
	private:
		LogMetaDataPrivate* d;
	};
}

#endif // Visindigo_General_LogMetaData_H