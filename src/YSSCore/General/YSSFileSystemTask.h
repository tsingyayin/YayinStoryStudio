#ifndef YSSCore_General_YSSFileSystemTask_h
#define YSSCore_General_YSSFileSystemTask_h
#include "General/YSSFileSystemRequest.h"
#include "General/YSSURI.h"
#include "YSSCoreCompileMacro.h"
// Forward declarations
namespace YSSCore::General {
	class YSSFileSystemTaskPrivate;
	class YSSUnifiedFileSystem;
	class YSSUnifiedFileSystemPrivate;
}
// Main
namespace YSSCore::General {
	class YSSCoreAPI YSSFileSystemTask {
		friend class YSSUnifiedFileSystem;
		friend class YSSUnifiedFileSystemPrivate;
	public:
		YSSFileSystemTask();
		VICopyable(YSSFileSystemTask);
		VIMoveable(YSSFileSystemTask);
		~YSSFileSystemTask();
	public:
		quint64 getId() const;
		YSSURI getUri() const;
		YSSURI getTargetUri() const;
		YSSFileSystemRequest::Operation getOperation() const;
		qint64 getQueuedAtMs() const;
		qint64 getStartedAtMs() const;
		qint64 getBytesTransferred() const;
		qint64 getTotalBytes() const;
	public:
		bool isRunning() const;
		qreal progressPercent() const;
	protected:
		YSSFileSystemTaskPrivate* d;
	};
}
Q_DECLARE_METATYPE(YSSCore::General::YSSFileSystemTask)
#endif // YSSCore_General_YSSFileSystemTask_h
