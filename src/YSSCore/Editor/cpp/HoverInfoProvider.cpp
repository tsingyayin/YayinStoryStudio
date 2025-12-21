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

	HoverInfoProvider::HoverInfoProvider(QTextDocument* doc)
		: QObject(doc), d(new YSSCore::__Private__::HoverInfoProviderPrivate)
	{
		d->Document = doc;
	}

	bool HoverInfoProvider::triggerFromHover() {
		return d->TriggerFromHover;
	}

	void HoverInfoProvider::setContent(const QString& content, Format format) {
		d->CurrentFormat = format;
		d->Content = content;
		d->HoverSetSth = true;
	}
	void HoverInfoProvider::setPlainText(const QString& text) {
		setContent(text, PlainText);
	}

	void HoverInfoProvider::setMarkdown(const QString& md) {
		setContent(md, Markdown);
	}

	void HoverInfoProvider::setHtml(const QString& html) {
		setContent(html, Html);
	}

}