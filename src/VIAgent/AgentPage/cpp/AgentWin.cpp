#include <QtCore/qjsondocument.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qstandardpaths.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qtimer.h>
#include <QtGui/qcolor.h>
#include <QtGui/qevent.h>
#include <QtGui/qfont.h>
#include <QtGui/qfontinfo.h>
#include <QtGui/qicon.h>
#include <QtGui/qtextcursor.h>
#include <QtGui/qtextdocument.h>
#include <QtGui/qtextdocumentfragment.h>
#include <QtGui/qtextobject.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qscrollbar.h>
#include <QtWidgets/qtextbrowser.h>
#include <Agent/Center.h>
#include <Agent/Dialog.h>
#include <Agent/Model.h>
#include <Agent/Provider.h>
#include <General/Log.h>
#include <General/TranslationHost.h>
#include <Utility/FileOperation.h>
#include <Utility/JsonConfig.h>
#include <Widgets/BorderLabel.h>
#include "AgentPage/AgentWin.h"

namespace Visindigo::AgentPage {
	namespace {
		// 流式增量按帧到达，逐帧重排整篇会让长对话彻底卡死；
		// 攒到刷新间隔再画一次，人眼分辨不出与逐帧刷新的差别
		constexpr int RefreshIntervalMs = 80;
		// 思考过程的字号相对于正文的比例
		constexpr qreal ReasoningFontScale = 0.7;

		// 角色名直接取自枚举本身，新增角色时不需要记得补一份对照表
		QString roleName(Visindigo::Agent::Message::Role role) {
			const QMetaEnum metaEnum = QMetaEnum::fromType<Visindigo::Agent::Message::Role>();
			const char* key = metaEnum.valueToKey(static_cast<int>(role));
			return key != nullptr ? QString::fromUtf8(key) : QStringLiteral("Unknown");
		}

		QString modelConfigPath() {
			return QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
				+ QStringLiteral("/visindigo-agent-model.json");
		}
	}

	ChatInput::ChatInput(QWidget* parent) : QTextEdit(parent) {
		setAcceptRichText(false);
	}

	void ChatInput::keyPressEvent(QKeyEvent* event) {
		const bool isReturn = event->key() == Qt::Key_Return or event->key() == Qt::Key_Enter;
		if (isReturn and not event->modifiers().testFlag(Qt::ShiftModifier)) {
			emit submitted();
			return;
		}
		QTextEdit::keyPressEvent(event);
	}

	AgentWin::AgentWin() : QFrame() {
		Chat = nullptr;
		Ready = false;
		RenderedCount = -1;
		TailBlock = 0;
		this->setWindowIcon(QIcon(":/resource/cn.yxgeneral.yayinstorystudio/icon.png"));
		this->setMinimumSize(900, 600);
		this->setWindowTitle(VITRL("YSS::agent.title"));
		TitleLabel = new Visindigo::Widgets::BorderLabel(this);
		TitleLabel->setContentsMargins(10, 0, 10, 0);
		TitleLabel->setText(VITRL("YSS::agent.title"));
		TitleLabel->setObjectName("ProgramTitleLabel");
		TitleLabel->setFixedHeight(60);
		Transcript = new QTextBrowser(this);
		Transcript->setOpenExternalLinks(true);
		Input = new ChatInput(this);
		Input->setPlaceholderText(VITRL("YSS::agent.placeholder"));
		Input->setFixedHeight(80);
		SendButton = new QPushButton(VITRL("YSS::agent.send"), this);
		StatusLabel = new QLabel(this);
		StatusLabel->setWordWrap(true);
		QVBoxLayout* rootLayout = new QVBoxLayout(this);
		rootLayout->setContentsMargins(0, 0, 0, 0);
		rootLayout->setSpacing(0);
		rootLayout->addWidget(TitleLabel);
		QVBoxLayout* bodyLayout = new QVBoxLayout();
		bodyLayout->setContentsMargins(16, 12, 16, 12);
		bodyLayout->setSpacing(8);
		bodyLayout->addWidget(Transcript, 1);
		QHBoxLayout* inputLayout = new QHBoxLayout();
		inputLayout->setSpacing(8);
		inputLayout->addWidget(Input, 1);
		inputLayout->addWidget(SendButton);
		bodyLayout->addLayout(inputLayout);
		bodyLayout->addWidget(StatusLabel);
		rootLayout->addLayout(bodyLayout, 1);
		connect(SendButton, &QPushButton::clicked, this, &AgentWin::onSendClicked);
		connect(Input, &ChatInput::submitted, this, &AgentWin::onSendClicked);
		RefreshTimer = new QTimer(this);
		RefreshTimer->setSingleShot(true);
		RefreshTimer->setInterval(RefreshIntervalMs);
		connect(RefreshTimer, &QTimer::timeout, this, [this]() { refreshTranscript(); });
		setColorfulEnable(true);
		onThemeChanged();
		setupCenter();
	}

