#ifndef Visindigo_General_StacktraceHelper_H
#define Visindigo_General_StacktraceHelper_H
#include "../Macro.h"
#include <QtCore/qlist.h>
class QString;
namespace Visindigo::General {
	class StacktraceFramePrivate;
	class VisindigoAPI StacktraceFrame {
		VIMoveable(StacktraceFrame);
		VICopyable(StacktraceFrame);
	public:
		StacktraceFrame();
		StacktraceFrame(const QString& functionName, const QString& sourceFileName, const QString& binaryFileName, quint64 address, quint64 lineNumber);
		~StacktraceFrame();
		QString getFunctionName() const;
		QString getSourceFileName() const;
		QString getBinaryFileName() const;
		quint64 getAddress() const;
		quint64 getLineNumber() const;
		QString toString() const;
	private:
		StacktraceFramePrivate* d;
	};

	class VisindigoAPI StacktraceHelper final {
	public:
		static QList<StacktraceFrame> getStacktrace(qint32 maxFrames = 63, qint32 skipFrames = 0);
	};
}
#endif // Visindigo_General_StacktraceHelper_H