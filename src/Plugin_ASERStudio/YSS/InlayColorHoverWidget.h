#ifndef Plugin_ASERStudio_YSS_InlayColorHoverWidget_h
#define Plugin_ASERStudio_YSS_InlayColorHoverWidget_h
#include <QtGui/qcolor.h>
#include <Editor/InlayHintHoverWidget.h>
class QLabel;
class QPushButton;
namespace ASERStudio::YSS {
	class InlayColorHoverWidget :public YSSCore::Editor::InlayHintHoverWidget {
		Q_OBJECT;
	public:
		InlayColorHoverWidget(QWidget* parent = nullptr);
		virtual ~InlayColorHoverWidget();
	public:
		virtual void onHoverRequest() override;
	private:
		void openColorDialog();
		QString buildNewValue(const QColor& color) const;
		void updatePreview();
	private:
		QLabel* ValueLabel = nullptr;
		QPushButton* PickButton = nullptr;
		QLabel* HintLabel = nullptr;
		qint32 Start = 0;
		qint32 Length = 0;
		QString Kind;
		double Scale = 1.0;
		bool HasAlpha = false;
		int Components = 3;
		QColor CurrentColor;
	};
}
#endif // Plugin_ASERStudio_YSS_InlayColorHoverWidget_h
