#pragma once
#include "DiffSequence.h"
#include <QtCore/qstring.h>
// Forward declarations
namespace Visindigo::Utility {
	class CodeLinePrivate;
}
// Main
namespace Visindigo::Utility {
	class VisindigoAPI CodeLine :public DiffUnit {
	public:
		CodeLine(const QString& str);
		virtual ~CodeLine();
		virtual CompareType onCompare(const DiffUnit* other) override;
		QString getLine();
	protected:
		CodeLinePrivate* d;
	};

	class CodeDiffPrivate;
	class VisindigoAPI CodeDiff {
	public:
		CodeDiff();
		~CodeDiff();
		void setOldCode(QStringList oldCode);
		void setNewCode(QStringList newCode);
		DiffSequence* getOldCode();
		DiffSequence* getNewCode();
		void compare();
		void debugPrint();
	private:
		CodeDiffPrivate* d;
	};
}