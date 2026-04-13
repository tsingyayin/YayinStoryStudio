#include "Editor/HoverInfoProvider.h"
#include "Editor/private/HoverInfoProvider_p.h"
#include "Editor/private/TextEdit_p.h"
#include "Editor/TextEdit.h"

namespace YSSCore::Editor {
	class HoverInfoProviderPrivate {
		friend class HoverInfoProvider;
	protected:
		QTextDocument* Document;
		TextEdit* Parent;
	};

	/*!
		\class YSSCore::Editor::HoverInfoProvider
		\brief HoverInfoProvider使用户可以向TextEdit提供鼠标悬停提示信息
		\since YSS 0.13.0
		\inmodule YSSCore
		\ingroup LangService

		HoverInfoProvider使用户可以向TextEdit提供鼠标悬停提示信息。当用户在TextEdit中悬停鼠标、
		或正在编辑中时，就会调用该类的onMouseHover函数，并传入当前鼠标所在位置的行列号以及该位置的文本内容。
		用户可以在该函数中根据需要设置悬停提示信息的内容和格式。

		在实现onMouseHover函数时，你可以调用setContent、setPlainText、setMarkdown或setHtml函数来设置悬停提示信息的内容和格式。

		此外，还可以通过triggerFromHover函数来判断当前的悬停提示信息是否是由鼠标悬停触发的，还是由编辑操作触发的。
		这对于一些需要区分这两种情况的功能实现可能会有帮助。

		如果你决定不进行任何提示，只需要在onMouseHover函数中不调用任何设置内容的函数即可。TextEdit会根据是否设置了内容来决定是否显示悬停提示信息。
	*/

	/*!
		\enum YSSCore::Editor::HoverInfoProvider::Format
		\since YSS 0.13.0
		\value PlainText 纯文本格式
		\value Markdown Markdown格式
		\value Html HTML格式
	*/

	/*!
		\since YSS 0.13.0
		构造函数
	*/
	HoverInfoProvider::HoverInfoProvider(TextEdit* textEdit)
		: QObject(textEdit->getDocument()), d(new YSSCore::__Private__::HoverInfoProviderPrivate){
		d->Document = textEdit->getDocument();
	}

	/*!
		\since YSS 0.13.0
		triggerFromHover函数用于判断当前的悬停提示信息是否是由鼠标悬停触发的，还是由编辑操作触发的。
	*/
	bool HoverInfoProvider::triggerFromHover() {
		return d->TriggerFromHover;
	}

	/*!
		\since YSS 0.13.0
		setContent函数用于设置悬停提示信息的内容和格式。你可以在onMouseHover函数中调用该函数来设置悬停提示信息。
	*/
	void HoverInfoProvider::setContent(const QString& content, Format format) {
		d->CurrentFormat = format;
		d->Content = content;
		d->HoverSetSth = true;
	}

	/*!
		\since YSS 0.13.0
		setPlainText函数用于设置悬停提示信息的内容为纯文本格式。你可以在onMouseHover函数中调用该函数来设置悬停提示信息。
	*/
	void HoverInfoProvider::setPlainText(const QString& text) {
		setContent(text, PlainText);
	}

	/*!
		\since YSS 0.13.0
		setMarkdown函数用于设置悬停提示信息的内容为Markdown格式。你可以在onMouseHover函数中调用该函数来设置悬停提示信息。
	*/
	void HoverInfoProvider::setMarkdown(const QString& md) {
		setContent(md, Markdown);
	}

	/*!
		\since YSS 0.13.0
		setHtml函数用于设置悬停提示信息的内容为HTML格式。你可以在onMouseHover函数中调用该函数来设置悬停提示信息。
	*/
	void HoverInfoProvider::setHtml(const QString& html) {
		setContent(html, Html);
	}

}