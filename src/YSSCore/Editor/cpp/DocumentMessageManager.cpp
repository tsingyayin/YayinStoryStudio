#include "Editor/DocumentMessageManager.h"
#include "Editor/private/DocumentMessageManager_p.h"
#include "Editor/DocumentMessage.h"
#include <General/Log.h>
namespace YSSCore::__Private__ {
	YSSCore::Editor::DocumentMessageManager* DocumentMessageManagerPrivate::Instance = nullptr;

	void DocumentMessageManagerPrivate::clearMessagesForFile(const QString& filePath) {
		if (!Messages.contains(filePath)) return;
		Messages.remove(filePath);
		MessageCounts.remove(filePath);
		emit Instance->messageChanged(filePath);
	}

	void DocumentMessageManagerPrivate::clearMessagesForBlock(const QString& filePath, qint32 blockNumber) {
		if (!Messages.contains(filePath)) return;
		if (!Messages[filePath].contains(blockNumber)) return;
		qint32 errorCount = 0, warningCount = 0, infoCount = 0;
		for (const auto& msg : Messages[filePath][blockNumber]) {
			switch (msg.getType()) {
			case Editor::DocumentMessage::MessageType::Error:
				errorCount++;
				break;
			case Editor::DocumentMessage::MessageType::Warning:
				warningCount++;
				break;
			case Editor::DocumentMessage::MessageType::Info:
				infoCount++;
				break;
			}
		}
		std::get<0>(MessageCounts[filePath]) -= errorCount;
		std::get<1>(MessageCounts[filePath]) -= warningCount;
		std::get<2>(MessageCounts[filePath]) -= infoCount;
		Messages[filePath].remove(blockNumber);
		emit Instance->messageChangedForLine(filePath, blockNumber);
	}

	void DocumentMessageManagerPrivate::addMessage(const QString& filePath, qint32 blockNumber, const YSSCore::Editor::DocumentMessage& message) {
		Messages[filePath][blockNumber].append(message);
		switch (message.getType())
		{
			case Editor::DocumentMessage::MessageType::Error:
				std::get<0>(MessageCounts[filePath])++;
				break;
			case Editor::DocumentMessage::MessageType::Warning:
				std::get<1>(MessageCounts[filePath])++;
				break;
			case Editor::DocumentMessage::MessageType::Info:
				std::get<2>(MessageCounts[filePath])++;
				break;
		}
	}

	void DocumentMessageManagerPrivate::moveMessageForward(const QString& filePath, qint32 deletedBlockStartNumber, qint32 count) {
		if (!Messages.contains(filePath)) return;
		QMap<qint32, QList<YSSCore::Editor::DocumentMessage>> newMessages;
		for (auto it = Messages[filePath].begin(); it != Messages[filePath].end(); ++it) {
			qint32 blockNumber = it.key();
			if (blockNumber < deletedBlockStartNumber) {
				newMessages[blockNumber] = it.value();
			}
			else if (blockNumber >= deletedBlockStartNumber + count) {
				//vgDebug << "Moving messages for block " << blockNumber << " to " << (blockNumber - count);
				newMessages[blockNumber - count] = it.value();
				for (auto& msg : newMessages[blockNumber - count]) {
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
			}
			else if (blockNumber >= insertedBlockStartNumber) {
				//vgDebug << "Moving messages for block " << blockNumber << " to " << (blockNumber + count);
				newMessages[blockNumber + count] = it.value();
				for (auto& msg : newMessages[blockNumber + count]) {
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

	std::tuple<qint32, qint32, qint32> DocumentMessageManagerPrivate::getMessageCount(const QString& filePath) {
		if (not MessageCounts.contains(filePath)) return {0, 0, 0};
		return MessageCounts[filePath];
	}
}

namespace YSSCore::Editor {
	/*!
		\class YSSCore::Editor::DocumentMessageManager
		\brief 此类负责收集TextEdit在SyntaxHighligher着色过程中产生的DocumentMessage，并提供接口供外部查询这些消息。
		\since YSS 0.13.0
		\inmodule YSSCore

		DocumentMessageManager负责收集TextEdit在SyntaxHighligher着色过程中产生的DocumentMessage，
		并提供接口供外部查询这些消息。它是一个单例类，可以通过getInstance()方法获取实例。

		这个类目前不提供任何设置类API，其只提供查询功能。其内部与各TextEdit及其
		SyntaxHighlighter的交互均被隐藏。
	*/

	/*!
		\since YSS 0.13.0
		构造函数
	*/
	DocumentMessageManager::DocumentMessageManager() {
		d = new YSSCore::__Private__::DocumentMessageManagerPrivate();
	}

	/*!
		\since YSS 0.13.0
		析构函数
	*/
	DocumentMessageManager::~DocumentMessageManager() {
		delete d;
	}

	/*!
		\since YSS 0.13.0
		
		return 此类的单例
	*/
	DocumentMessageManager* DocumentMessageManager::getInstance() {
		if (YSSCore::__Private__::DocumentMessageManagerPrivate::Instance == nullptr) {
			YSSCore::__Private__::DocumentMessageManagerPrivate::Instance = new DocumentMessageManager();
		}
		return YSSCore::__Private__::DocumentMessageManagerPrivate::Instance;
	}

	/*!
		\fn bool YSSCore::Editor::DocumentMessageManager::hasMessage(const QString& filePath)
		\since YSS 0.13.0
		\a filePath 要查询文件的绝对路径

		return 指定的文件是否有文档消息
	*/
	bool DocumentMessageManager::hasMessage(const QString& filePath) {
		return d->hasMessage(filePath);
	}

	/*!
		\fn bool YSSCore::Editor::DocumentMessageManager::hasMessage(const QString& filePath, qint32 lineNumber)
		\since YSS 0.13.0
		\a filePath 要查询文件的绝对路径
		\a lineNumber 要查询文件的行号，从0开始

		return 指定的文件在指定的行是否有文档消息
	*/
	bool DocumentMessageManager::hasMessage(const QString& filePath, qint32 lineNumber) {
		return d->hasMessage(filePath, lineNumber);
	}

	/*!
		\since YSS 0.13.0
		\a filePath 要查询文件的绝对路径

		return 指定文档的全部文档消息，键为行号，值为消息列表

		如果整个文档都没有消息，则返回一个空的QMap。如果有文档消息，则只会包括那些有消息的行。
	*/
	QMap<qint32, QList<DocumentMessage>> DocumentMessageManager::getAllMessages(const QString& filePath) {
		return d->getAllMessages(filePath);
	}

	/*!
		\since YSS 0.13.0
		\a filePath 要查询文件的绝对路径
		\a lineNumber 要查询的行号，从0开始

		return 指定的行的全部文档消息

		如果这一行没有消息，则返回一个空的QList。
	*/
	QList<DocumentMessage> DocumentMessageManager::getMessages(const QString& filePath, qint32 lineNumber) {
		return d->getMessages(filePath, lineNumber);
	}


	/*!
		\since YSS 0.16.0
		\a filePath 要查询文件的绝对路径

		return 指定文件的消息计数。

		返回值是一个元组，第一个值为错误，第二个值为警告，第三个值为消息
	*/
	std::tuple<qint32, qint32, qint32> DocumentMessageManager::getMessageCount(const QString& filePath) {
		return d->getMessageCount(filePath);
	}

}