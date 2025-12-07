#pragma once
#include <QtCore/qtypes.h>
#include "../Macro.h"

class QString;

namespace YSSCore::Editor
{
	class DebugFailedDataPrivate;
	class YSSCoreAPI DebugFailedData {
	public:
		DebugFailedData();
		~DebugFailedData();
		DebugFailedData(const DebugFailedData& other);
		DebugFailedData(DebugFailedData&& other);
		DebugFailedData& operator=(const DebugFailedData& other);
		DebugFailedData& operator=(DebugFailedData&& other);
		void setIndex(const QString& fileName, quint32 line, quint32 chr = 0);
		void setTitle(const QString& title);
		void setDescription(const QString& desc);
		void setStackTrace(const QStringList& lines);
		QString getFileName() const;
		quint32 getLine() const;
		quint32 getChar() const;
		QString getTitle() const;
		QString getDescription() const;
		QStringList getStackTrace() const;
	private:
		DebugFailedDataPrivate* d;
	};
}