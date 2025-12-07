#pragma once
#include "../Macro.h"
#include <QtCore/qobject.h>
#include <QtCore/qstringlist.h>
class QTextDocument;
namespace YSSCore::Editor {
	class HoverInfoProviderPrivate;
	class YSSCoreAPI HoverInfoProvider :public QObject {
		Q_OBJECT;
		VImpl(HoverInfoProvider);
	public:
		HoverInfoProvider(QTextDocument* doc) {};
		virtual QStringList onMouseHover(const QString& text, int position) = 0;
	};
}