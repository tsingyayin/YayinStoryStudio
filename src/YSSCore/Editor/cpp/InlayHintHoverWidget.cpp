#include "Editor/InlayHintHoverWidget.h"
namespace YSSCore::Editor {
	class InlayHintHoverWidgetPrivate {
		friend class InlayHintHoverWidget;
	protected:
		YSSCore::General::YSSURI TargetFile;
		qint32 BlockNumber = -1;
		qint32 Column = 0;
		QString RawText;
		Visindigo::Utility::JsonConfig Para;
	};

	/*!
		\class YSSCore::Editor::InlayHintHoverWidget
		\brief 图片内联提示的悬停浮窗基类。
		\since YSS 0.18.0
		\inmodule YSSCore

		InlayHintHoverWidget由 YSSCore::Editor::InlayHintProvider::createHoverWidget 创建，
		所有权随后交给对应的 TextEdit。TextEdit负责浮窗的显示、隐藏、位置与大小，
		Provider只需要在 onHoverRequest() 中根据当前上下文调整页面内容。

		在调用 onHoverRequest() 之前，TextEdit会先通过 setTargetFile、setBlockNumber、
		setColumn、setRawText、setPara 设置好当前上下文，实现可以直接读取这些值。

		Provider在用户要求修改文本时发出 requestModify 信号。TextEdit收到后会从
		\a startColumn 开始、替换 \a rawLength 个字符为 \a newContent；列号以文本块为基准，
		从0开始。
	*/
	InlayHintHoverWidget::InlayHintHoverWidget(QWidget* parent) :QFrame(parent) {
		d = new InlayHintHoverWidgetPrivate();
		this->setObjectName(QStringLiteral("InlayHintHoverWidget"));
		this->setFrameShape(QFrame::StyledPanel);
		this->setAttribute(Qt::WA_StyledBackground, true);
		auto font = this->font();
		font.setPointSizeF(font.pointSizeF() * 0.8);
		this->setFont(font);
	}

	InlayHintHoverWidget::~InlayHintHoverWidget() {
		delete d;
	}

	/*!
		\since YSS 0.18.0
		\a targetFile 当前文本编辑器的文件URI。
	*/
	void InlayHintHoverWidget::setTargetFile(const YSSCore::General::YSSURI& targetFile) {
		d->TargetFile = targetFile;
	}

	/*!
		\since YSS 0.18.0
		\return 当前文本编辑器的文件URI。
	*/
	YSSCore::General::YSSURI InlayHintHoverWidget::getTargetFile() const {
		return d->TargetFile;
	}

	/*!
		\since YSS 0.18.0
		\a blockNumber 当前内联提示所在的文本块号，从0开始。
	*/
	void InlayHintHoverWidget::setBlockNumber(qint32 blockNumber) {
		d->BlockNumber = blockNumber;
	}

	/*!
		\since YSS 0.18.0
		\return 当前内联提示所在的文本块号。
	*/
	qint32 InlayHintHoverWidget::getBlockNumber() const {
		return d->BlockNumber;
	}

	/*!
		\since YSS 0.18.0
		\a column 当前文本块内的字符位置，从0开始。
	*/
	void InlayHintHoverWidget::setColumn(qint32 column) {
		d->Column = column;
	}

	/*!
		\since YSS 0.18.0
		\return 当前文本块内的字符位置。
	*/
	qint32 InlayHintHoverWidget::getColumn() const {
		return d->Column;
	}

	/*!
		\since YSS 0.18.0
		\a rawText 当前文本块的完整原文。
	*/
	void InlayHintHoverWidget::setRawText(const QString& rawText) {
		d->RawText = rawText;
	}

	/*!
		\since YSS 0.18.0
		\return 当前文本块的完整原文。
	*/
	QString InlayHintHoverWidget::getRawText() const {
		return d->RawText;
	}

	/*!
		\since YSS 0.18.0
		\a para Multi 中 \c{?} 后面的JSON参数。
	*/
	void InlayHintHoverWidget::setPara(const Visindigo::Utility::JsonConfig& para) {
		d->Para = para;
	}

	/*!
		\since YSS 0.18.0
		\return Multi 中 \c{?} 后面的JSON参数。
	*/
	Visindigo::Utility::JsonConfig InlayHintHoverWidget::getPara() const {
		return d->Para;
	}
}
