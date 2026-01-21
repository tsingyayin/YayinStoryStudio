#ifndef Visindigo_Widgets_TitleWidget_h
#define Visindigo_Widgets_TitleWidget_h
#include "../Macro.h"
#include <QtWidgets/qframe.h>
namespace Visindigo::__Private__ {
	class TitleWidgetPrivate;
}

namespace Visindigo::Widgets {
	class TitleWidgetPrivate;
	class VisindigoAPI TitleWidget :public QFrame {
		Q_OBJECT;
		friend class TitleWidgetPrivate;
	public:
		enum TitleButtonFlag {
			Minimumize = 0x01,
			Maximize = 0x02,
			Close = 0x04,
			NotMinimumize = Minimumize | Close,
			All = Minimumize | Maximize | Close
		};
		enum ButtonStyleFlag {
			SignButton = 0x01,
			RoundButton = 0x02,
			ButtonLeft = 0x04,
			ButtonRight = 0x08,
			Win32Style = SignButton | ButtonRight,
			MacOSStyle = RoundButton | ButtonLeft
		};
		Q_DECLARE_FLAGS(TitleButtonFeature, TitleButtonFlag);
		Q_DECLARE_FLAGS(ButtonStyleFeature, ButtonStyleFlag);
	public:
		TitleWidget(QWidget* parent = nullptr, QWidget* topWidget = nullptr, bool autoSetupTopWidget = true, bool enableDragMove = true);
		virtual ~TitleWidget();
		void setTopWidget(QWidget* topWidget);
		QWidget* getTopWidget() const;
		void setTitleText(const QString& text);
		QString getTitleText() const;
		void setPixmapPath(const QString& filePath);
		QString getPixmapPath() const;
		void setTitleButtonFeature(TitleButtonFeature feature);
		TitleButtonFeature getTitleButtonFeature() const;
		void setButtonStyle(ButtonStyleFeature style);
		ButtonStyleFeature getButtonStyle() const;
		void setupTopWidget();
		void setDragMoveEnable(bool enable);
		bool isDragMoveEnable() const;
		void setInsertWidget(QWidget* widget);
		QWidget* getInsertWidget() const;
		void setTitleAlignment(Qt::Alignment alignment);
		Qt::Alignment getTitleAlignment() const;
	private:
		__Private__::TitleWidgetPrivate* d;
	};
	Q_DECLARE_OPERATORS_FOR_FLAGS(TitleWidget::TitleButtonFeature);
	Q_DECLARE_OPERATORS_FOR_FLAGS(TitleWidget::ButtonStyleFeature);
}
#endif // Visindigo_Widgets_TitleWidget_h