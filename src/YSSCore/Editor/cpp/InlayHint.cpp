#include <QtGui/qfontinfo.h>
#include <QtGui/qfontmetrics.h>
#include "Editor/InlayHint.h"
#include "Editor/private/InlayHint_p.h"

/*!
	\class YSSCore::Editor::InlayHint
	\brief 一条内联提示，用于在文本流里插入不属于文档内容的辅助文字。
	\since YSS 0.18.0
	\inmodule YSSCore

	内联提示指的是显示在字符之间，并且会把后面的文字推开的辅助文字。
	它可以用来显示参数名、类型、提示信息等，帮助用户理解当前文本的含义。
	但它不是文档内容的一部分，不会被保存、不会进入撤销栈、也不会改变光标位置，
	更不会在复制的时候跟着进入剪贴板。它的显示与否由编辑器控制，用户可以选择关闭内联提示。

	提示由 YSSCore::Editor::SyntaxHighlighter::createInlayHint 在着色过程中创建，随该行的文本块一起移动，
	并在下一次着色该行时自动被替换。

	\note 提示位置必须大于0，因为把后面的文字推开依靠的是"给前一个字符加字距"，
	行首没有可用的锚点字符，位于行首的提示会被忽略。
*/

/*!
	\since YSS 0.18.0
	构造函数。位置为0、文字为空、颜色无效、内边距为8。
*/
YSSCore::Editor::InlayHint::InlayHint() {
}

/*!
	\since YSS 0.18.0
	\a positionInBlock 提示插入的位置（块内字符偏移，提示显示在该位置之前，必须大于0）。
	\a text 提示文字。
	\a color 提示文字颜色。无效颜色时使用编辑器默认的弱化文字颜色。
	\a paddings 提示文字左右两侧的内边距（像素）。
*/
YSSCore::Editor::InlayHint::InlayHint(qint32 positionInBlock, const QString& text, const QColor& color, qint32 paddings) {
	this->PositionInBlock = positionInBlock;
	this->Text = text;
	this->Color = color;
	this->Paddings = paddings;
}

/*!
	\since YSS 0.18.0
	析构函数。
*/
YSSCore::Editor::InlayHint::~InlayHint() {
}

/*!
	\since YSS 0.18.0
	设置提示插入的位置（块内字符偏移）。
*/
void YSSCore::Editor::InlayHint::setPositionInBlock(qint32 positionInBlock) {
	this->PositionInBlock = positionInBlock;
}

/*!
	\since YSS 0.18.0
	设置提示文字。
*/
void YSSCore::Editor::InlayHint::setText(const QString& text) {
	this->Text = text;
}

/*!
	\since YSS 0.18.0
	设置提示文字颜色。设置为无效颜色时使用编辑器默认的弱化文字颜色。
*/
void YSSCore::Editor::InlayHint::setColor(const QColor& color) {
	this->Color = color;
}

/*!
	\since YSS 0.18.0
	设置提示文字左右两侧的内边距（像素）。
*/
void YSSCore::Editor::InlayHint::setPaddings(qint32 paddings) {
	this->Paddings = qMax(0, paddings);
}

/*!
	\since YSS 0.18.0
	\return 提示文字。
*/
QString YSSCore::Editor::InlayHint::getText() const {
	return this->Text;
}

/*!
	\since YSS 0.18.0
	\return 提示文字颜色。未设置时返回无效颜色。
*/
QColor YSSCore::Editor::InlayHint::getColor() const {
	return this->Color;
}

/*!
	\since YSS 0.18.0
	\return 提示插入的位置（块内字符偏移）。
*/
qint32 YSSCore::Editor::InlayHint::getPositionInBlock() const {
	return this->PositionInBlock;
}

/*!
	\since YSS 0.18.0
	\return 提示文字左右两侧的内边距（像素）。
*/
qint32 YSSCore::Editor::InlayHint::getPaddings() const {
	return this->Paddings;
}

QFont YSSCore::__Private__::inlayHintFont(const QFont& baseFont, qreal fontScale) {
	QFont font = baseFont;
	// 正文的字体可能没有指定字号（由系统或文档默认值决定），需要先解析出实际生效的字号。
	const QFontInfo info(baseFont);
	const qreal pointSize = info.pointSizeF();
	if (pointSize > 0) {
		font.setPointSizeF(pointSize * fontScale);
	}
	else if (info.pixelSize() > 0) {
		font.setPixelSize(qMax(1, static_cast<int>(info.pixelSize() * fontScale)));
	}
	return font;
}

qreal YSSCore::__Private__::inlayHintGapWidth(const YSSCore::Editor::InlayHint& hint, const QFont& hintFont) {
	const QFontMetricsF metrics(hintFont);
	return metrics.horizontalAdvance(hint.getText()) + hint.getPaddings() * 2;
}
