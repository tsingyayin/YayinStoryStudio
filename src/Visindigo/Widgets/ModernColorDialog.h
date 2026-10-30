#ifndef Visindigo_Widgets_ModernColorDialog_h
#define Visindigo_Widgets_ModernColorDialog_h
#include <QtGui/qcolor.h>
#include <QtWidgets/qdialog.h>
#include <QtWidgets/qwidget.h>
#include "VICompileMacro.h"
// Forward declarations
class QEvent;
namespace Visindigo::__Private__ {
	class ModernColorPickerPrivate;
	class ModernColorDialogPrivate;
}
// Main
namespace Visindigo::Widgets {
	class VisindigoAPI ModernColorPicker :public QWidget {
		friend class Visindigo::__Private__::ModernColorPickerPrivate;
		Q_OBJECT;
	signals:
		void colorChanged(const QColor& color);
	public:
		ModernColorPicker(QWidget* parent = nullptr);
		virtual ~ModernColorPicker();
	public:
		void setColor(const QColor& color);
		QColor getColor() const;
		void setAlphaEnabled(bool enable);
		bool isAlphaEnabled() const;
	protected:
		virtual void changeEvent(QEvent* event) override;
	private:
		Visindigo::__Private__::ModernColorPickerPrivate* d;
	};

	class VisindigoAPI ModernColorDialog :public QDialog {
		Q_OBJECT;
	signals:
		void colorChanged(const QColor& color);
	public:
		ModernColorDialog(const QColor& initialColor, QWidget* parent = nullptr);
		virtual ~ModernColorDialog();
	public:
		void setColor(const QColor& color);
		QColor getColor() const;
		void setAlphaEnabled(bool enable);
		bool isAlphaEnabled() const;
		ModernColorPicker* getColorPicker() const;
	public:
		static QColor pickColor(const QColor& initialColor, QWidget* parent = nullptr,
			bool alphaEnabled = false);
	private:
		Visindigo::__Private__::ModernColorDialogPrivate* d;
	};
}
#endif // Visindigo_Widgets_ModernColorDialog_h
