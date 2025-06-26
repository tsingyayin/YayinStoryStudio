#pragma once
#include "../Macro.h"
#include <QtCore/qtypes.h>
#include <QtCore/qstring.h>
#include <QtCore/qurl.h>

namespace YSSCore::General {
	class ProjectMessageCollector;
	class ProjectMessageDataPrivate;
	class YSSCoreAPI ProjectMessageData {
	public:
		enum MessageType {
			Unknown = 0,
			EditTimeError = 1,
			CompileTimeError = 2,
			RunTimeError = 3,
			EditTimeWarning = 4,
			CompileTimeWarning = 5,
			RunTimeWarning = 6,
			EditTimeInfo = 7,
			CompileTimeInfo = 8,
			RunTimeInfo = 9,
			__UserDefined__ = 1000
		};
	public:
		ProjectMessageData();
		ProjectMessageData(MessageType type, const QString& errorCode, const QString& description,
			qint32 line_index = 0, const QString& abs_filePath = "", const QUrl& helperUrl = QUrl(""));
		ProjectMessageData(const ProjectMessageData& other);
		ProjectMessageData(ProjectMessageData&& other) noexcept;
		~ProjectMessageData();
		ProjectMessageData& operator=(const ProjectMessageData& other);
		ProjectMessageData& operator=(ProjectMessageData&& other) noexcept;
		bool operator==(const ProjectMessageData& other) const;
		bool operator!=(const ProjectMessageData& other) const;
		MessageType getMessageType() const;
		QString getErrorCode() const;
		QString getDescription() const;
		qint32 getLineIndex() const;
		QString getFilePath() const;
		QUrl getHelperUrl() const;
		void setMessageType(MessageType type);
		void setErrorCode(const QString& errorCode);
		void setDescription(const QString& description);
		void setLineIndex(qint32 line_index);
		void setFilePath(const QString& abs_filePath);
		void setHelperUrl(const QUrl& helperUrl);
	private:
		ProjectMessageDataPrivate* d;
	};
}