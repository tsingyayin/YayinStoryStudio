#include <QtGui/qfontinfo.h>
#include <QtGui/qfontmetrics.h>
#include "Editor/InlayHint.h"
#include "Editor/InlayHintManager.h"
#include "Editor/InlayHintProvider.h"
#include "Editor/VirtualFilePath.h"
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

	\section1 文字提示与图片提示

	默认情况下提示显示 Text 中的文字。若 Multi 非空、符合
	\c{@providerId!name?param} 格式且对应 Provider 已注册，则提示改由
	InlayHintManager 中保存的 YSSCore::Editor::InlayHintProvider 绘制，Text 不再显示。
	Multi 的格式与 VirtualFilePath 相同：
	\list
	\li \c providerId 对应 InlayHintProvider 的模块ID。
	\li \c name 原样交给 Provider，具体含义由 Provider 约定。
	\li \c param 为 JSON 参数，原样交给 Provider。
	\endlist
	Multi 有效时优先于 Text；Multi 无效或对应 Provider 未注册时回退到 Text。
	图片提示占用的宽度与高度都等于正文行高。

	\note 提示位置必须大于0，因为把后面的文字推开依靠的是"给前一个字符加字距"，
	行首没有可用的锚点字符，位于行首的提示会被忽略。
*/

/*!
	\since YSS 0.18.0
	构造函数。位置为0、文字与 Multi 为空、颜色无效、内边距为8。
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
	设置提示文字。Multi 为空或无效时显示这段文字。
*/
void YSSCore::Editor::InlayHint::setText(const QString& text) {
	this->Text = text;
}

/*!
	\since YSS 0.18.0
	\a multi 图片提示描述，格式与 VirtualFilePath 相同：\c{@providerId!name?param}。

	Multi 有效且对应 Provider 已注册时，提示由该 Provider 绘制，Text 不再显示。
*/
void YSSCore::Editor::InlayHint::setMulti(const QString& multi) {
	this->Multi = multi;
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
	\return 图片提示描述。未设置时返回空字符串。
*/
QString YSSCore::Editor::InlayHint::getMulti() const {
	return this->Multi;
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

qreal YSSCore::__Private__::inlayHintImageSide(const QFont& baseFont) {
	return qMax<qreal>(8.0, QFontMetricsF(baseFont).height());
}

bool YSSCore::__Private__::inlayHintUsesProvider(const YSSCore::Editor::InlayHint& hint, QString* providerID) {
	if (hint.getMulti().isEmpty()) {
		return false;
	}
	const YSSCore::Editor::VirtualFilePath path(hint.getMulti());
	if (not path.isValid()) {
		return false;
	}
	if (YSSCore::Editor::InlayHintManager::getInstance()->getProvider(path.getExt()) == nullptr) {
		return false;
	}
	if (providerID != nullptr) {
		*providerID = path.getExt();
	}
	return true;
}

qreal YSSCore::__Private__::inlayHintGapWidth(const YSSCore::Editor::InlayHint& hint, const QFont& baseFont, const QFont& hintFont) {
	if (inlayHintUsesProvider(hint)) {
		return inlayHintImageSide(baseFont);
	}
	if (hint.getText().isEmpty()) {
		return 0;
	}
	const QFontMetricsF metrics(hintFont);
	return metrics.horizontalAdvance(hint.getText()) + hint.getPaddings() * 2;
}
