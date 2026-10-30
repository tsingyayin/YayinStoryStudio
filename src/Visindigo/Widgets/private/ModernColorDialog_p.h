#ifndef Visindigo_Widgets_private_ModernColorDialog_p_h
#define Visindigo_Widgets_private_ModernColorDialog_p_h
#include <QtCore/qpair.h>
#include <QtCore/qpoint.h>
#include <QtCore/qsize.h>
#include <QtCore/qvector.h>
#include <QtGui/qcolor.h>
#include <QtGui/qimage.h>
#include <QtWidgets/qwidget.h>
// Forward declarations
class QGridLayout;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QSpinBox;
class QVBoxLayout;
namespace Visindigo::Widgets {
	class ModernColorPicker;
	class ModernColorDialog;
}
// Main
namespace Visindigo::__Private__ {
	class ModernColorPicker_Preview;

	class ModernColorPicker_Slider :public QWidget {
		Q_OBJECT;
	signals:
		void valueChanged(qint32 value);
	public:
		ModernColorPicker_Slider(QWidget* parent = nullptr);
		virtual ~ModernColorPicker_Slider();
	public:
		void setRange(qint32 minimum, qint32 maximum);
		void setValue(qint32 value);
		qint32 getValue() const;
		void setGradientStops(const QVector<QPair<qreal, QColor>>& stops);
		void setCheckerboard(bool enable);
	public:
		virtual QSize sizeHint() const override;
	protected:
		virtual void paintEvent(QPaintEvent* event) override;
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseMoveEvent(QMouseEvent* event) override;
		virtual void mouseReleaseEvent(QMouseEvent* event) override;
		virtual void wheelEvent(QWheelEvent* event) override;
	private:
		void updateValueFromPosition(qreal x);
		QRectF getTrackRect() const;
	protected:
		QVector<QPair<qreal, QColor>> GradientStops;
		qint32 Minimum = 0;
		qint32 Maximum = 255;
		qint32 Value = 0;
		bool Checkerboard = false;
	};

	class ModernColorPicker_Wheel :public QWidget {
		Q_OBJECT;
	signals:
		void colorChanged(const QColor& color);
	private:
		enum class DragMode {
			None = 0,
			Hue,
			Triangle
		};
	public:
		ModernColorPicker_Wheel(QWidget* parent = nullptr);
		virtual ~ModernColorPicker_Wheel();
	public:
		void setColor(const QColor& color);
		QColor getColor() const;
	public:
		virtual QSize sizeHint() const override;
	protected:
		virtual void paintEvent(QPaintEvent* event) override;
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseMoveEvent(QMouseEvent* event) override;
		virtual void mouseReleaseEvent(QMouseEvent* event) override;
	private:
		void ensureImages();
		void rebuildRingImage();
		void rebuildTriangleImage();
		void applyTrianglePoint(const QPointF& pos);
		void notifyColorChanged();
		QPointF getCenter() const;
		qreal getOuterRadius() const;
		qreal getInnerRadius() const;
		QPointF getPointAtAngle(qreal angleDegrees, qreal radius) const;
		qreal getAngleOfPoint(const QPointF& pos) const;
		qreal getRadiusOfPoint(const QPointF& pos) const;
		void getTriangleVertices(QPointF& hueVertex, QPointF& whiteVertex, QPointF& blackVertex) const;
		void getTriangleWeights(const QPointF& hueVertex, const QPointF& whiteVertex,
			const QPointF& blackVertex, const QPointF& pos, qreal& hueWeight,
			qreal& whiteWeight, qreal& blackWeight) const;
		QPointF getMarkerPosition() const;
	protected:
		QImage RingImage;
		QImage TriangleImage;
		QSize RingImagePixelSize;
		QSize TriangleImagePixelSize;
		qreal CachedHue = -1.0;
		qreal Hue = 0.0;
		qreal Saturation = 0.0;
		qreal Value = 1.0;
		qreal MarkerRadius = 6.0;
		DragMode CurrentDrag = DragMode::None;
	};

	class ModernColorPickerPrivate {
		friend class Visindigo::Widgets::ModernColorPicker;
	protected:
		struct ChannelRow {
			QSpinBox* Spin = nullptr;
			ModernColorPicker_Slider* Slider = nullptr;
		};
	protected:
		Visindigo::Widgets::ModernColorPicker* q = nullptr;
		ModernColorPicker_Wheel* Wheel = nullptr;
		ModernColorPicker_Slider* AlphaSlider = nullptr;
		QSpinBox* AlphaSpin = nullptr;
		QLineEdit* HexEdit = nullptr;
		ChannelRow RedRow;
		ChannelRow GreenRow;
		ChannelRow BlueRow;
		ChannelRow HslHueRow;
		ChannelRow HslSaturationRow;
		ChannelRow HslLightnessRow;
		ChannelRow HsvHueRow;
		ChannelRow HsvSaturationRow;
		ChannelRow HsvValueRow;
		ModernColorPicker_Preview* Preview = nullptr;
		QColor CurrentColor = QColor(255, 255, 255);
		qreal HueHint = 0.0;
		bool AlphaEditable = true;
		bool HexEditing = false;
		bool Updating = false;
	protected:
		void buildUi();
		void applyControlStyle();
		void updatePreview();
		QSpinBox* createSpinBox(const QString& name, qint32 maximum);
		QLabel* createChannelLabel(const QString& text);
		void addSectionHeader(QGridLayout* grid, const QString& text, qint32& row);
		void addChannelRow(QGridLayout* grid, ChannelRow* row, const QString& label,
			const QString& name, qint32 maximum, qint32& gridRow);
		void connectChannelRow(ChannelRow* row, void (ModernColorPickerPrivate::* handler)(qint32));
		void applyColor(const QColor& color, bool syncWheel);
		void refreshControls(bool syncWheel);
		void updateGradients();
		void updateHexText();
		QString getHexText() const;
		qreal getHueForDisplay() const;
		void notifyColorChanged();
		void onWheelColorChanged(const QColor& color);
		void onAlphaChanged(qint32 value);
		void onRedChanged(qint32 value);
		void onGreenChanged(qint32 value);
		void onBlueChanged(qint32 value);
		void onHslHueChanged(qint32 value);
		void onHslSaturationChanged(qint32 value);
		void onHslLightnessChanged(qint32 value);
		void onHsvHueChanged(qint32 value);
		void onHsvSaturationChanged(qint32 value);
		void onHsvValueChanged(qint32 value);
		void onHexEdited(const QString& text);
		void onHexEditingFinished();
	};

	class ModernColorDialogPrivate {
		friend class Visindigo::Widgets::ModernColorDialog;
	protected:
		Visindigo::Widgets::ModernColorPicker* Picker = nullptr;
	};
}
#endif // Visindigo_Widgets_private_ModernColorDialog_p_h
