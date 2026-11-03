#ifndef YSSCore_General_private_YSSFileSystemTask_p_h
#define YSSCore_General_private_YSSFileSystemTask_p_h
#include "General/YSSFileSystemTask.h"
#include "General/YSSFileSystemRequest.h"
#include "General/YSSURI.h"
// Forward declarations
namespace YSSCore::General {
	class YSSUnifiedFileSystem;
	class YSSUnifiedFileSystemPrivate;
}
// Main
namespace YSSCore::General {
	class YSSFileSystemTaskPrivate {
		friend class YSSFileSystemTask;
		friend class YSSUnifiedFileSystem;
		friend class YSSUnifiedFileSystemPrivate;
	protected:
		quint64 Id = 0;
		YSSURI Uri;
		YSSURI TargetUri;
		YSSFileSystemRequest::Operation Operation = YSSFileSystemRequest::Operation::Unknown;
		qint64 QueuedAtMs = 0;
		qint64 StartedAtMs = 0;
		qint64 BytesTransferred = 0;
		qint64 TotalBytes = -1;
	};
}
#endif // YSSCore_General_private_YSSFileSystemTask_p_h
