#include <QtCore/qlist.h>
#include <Editor/DocumentMessage.h>
#include <Editor/TextEdit.h>
#include "Editor/MainEditor/DocumentMessageTracer.h"
#include "Editor/MainEditor/MainWin.h"
#include "Editor/MainEditor/MessageCenter.h"

namespace YSS::Editor {
	/*!
		\class YSS::Editor::MessageCenter
		\brief 主程序的消息响应层。

		它以单例常驻，跟踪当前聚焦的非工具编辑器，把它发出的消息变化转发给消息查看器，并维护当前文件的
		错误、警告、信息数量。消息查看器与底部的计数显示都只查这一层，不再各自去连编辑器。

		每一条带消息的行上都有一个 YSS::Editor::DocumentMessageTracer，这一行的消息数量记在它上面，
		所以总数是随消息增删加减出来的；行被插入、删除或合并时它跟着块走，统计不受影响，
		也不需要为了统计重新遍历文档。
	*/
	MessageCenter* MessageCenter::Instance = nullptr;

	namespace {
		// 把一行的消息按类型数一遍。
		void countMessages(const QList<YSSCore::Editor::DocumentMessage>& messages,
			qint32& error, qint32& warning, qint32& info) {
			error = 0;
			warning = 0;
			info = 0;
			for (const YSSCore::Editor::DocumentMessage& message : messages) {
				switch (message.getType()) {
				case YSSCore::Editor::DocumentMessage::MessageType::Error:
					error++;
					break;
				case YSSCore::Editor::DocumentMessage::MessageType::Warning:
					warning++;
					break;
				case YSSCore::Editor::DocumentMessage::MessageType::Info:
					info++;
					break;
				}
			}
		}
	}

	MessageCenter* MessageCenter::getInstance() {
		if (Instance == nullptr) {
			Instance = new MessageCenter();
		}
		return Instance;
	}

	MessageCenter::MessageCenter() {
		connect(MainWin::getInstance(), &MainWin::currentFileEditWidgetChangedNotTool,
			this, &MessageCenter::onCurrentFileEditWidgetChanged);
	}

	MessageCenter::~MessageCenter() {
	}

	YSSCore::Editor::TextEdit* MessageCenter::getCurrentEdit() const {
		return CurrentEdit;
	}

	std::tuple<qint32, qint32, qint32> MessageCenter::getMessageCount() const {
		return Count;
	}

	DocumentMessageTracer* MessageCenter::getTracer(qint32 lineNumber) const {
		if (CurrentEdit == nullptr || lineNumber < 0) {
			return nullptr;
		}
		return CurrentEdit->getBlockData<DocumentMessageTracer>(lineNumber);
	}

	QList<DocumentMessageTracer*> MessageCenter::getTracers() const {
		return Tracers.values();
	}

	void MessageCenter::onCurrentFileEditWidgetChanged(YSSCore::Editor::FileEditWidget* widget) {
		YSSCore::Editor::TextEdit* edit = qobject_cast<YSSCore::Editor::TextEdit*>(widget);
		if (CurrentEdit == edit) {
			return;
		}
		if (CurrentEdit != nullptr) {
			disconnect(CurrentEdit, nullptr, this, nullptr);
		}
		CurrentEdit = edit;
		// 换文件：上一个文档上的 tracer 仍挂在它的块上，但不再计入本文件（它们的信号会被 Tracers 挡掉）。
		Tracers.clear();
		Count = { 0, 0, 0 };
		if (CurrentEdit != nullptr) {
			connect(CurrentEdit, &YSSCore::Editor::TextEdit::messageChanged, this, &MessageCenter::onMessageChanged);
			connect(CurrentEdit, &YSSCore::Editor::TextEdit::messageChangedForLine, this, &MessageCenter::onMessageChangedForLine);
			// 这个文档可能以前就被打开过（块上的 tracer 还在），所以是按块上现有的东西重建一次。
			const QMap<qint32, QList<YSSCore::Editor::DocumentMessage>> all = CurrentEdit->getAllMessages();
			for (auto it = all.begin(); it != all.end(); ++it) {
				qint32 error = 0;
				qint32 warning = 0;
				qint32 info = 0;
				countMessages(it.value(), error, warning, info);
				DocumentMessageTracer* tracer = ensureTracer(it.key());
				tracer->setShownLine(it.key());
				applyLineCount(tracer, error, warning, info);
			}
		}
		emit currentEditChanged(CurrentEdit);
		emit messageCountChanged();
	}

