#ifndef Visindigo_Widgets_LiquidGlassEffect_h
#define Visindigo_Widgets_LiquidGlassEffect_h
#include <QtWidgets/qgraphicseffect.h>
#include "VICompileMacro.h"
namespace Visindigo::Widgets {
	class LiquidGlassEffectPrivate;
	class VisindigoAPI LiquidGlassEffect : public QGraphicsEffect {
		Q_OBJECT;
	public:
		enum class EffectType {
			Distort = 0x01,
			Blur = 0x02,
			All = Distort | Blur
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
		static QImage distortImage(const QImage& image, int radius);
	public:
		explicit LiquidGlassEffect(QWidget* parent = nullptr);
		~LiquidGlassEffect() override;
		void setEffectTypes(EffectTypes effects);
		void setBackgroundImage(const QImage& image);
		void setLiquidDistortRadius(int radius);
		void setBorderRadius(int radius);
		void setPositionPolicy(PositionPolicy policy);
		void setCustomGeometry(const QRect& geometry);
		void setBackgroundPolicy(BackgroundPolicy policy);
		void setBlurRadius(int radius);
		virtual void draw(QPainter* painter) override;
	private:
		LiquidGlassEffectPrivate* d;
	};
	Q_DECLARE_OPERATORS_FOR_FLAGS(LiquidGlassEffect::EffectTypes)
}

#endif // Visindigo_Widgets_LiquidGlassEffect_h