#ifndef YayinStoryStudio_Editor_MainEditor_ConsumingOperationWidget_h
#define YayinStoryStudio_Editor_MainEditor_ConsumingOperationWidget_h
#include <Widgets/BorderFrame.h>
namespace YSS::Editor {
	class ConsumingOperationWidgetPrivate;
	class ConsumingOperationWidget :public Visindigo::Widgets::BorderFrame {
		Q_OBJECT;
	public:
		ConsumingOperationWidget(QWidget* parent = nullptr) {};
		virtual ~ConsumingOperationWidget() {};
	public:
		void setText(const QString& text);
		void setProgress(qint32 progress);
		void setIndeterminate(bool indeterminate);
		void setCancelButtonVisible(bool visible);
	};
}
#endif // YayinStoryStudio_Editor_MainEditor_ConsumingOperationWidget_h
