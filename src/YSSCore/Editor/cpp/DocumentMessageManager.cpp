#include "Editor/DocumentMessageManager.h"
#include "Editor/private/DocumentMessageManager_p.h"
#include "Editor/private/DocumentMessage_p.h"
#include <General/Log.h>
namespace YSSCore::__Private__ {
	YSSCore::Editor::DocumentMessageManager* DocumentMessageManagerPrivate::Instance = nullptr;

	void DocumentMessageManagerPrivate::clearMessagesForFile(const QString& filePath) {
		if (!Messages.contains(filePath)) return;
		Messages.remove(filePath);
		emit Instance->messageChanged(filePath);
	}

	void DocumentMessageManagerPrivate::clearMessagesForBlock(const QString& filePath, qint32 blockNumber) {
		if (!Messages.contains(filePath)) return;
		if (!Messages[filePath].contains(blockNumber)) return;
		Messages[filePath].remove(blockNumber);
		emit Instance->messageChangedForLine(filePath, blockNumber);
	}

	void DocumentMessageManagerPrivate::addMessage(const QString& filePath, qint32 blockNumber, const YSSCore::Editor::DocumentMessage& message) {
		Messages[filePath][blockNumber].append(message);
	}

	void DocumentMessageManagerPrivate::moveMessageForward(const QString& filePath, qint32 deletedBlockStartNumber, qint32 count) {
		if (!Messages.contains(filePath)) return;
		QMap<qint32, QList<YSSCore::Editor::DocumentMessage>> newMessages;
		for (auto it = Messages[filePath].begin(); it != Messages[filePath].end(); ++it) {
			qint32 blockNumber = it.key();
			if (blockNumber < deletedBlockStartNumber) {
				newMessages[blockNumber] = it.value();
			} else if (blockNumber >= deletedBlockStartNumber + count) {
				//vgDebug << "Moving messages for block " << blockNumber << " to " << (blockNumber - count);
				newMessages[blockNumber - count] = it.value();
				for(auto& msg : newMessages[blockNumber - count]) {
					msg.d->LineNumber -= count;
				}
			}
		}
		Messages[filePath] = newMessages;
		emit Instance->messageChanged(filePath);
	}

	void DocumentMessageManagerPrivate::moveMessageBackward(const QString& filePath, qint32 insertedBlockStartNumber, qint32 count) {
		if (!Messages.contains(filePath)) return;
		QMap<qint32, QList<YSSCore::Editor::DocumentMessage>> newMessages;
		for (auto it = Messages[filePath].begin(); it != Messages[filePath].end(); ++it) {
			qint32 blockNumber = it.key();
			if (blockNumber < insertedBlockStartNumber) {
				newMessages[blockNumber] = it.value();
			} else if (blockNumber >= insertedBlockStartNumber) {
				//vgDebug << "Moving messages for block " << blockNumber << " to " << (blockNumber + count);
				newMessages[blockNumber + count] = it.value();
				for(auto& msg : newMessages[blockNumber + count]) {
					msg.d->LineNumber += count;
				}
			}
		}
		Messages[filePath] = newMessages;
		emit Instance->messageChanged(filePath);
	}

	void DocumentMessageManagerPrivate::flushMessages(const QString& filePath, qint32 blockNumber) {
		if (!Messages.contains(filePath)) return;
		if (!Messages[filePath].contains(blockNumber)) return;
		emit Instance->messageChangedForLine(filePath, blockNumber);
	}

	void DocumentMessageManagerPrivate::flushMessages(const QString& filePath) {
		if (!Messages.contains(filePath)) return;
		emit Instance->messageChanged(filePath);
	}

	bool DocumentMessageManagerPrivate::hasMessage(const QString& filePath) {
		return Messages.contains(filePath) && !Messages[filePath].isEmpty();
	}

	bool DocumentMessageManagerPrivate::hasMessage(const QString& filePath, qint32 lineNumber) {
		return Messages.contains(filePath) && Messages[filePath].contains(lineNumber) && !Messages[filePath][lineNumber].isEmpty();
	}

	QMap<qint32, QList<YSSCore::Editor::DocumentMessage>> DocumentMessageManagerPrivate::getAllMessages(const QString& filePath) {
		if (!Messages.contains(filePath)) return {};
		return Messages[filePath];
	}

	QList<YSSCore::Editor::DocumentMessage> DocumentMessageManagerPrivate::getMessages(const QString& filePath, qint32 lineNumber) {
		if (!Messages.contains(filePath)) return {};
		if (!Messages[filePath].contains(lineNumber)) return {};
		return Messages[filePath][lineNumber];
	}
}

namespace YSSCore::Editor {
	DocumentMessageManager::DocumentMessageManager() {
		d = new YSSCore::__Private__::DocumentMessageManagerPrivate();
	}
	DocumentMessageManager::~DocumentMessageManager() {
		delete d;
	}
	DocumentMessageManager* DocumentMessageManager::getInstance() {
		if (YSSCore::__Private__::DocumentMessageManagerPrivate::Instance == nullptr) {
			YSSCore::__Private__::DocumentMessageManagerPrivate::Instance = new DocumentMessageManager();
		}
		return YSSCore::__Private__::DocumentMessageManagerPrivate::Instance;
	}
	bool DocumentMessageManager::hasMessage(const QString& filePath) {
		return d->hasMessage(filePath);
	}
	bool DocumentMessageManager::hasMessage(const QString& filePath, qint32 lineNumber) {
		return d->hasMessage(filePath, lineNumber);
	}
	QMap<qint32, QList<DocumentMessage>> DocumentMessageManager::getAllMessages(const QString& filePath) {
		return d->getAllMessages(filePath);
	}
	QList<DocumentMessage> DocumentMessageManager::getMessages(const QString& filePath, qint32 lineNumber) {
		return d->getMessages(filePath, lineNumber);
	}
}