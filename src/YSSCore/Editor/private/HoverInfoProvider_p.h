#ifndef YSSCore_Editor_HoverInfoProvider_p_h
#define YSSCore_Editor_HoverInfoProvider_p_h
#include <QtWidgets/qframe.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qtextbrowser.h>
#include <QtWidgets/qboxlayout.h>
#include "Editor/HoverInfoProvider.h"
class QTextDocument;
namespace YSSCore::__Private__ {
	class TextEditPrivate;
}
namespace YSSCore::Editor {
	class TextEdit;
}
namespace YSSCore::__Private__ {
	class HoverInfoProviderPrivate {
		friend class YSSCore::Editor::HoverInfoProvider;
		friend class YSSCore::__Private__::TextEditPrivate;
	protected:
		QTextDocument* Document;
		YSSCore::Editor::HoverInfoProvider::Format CurrentFormat = YSSCore::Editor::HoverInfoProvider::PlainText;
		QString Content;
		bool HoverSetSth = false;
		bool TriggerFromHover = false;
	};

	class HoverInfoWidget :public QFrame {
		Q_OBJECT;
		friend class YSSCore::__Private__::TextEditPrivate;
		friend class YSSCore::Editor::TextEdit;
	protected:
		HoverInfoWidget(QWidget* parent = nullptr);
		QTextBrowser* ContentArea;
		QVBoxLayout* Layout;
		void setPlainText(const QString& text);
		void setMarkdown(const QString& md);
		void setHtml(const QString& html);
	};
}
#endif
