#ifndef Visindigo_Widgets_LiquidGlassEffect_h
#define Visindigo_Widgets_LiquidGlassEffect_h
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>
#include <QtCore/qsize.h>
#include <QtGui/qcolor.h>
#include <QtGui/qimage.h>
#include <QtWidgets/qgraphicseffect.h>
#include "VICompileMacro.h"
namespace Visindigo::Widgets {
	class LiquidGlassEffectPrivate;
	class VisindigoAPI LiquidGlassEffect : public QGraphicsEffect {
		Q_OBJECT;
	public:
		enum class EffectType {
			NormalDistort = 0x01,
			Blur = 0x02,
			RimLight = 0x04,
			GlobalRim = 0x08,
			DispersionDistort = 0x10,
				All = NormalDistort | Blur | RimLight | GlobalRim | DispersionDistort
		};
		Q_DECLARE_FLAGS(EffectTypes, EffectType);
		enum class BackgroundPolicy {
			Render,
			CustomImage
		};
		enum class PositionPolicy {
			ParentLocalGeometry,
			ParentGlobalGeometry,
			CustomGeometry
		};
	public:
		static QImage blurImage(const QImage& image, int radius);
		static QImage distortImage(const QImage& image, int radius, qreal dispersion = 0.0);
		static QImage colorMaskImage(const QImage& image, const QColor& color, qreal percent);
		static QImage drawDebugBackground(qint32 width, qint32 height);
		static QImage rimLightImage(const QImage& image, int borderRadius, const QPoint& mousePosition,
			int thickness, int range, const QColor& color, qreal intensity = 1.0, QRect* changedRect = nullptr);
		static QImage globalRimImage(const QImage& image, int borderRadius, const QPointF& lightDirection,
			int lightThickness, const QColor& lightColor, int shadowThickness, const QColor& shadowColor);
		static void applyForegroundAlpha(QWidget* widget, qreal alpha = 0.7);
	public:
		explicit LiquidGlassEffect(QWidget* parent = nullptr);
		~LiquidGlassEffect() override;
		void setEffectTypes(EffectTypes effects);
		void setBackgroundImage(const QImage& image);
		void setLiquidDistortRadius(int radius);
		void setLiquidDistortDispersion(qreal dispersion);
		void setBorderRadius(int radius);
		void setColorMask(const QColor& color, qreal percent);
		void setPositionPolicy(PositionPolicy policy);
		void setCustomGeometry(const QRect& geometry);
		void setBackgroundPolicy(BackgroundPolicy policy);
		void setBlurRadius(int radius);
		void setRimLightColor(const QColor& color);
		void setRimLightThickness(int thickness);
		void setRimLightRange(int range);
		void setRimLightActivationDistance(int distance);
		void setGlobalRimLightDirection(const QPointF& direction);
		void setGlobalRimLightColor(const QColor& color);
		void setGlobalRimLightThickness(int thickness);
		void setGlobalRimShadowColor(const QColor& color);
		void setGlobalRimShadowThickness(int thickness);
		virtual void draw(QPainter* painter) override;
		virtual bool eventFilter(QObject* watched, QEvent* event) override;
	private:
		LiquidGlassEffectPrivate* d;
	};
	Q_DECLARE_OPERATORS_FOR_FLAGS(LiquidGlassEffect::EffectTypes)
}

#endif // Visindigo_Widgets_LiquidGlassEffect_h