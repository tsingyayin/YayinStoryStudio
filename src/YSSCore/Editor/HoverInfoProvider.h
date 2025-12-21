#pragma once
#include "../Macro.h"
#include <QtCore/qobject.h>
#include <QtCore/qstringlist.h>
#include <QtGui/qtextdocument.h>
namespace YSSCore::__Private__ {
	class HoverInfoProviderPrivate;
	class TextEditPrivate;
}
namespace YSSCore::Editor {
	class TextEdit;
	class YSSCoreAPI HoverInfoProvider :public QObject {
		Q_OBJECT;
		friend class YSSCore::__Private__::HoverInfoProviderPrivate;
		friend class YSSCore::__Private__::TextEditPrivate;
		friend class YSSCore::Editor::TextEdit;
	public:
		enum Format {
			PlainText,
			Markdown,
			Html
		};
	public:
		HoverInfoProvider(QTextDocument* doc);
		bool triggerFromHover();
		virtual void onMouseHover(const QString& text, int position) = 0;
		void setContent(const QString& content, Format format);
		void setPlainText(const QString& text);
		void setMarkdown(const QString& md);
		void setHtml(const QString& html);
	protected:
		YSSCore::__Private__::HoverInfoProviderPrivate* d;
	};
}