	void MessageCenter::onMessageChanged() {
		// 行结构变了：让每条带消息的行核对一下自己的行号，搬过家的会发 tracerLineChanged，
		// 界面只改那几个行号。被删掉的行，它的 tracer 随块一起析构并通知（tracerDestroyed）。
		// 统计不在这里重算——行移动不改变总数，块消失由 tracer 的析构负责扣减。
		const QList<DocumentMessageTracer*> tracers = Tracers.values();
		for (DocumentMessageTracer* tracer : tracers) {
			tracer->syncBlockNumber();
		}
	}

	void MessageCenter::onMessageChangedForLine(qint32 lineNumber) {
		if (CurrentEdit != nullptr && lineNumber >= 0) {
			qint32 error = 0;
			qint32 warning = 0;
			qint32 info = 0;
			countMessages(CurrentEdit->getMessages(lineNumber), error, warning, info);
			DocumentMessageTracer* tracer = getTracer(lineNumber);
			if (error + warning + info == 0) {
				if (tracer != nullptr) {
					CurrentEdit->removeBlockData<DocumentMessageTracer>(lineNumber);
				}
			}
			else {
				if (tracer == nullptr) {
					tracer = ensureTracer(lineNumber);
				}
				tracer->setShownLine(lineNumber);
				applyLineCount(tracer, error, warning, info);
			}
		}
		emit messageChangedForLine(lineNumber);
	}

	DocumentMessageTracer* MessageCenter::ensureTracer(qint32 lineNumber) {
		DocumentMessageTracer* tracer = getTracer(lineNumber);
		if (tracer == nullptr) {
			tracer = new DocumentMessageTracer();
			connect(tracer, &DocumentMessageTracer::lineChanged, this, [this, tracer](qint32 oldLine, qint32 newLine) {
				onTracerLineChanged(tracer, oldLine, newLine);
				});
			CurrentEdit->setBlockData<DocumentMessageTracer>(lineNumber, tracer);
		}
		if (not Tracers.contains(tracer)) {
			Tracers.insert(tracer);
		}
		return tracer;
	}

	// 行上的计数变了：只按差值调整总数，不重算整个文档。
	void MessageCenter::applyLineCount(DocumentMessageTracer* tracer, qint32 error, qint32 warning, qint32 info) {
		if (tracer == nullptr) {
			return;
		}
		qint32 deltaError = error - tracer->getErrorCount();
		qint32 deltaWarning = warning - tracer->getWarningCount();
		qint32 deltaInfo = info - tracer->getInfoCount();
		if (deltaError == 0 && deltaWarning == 0 && deltaInfo == 0) {
			return;
		}
		tracer->setMessageCount(error, warning, info);
		Count = { std::get<0>(Count) + deltaError, std::get<1>(Count) + deltaWarning, std::get<2>(Count) + deltaInfo };
		emit messageCountChanged();
	}

	void MessageCenter::onTracerLineChanged(DocumentMessageTracer* tracer, qint32 oldLine, qint32 newLine) {
		if (not Tracers.contains(tracer)) {
			return;
		}
		emit tracerLineChanged(tracer, oldLine, newLine);
	}

	void MessageCenter::onTracerDestroyed(DocumentMessageTracer* tracer) {
		if (not Tracers.remove(tracer)) {
			return;
		}
		qint32 deltaError = tracer->getErrorCount();
		qint32 deltaWarning = tracer->getWarningCount();
		qint32 deltaInfo = tracer->getInfoCount();
		if (deltaError != 0 || deltaWarning != 0 || deltaInfo != 0) {
			Count = { std::get<0>(Count) - deltaError, std::get<1>(Count) - deltaWarning, std::get<2>(Count) - deltaInfo };
			emit messageCountChanged();
		}
		emit tracerDestroyed(tracer);
	}
}
