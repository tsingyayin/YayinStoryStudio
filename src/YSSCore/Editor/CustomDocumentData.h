#ifndef YSSCore_Editor_CustomDocumentData_h
#define YSSCore_Editor_CustomDocumentData_h
#include "YSSCoreCompileMacro.h"
#include <QtCore/qglobal.h>
#include <QtGui/qtextobject.h>
// Forward declarations
namespace YSSCore::Editor {
	class CustomDataProxy;
}
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI ICustomDocumentData {
		friend class CustomDataProxy;
	public:
		ICustomDocumentData();
		virtual ~ICustomDocumentData();
	public:
		QTextBlock getBlock() const;
		qint32 getBlockNumber() const;
		qint32 getLastBlockNumber() const;
		bool isBlockNumberChanged() const;
		void syncBlockNumber();
	public:
		virtual void onBlockNumberChanged(qint32 newBlockNumber);
	protected:
		void setBlock(const QTextBlock& block);
	private:
		QTextBlock Block;
		qint32 LastBlockNumber = -1;
	};
}
#endif // YSSCore_Editor_CustomDocumentData_h
