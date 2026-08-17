#ifndef ASERStudio_YSS_ASERFileServer_p_h
#define ASERStudio_YSS_ASERFileServer_p_h
#include <QtWidgets/qlabel.h>
#include <QtGui/qpalette.h>
#include <QtCore/qmargins.h>
namespace ASERStudio::YSS {
	class TagLabel :public QLabel {
		Q_OBJECT;
	public:
		explicit TagLabel(QWidget* parent = nullptr);
		void setFillRole(QPalette::ColorRole role);
	protected:
		void paintEvent(QPaintEvent* event) override;
		QSize sizeHint() const override;
		QSize minimumSizeHint() const override;
	private:
		QColor visibleFillColor() const;
		QSize paddedSize(const QSize& sz) const;
		QPalette::ColorRole m_fillRole = QPalette::Base;
	};
}
#endif // ASERStudio_YSS_ASERFileServer_p_h
