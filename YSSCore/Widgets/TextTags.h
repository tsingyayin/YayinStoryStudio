#pragma once
#include <QtWidgets/qframe.h>
// Forward declarations
namespace YSSCore::Widgets {
	class TextTagsPrivate;
}
// Main
namespace YSSCore::Widgets {
	class TextTags :public QFrame {
		Q_OBJECT;
	public:
		TextTags(QWidget* parent = nullptr);
		~TextTags() noexcept;
		QStringList getAllTags();
		void addTag(const QString& tag);
		qint32 indexOf(const QString& tag);
		void removeAt(qint32 index);
		virtual void resizeEvent(QResizeEvent* event);
	private:
		TextTagsPrivate* d;
	};
}
