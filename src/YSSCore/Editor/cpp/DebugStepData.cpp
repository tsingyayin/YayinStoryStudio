#include "Editor/DebugStepData.h"
#include <QtCore/qstring.h>

namespace YSSCore::Editor {
	class DebugStepDataPrivate {
		friend class DebugStepData;
	protected:
		QString filePath;
		qint32 lineCount;
		qint32 cursorColumn;
		qint32 selectedLength;
	};

	DebugStepData::DebugStepData(const QString& filePath, qint32 lineCount, qint32 column, qint32 selected)
		: d(new DebugStepDataPrivate()) {
		d->filePath = filePath;
		d->lineCount = lineCount;
		d->cursorColumn = column;
		d->selectedLength = selected;
	}

	DebugStepData::~DebugStepData() {
		delete d;
	}

	VIMoveable_Impl(DebugStepData);
	VICopyable_Impl(DebugStepData);

	void DebugStepData::setFilePath(const QString& filePath) {
		d->filePath = filePath;
	}
	void DebugStepData::setLineCount(qint32 lineCount) {
		d->lineCount = lineCount;
	}
	void DebugStepData::setCursorPosition(qint32 column, qint32 selected) {
		d->cursorColumn = column;
		d->selectedLength = selected;
	}
	QString DebugStepData::getFilePath() const {
		return d->filePath;
	}
	qint32 DebugStepData::getLineCount() const {
		return d->lineCount;
	}
	qint32 DebugStepData::getCursorColumn() const {
		return d->cursorColumn;
	}
}  // namespace YSSCore::Editor