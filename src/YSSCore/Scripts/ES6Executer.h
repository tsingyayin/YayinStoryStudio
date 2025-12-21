#ifndef YSSCore_Scripts_ES6Executer_h
#define YSSCore_Scripts_ES6Executer_h
#include "../Macro.h"
#include <QtCore/qobject.h>

namespace YSSCore::Scripts {
	class ES6ExecuterPrivate;
	class YSSCoreAPI ES6Executer :public QObject {
		Q_OBJECT;
	signals:
		void finished(int rtn);
	public:
		ES6Executer(QObject* parent = nullptr) {};
		~ES6Executer() {};
		int exec(const QString& scriptText) { return 0; };
		int execFile(const QString& scriptFilePath) { return 0; };
		void execAsync(const QString& scriptText) {};
		void execFileAsync(const QString& scriptFilePath) {};
	};
}
#endif // YSSCore_Scripts_ES6Executer_h