	AgentWin::~AgentWin() {
		if (Chat != nullptr) {
			Visindigo::Agent::Center::getInstance()->removeDialog(Chat->getId());
			Chat = nullptr;
		}
	}

	void AgentWin::onThemeChanged() {
		const QColor textColor = VISTM->getColor("PlainText");
		if (not textColor.isValid()) {
			return;
		}
		TextColor = textColor;
		Transcript->setStyleSheet(QStringLiteral("QTextBrowser { color: %1; background: transparent; border: none; }")
			.arg(textColor.name()));
		// 已经画好的内容里带着旧主题的颜色，必须整篇重画一遍
		RenderedCount = -1;
		refreshTranscript();
	}

	void AgentWin::setupCenter() {
		const QString configPath = modelConfigPath();
		Visindigo::Utility::FileOperation::Errorable<QString> readResult = Visindigo::Utility::FileOperation::readAll(configPath);
		if (not readResult) {
			vgError << "Failed to read agent model config:" << configPath
				<< Visindigo::Utility::FileOperation::errorCodeName(readResult.error());
			StatusLabel->setText(VITRL("YSS::agent.configError").arg(configPath));
			return;
		}
		Visindigo::Utility::JsonConfig config;
		if (config.parse(readResult.value()).error != QJsonParseError::NoError) {
			vgError << "Agent model config is not a valid JSON object:" << configPath;
			StatusLabel->setText(VITRL("YSS::agent.configError").arg(configPath));
			return;
		}
		Visindigo::Agent::Model model;
		model.fromJson(config);
		if (not model.isValid()) {
			vgError << "Agent model config is incomplete:" << configPath;
			StatusLabel->setText(VITRL("YSS::agent.configError").arg(configPath));
			return;
		}
		Visindigo::Agent::Center* center = Visindigo::Agent::Center::getInstance();
		center->addModel(model);
		Visindigo::Agent::Provider provider(model.getName());
		provider.addModel(model.getId(), 100);
		center->addProvider(provider);
		center->setDefaultProvider(provider.getId());
		Chat = center->createDialog();
		connect(Chat, &Visindigo::Agent::Dialog::streamBegan, this, &AgentWin::onStreamBegan);
		connect(Chat, &Visindigo::Agent::Dialog::streamDelta, this, &AgentWin::onStreamDelta);
		connect(Chat, &Visindigo::Agent::Dialog::streamReasoning, this, &AgentWin::onStreamReasoning);
		connect(Chat, &Visindigo::Agent::Dialog::streamEnded, this, &AgentWin::onStreamEnded);
		connect(Chat, &Visindigo::Agent::Dialog::runFailed, this, &AgentWin::onRunFailed);
		ModelName = model.getName();
		Ready = true;
		StatusLabel->setText(VITRL("YSS::agent.ready").arg(ModelName));
	}

	void AgentWin::onSendClicked() {
		if (not Ready or Chat == nullptr) {
			return;
		}
		const QString text = Input->toPlainText().trimmed();
		if (text.isEmpty()) {
			return;
		}
		Input->clear();
		Chat->appendMessage(Visindigo::Agent::Message::Role::User, text);
		RefreshTimer->stop();
		refreshTranscript();
		Chat->run();
	}

	void AgentWin::onStreamBegan() {
		StatusLabel->setText(VITRL("YSS::agent.generating"));
		RefreshTimer->stop();
		refreshTranscript();
	}

	void AgentWin::onStreamDelta(const QString& delta) {
		scheduleRefresh();
	}

	void AgentWin::onStreamReasoning(const QString& delta) {
		scheduleRefresh();
	}

	void AgentWin::onStreamEnded(const QString& fullContent) {
		RefreshTimer->stop();
		refreshTranscript();
		StatusLabel->setText(VITRL("YSS::agent.ready").arg(ModelName));
	}

	void AgentWin::onRunFailed(const QString& message) {
		RefreshTimer->stop();
		refreshTranscript();
		StatusLabel->setText(VITRL("YSS::agent.runError").arg(message));
	}

