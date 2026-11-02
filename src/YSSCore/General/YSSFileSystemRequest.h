#ifndef YSSCore_General_YSSFileSystemRequest_h
#define YSSCore_General_YSSFileSystemRequest_h
#include "General/YSSURI.h"
#include "YSSCoreCompileMacro.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qobjectdefs.h>
// Forward declarations
namespace YSSCore::General {
	class YSSFileSystemRequestPrivate;
	class YSSUnifiedFileSystemPrivate;
	class YSSUnifiedFileSystemProvider;
}
// Main
namespace YSSCore::General {
	class YSSCoreAPI YSSFileSystemRequest {
		Q_GADGET;
	public:
		enum class Operation {
			Unknown = 0,
			Stat,
			Read,
			Write,
			List,
			Remove,
			Recycle,
			Rename,
			Copy,
			Move,
			Mkdir,
			Rmdir
		};
		Q_ENUM(Operation)
	public:
		YSSFileSystemRequest();
		VICopyable(YSSFileSystemRequest);
		VIMoveable(YSSFileSystemRequest);
		~YSSFileSystemRequest();
	public:
		YSSFileSystemRequest& setOperation(Operation operation);
		YSSFileSystemRequest& setUri(const YSSURI& uri);
		YSSFileSystemRequest& setTargetUri(const YSSURI& uri);
		YSSFileSystemRequest& setData(const QByteArray& data);
		YSSFileSystemRequest& setAutoDelete(bool autoDelete);
	public:
		Operation getOperation() const;
		YSSURI getUri() const;
		YSSURI getTargetUri() const;
		bool hasUri() const;
		bool hasTargetUri() const;
		QByteArray getData() const;
		bool isAutoDelete() const;
	public:
		bool isValid() const;
	private:
		YSSFileSystemRequestPrivate* d;
	};
}
#endif // YSSCore_General_YSSFileSystemRequest_h
