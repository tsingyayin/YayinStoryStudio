#ifndef YSSCore_General_YSSFileEntry_h
#define YSSCore_General_YSSFileEntry_h
#include "General/YSSFileAttributes.h"
#include "General/YSSURI.h"
#include "YSSCoreCompileMacro.h"
#include <QtCore/qstring.h>
// Forward declarations
namespace YSSCore::General {
	class YSSFileEntryPrivate;
}
// Main
namespace YSSCore::General {
	class YSSCoreAPI YSSFileEntry {
	public:
		YSSFileEntry();
		VICopyable(YSSFileEntry);
		VIMoveable(YSSFileEntry);
		~YSSFileEntry();
	public:
		QString getName() const;
		YSSURI getUri() const;
		YSSFileAttributes getAttributes() const;
	public:
		void setName(const QString& name);
		void setUri(const YSSURI& uri);
		void setAttributes(const YSSFileAttributes& attributes);
	public:
		bool isValid() const;
	private:
		YSSFileEntryPrivate* d;
	};
}
#endif // YSSCore_General_YSSFileEntry_h
