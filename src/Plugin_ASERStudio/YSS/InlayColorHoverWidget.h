#ifndef Plugin_ASERStudio_YSS_InlayColorHoverWidget_h
#define Plugin_ASERStudio_YSS_InlayColorHoverWidget_h
#include <QtGui/qcolor.h>
#include <Editor/InlayHintHoverWidget.h>
// Forward declarations
class QLabel;
class QPushButton;
namespace Visindigo::Widgets {
	class ModernColorPicker;
}
// Main
namespace ASERStudio::YSS {
	class InlayColorHoverWidget :public YSSCore::Editor::InlayHintHoverWidget {
		Q_OBJECT;
	private:
		QLabel* ValueLabel = nullptr;
		Visindigo::Widgets::ModernColorPicker* Picker = nullptr;
		QLabel* HintLabel = nullptr;
		QPushButton* ApplyButton = nullptr;
		qint32 Start = 0;
		qint32 Length = 0;
		QString Kind;
		double Scale = 1.0;
		bool HasAlpha = false;
		int Components = 3;
		QString OriginalValue;
	public:
		InlayColorHoverWidget(QWidget* parent = nullptr);
		virtual ~InlayColorHoverWidget();
	public:
		virtual void onHoverRequest() override;
	protected:
		virtual void keyPressEvent(QKeyEvent* event) override;
		virtual void paintEvent(QPaintEvent* event) override;
	private:
		void buildUi();
		void updateApplyButtonStyle();
		QString buildNewValue(const QColor& color) const;
		void onApplyClicked();
	};
}
#endif // Plugin_ASERStudio_YSS_InlayColorHoverWidget_h

