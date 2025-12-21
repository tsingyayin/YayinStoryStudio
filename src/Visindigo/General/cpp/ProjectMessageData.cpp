#include "../ProjectMessageData.h"

namespace Visindigo::General {
	class ProjectMessageDataPrivate {
		friend class ProjectMessageData;
	protected:
		ProjectMessageData::MessageType Type = ProjectMessageData::MessageType::Unknown;
		QString ErrorCode;
		QString Description;
		qint32 LineIndex = -1;
		QString AbsFilePath;
		QUrl HelperUrl;
	};

	/*!
		\class Visindigo::General::ProjectMessageData
		\brief 项目消息数据类，表示项目中的一条消息。
		\inmodule Visindigo
		\since Visindigo 0.13.0

		\warning 此类没有完全开发，也没有更好的优化方案，随时可能被废弃或替换。
	*/
	ProjectMessageData::ProjectMessageData() {
		d = new ProjectMessageDataPrivate();
	}
	ProjectMessageData::ProjectMessageData(const ProjectMessageData& other) {
		d = new ProjectMessageDataPrivate();
		d->Type = other.d->Type;
		d->ErrorCode = other.d->ErrorCode;
		d->Description = other.d->Description;
		d->LineIndex = other.d->LineIndex;
		d->AbsFilePath = other.d->AbsFilePath;
		d->HelperUrl = other.d->HelperUrl;
	}
	ProjectMessageData::ProjectMessageData(ProjectMessageData&& other) noexcept {
		d = other.d;
		other.d = nullptr;
	}
	ProjectMessageData::ProjectMessageData(ProjectMessageData::MessageType type, const QString& errorCode, const QString& description,
		qint32 line_index, const QString& abs_filePath, const QUrl& helperUrl) {
		d = new ProjectMessageDataPrivate();
		d->Type = type;
		d->ErrorCode = errorCode;
		d->Description = description;
		d->LineIndex = line_index;
		d->AbsFilePath = abs_filePath;
		d->HelperUrl = helperUrl;
	}
	ProjectMessageData::~ProjectMessageData() {
		delete d;
	}
	ProjectMessageData& ProjectMessageData::operator=(const ProjectMessageData& other) {
		if (this != &other) {
			d->Type = other.d->Type;
			d->ErrorCode = other.d->ErrorCode;
			d->Description = other.d->Description;
			d->LineIndex = other.d->LineIndex;
			d->AbsFilePath = other.d->AbsFilePath;
			d->HelperUrl = other.d->HelperUrl;
		}
		return *this;
	}
	ProjectMessageData& ProjectMessageData::operator=(ProjectMessageData&& other) noexcept {
		if (this != &other) {
			if (d != nullptr) {
				delete d;
			}
			d = other.d;
			other.d = nullptr;
		}
		return *this;
	}
	bool ProjectMessageData::operator==(const ProjectMessageData& other) const {
		return d->Type == other.d->Type &&
			d->ErrorCode == other.d->ErrorCode &&
			d->Description == other.d->Description &&
			d->LineIndex == other.d->LineIndex &&
			d->AbsFilePath == other.d->AbsFilePath &&
			d->HelperUrl == other.d->HelperUrl;
	}
	bool ProjectMessageData::operator!=(const ProjectMessageData& other) const {
		return !(*this == other);
	}
	ProjectMessageData::MessageType ProjectMessageData::getMessageType() const {
		return d->Type;
	}
	void ProjectMessageData::setMessageType(ProjectMessageData::MessageType type) {
		d->Type = type;
	}
	QString ProjectMessageData::getErrorCode() const {
		return d->ErrorCode;
	}
	void ProjectMessageData::setErrorCode(const QString& errorCode) {
		d->ErrorCode = errorCode;
	}
	QString ProjectMessageData::getDescription() const {
		return d->Description;
	}
	void ProjectMessageData::setDescription(const QString& description) {
		d->Description = description;
	}
	qint32 ProjectMessageData::getLineIndex() const {
		return d->LineIndex;
	}
	void ProjectMessageData::setLineIndex(qint32 line_index) {
		d->LineIndex = line_index;
	}
	QString ProjectMessageData::getFilePath() const {
		return d->AbsFilePath;
	}
	void ProjectMessageData::setFilePath(const QString& abs_filePath) {
		d->AbsFilePath = abs_filePath;
	}
	QUrl ProjectMessageData::getHelperUrl() const {
		return d->HelperUrl;
	}
	void ProjectMessageData::setHelperUrl(const QUrl& helperUrl) {
		d->HelperUrl = helperUrl;
	}
}