	void AgentWin::scheduleRefresh() {
		// 已经排上队就什么都不做：定时器到点自然会画一次，把期间攒下的增量一次画完
		if (not RefreshTimer->isActive()) {
			RefreshTimer->start();
		}
	}

	void AgentWin::refreshTranscript() {
		if (Chat == nullptr) {
			return;
		}
		QTextDocument* document = Transcript->document();
		QScrollBar* scrollBar = Transcript->verticalScrollBar();
		const bool stickToBottom = scrollBar->value() >= scrollBar->maximum() - 4;
		const QList<Visindigo::Agent::Message> messages = Chat->getMessages();
		if (RenderedCount < 0 or RenderedCount > messages.size()) {
			// 历史被清空或回退过，之前记下的落脚点不再作数，整篇重画
			document->clear();
			RenderedCount = 0;
			TailBlock = 0;
		}
		else {
			// 丢掉上一次留下的流式尾块，末尾会重新空出一个落脚点
			const QTextBlock tail = document->findBlockByNumber(TailBlock);
			if (tail.isValid()) {
				QTextCursor remover(tail);
				remover.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
				remover.removeSelectedText();
			}
		}
		QTextCursor cursor(document);
		cursor.movePosition(QTextCursor::End);
		for (qint32 i = RenderedCount; i < messages.size(); i++) {
			writeMessage(cursor, messages.at(i));
		}
		RenderedCount = messages.size();
		// 已收尾的内容到此为止；从这一块开始写进去的东西，下次刷新会被删掉重写
		TailBlock = document->blockCount() - 1;
		if (Chat->isStreaming()) {
			Visindigo::Agent::Message pending(Visindigo::Agent::Message::Role::Assistant, Chat->getStreamingContent());
			pending.setReasoning(Chat->getReasoningContent());
			writeMessage(cursor, pending);
		}
		if (stickToBottom) {
			scrollBar->setValue(scrollBar->maximum());
		}
	}

	void AgentWin::writeMessage(QTextCursor& cursor, const Visindigo::Agent::Message& message) {
		// 落笔处若恰好是一个空段落就直接用它：这样消息之间只隔一个空行，
		// 而且删掉流式尾块之后能原样回到同一个落脚点
		const QTextBlock block = cursor.block();
		if (not (block.text().isEmpty() and cursor.position() == block.position())) {
			cursor.insertBlock();
		}
		QTextCharFormat headerFormat;
		headerFormat.setFontWeight(QFont::Bold);
		cursor.insertText(roleName(message.getRole()), headerFormat);
		if (not message.getReasoning().isEmpty()) {
			qreal baseSize = Transcript->font().pointSizeF();
			if (baseSize <= 0) {
				baseSize = QFontInfo(Transcript->font()).pointSizeF();
			}
			QColor dimColor = TextColor.isValid() ? TextColor : QColor(Qt::gray);
			dimColor.setAlphaF(0.65);
			QTextCharFormat reasoningFormat;
			reasoningFormat.setFontPointSize(baseSize * ReasoningFontScale);
			reasoningFormat.setForeground(dimColor);
			QTextCharFormat reasoningHeaderFormat = reasoningFormat;
			reasoningHeaderFormat.setFontWeight(QFont::Bold);
			cursor.insertBlock();
			cursor.insertText(VITRL("YSS::agent.reasoning"), reasoningHeaderFormat);
			cursor.insertBlock();
			cursor.insertText(message.getReasoning(), reasoningFormat);
		}
		if (not message.getContent().isEmpty()) {
			cursor.insertBlock();
			QTextDocument source;
			source.setDefaultFont(Transcript->font());
			source.setMarkdown(message.getContent());
			const int start = cursor.position();
			cursor.insertFragment(QTextDocumentFragment(&source));
			// 片段是从临时文档搬过来的，本身不带主题色；统一压上当前正文色，
			// 免得深色主题下出现黑字黑底
			if (TextColor.isValid()) {
				QTextCursor range(cursor);
				range.setPosition(start);
				range.setPosition(cursor.position(), QTextCursor::KeepAnchor);
				QTextCharFormat colorFormat;
				colorFormat.setForeground(TextColor);
				range.mergeCharFormat(colorFormat);
			}
		}
		// 末尾留一个空段落，作为下一条消息或流式尾块的落脚点
		cursor.insertBlock();
	}
}
