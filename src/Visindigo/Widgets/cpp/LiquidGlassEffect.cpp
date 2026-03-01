#include "Widgets/LiquidGlassEffect.h"
#include <QtWidgets/qwidget.h>
#include <QtWidgets/qlabel.h>
#include <QtGui/qimage.h>
#include <QtGui/qpainter.h>
#include <QtGui/qpainterpath.h>
namespace Visindigo::Widgets {
	class LiquidGlassEffectPrivate {
		friend class LiquidGlassEffect;
	protected:
		QImage backgroundImage;
		int liquidDistortRadius = 50;
		int borderRadius = 20;
		QWidget* parentWidget = nullptr;
		QRect customGeometry;
		int blurRadius = 13;
		LiquidGlassEffect::EffectTypes effectTypes = LiquidGlassEffect::EffectType::All;
		LiquidGlassEffect::PositionPolicy positionPolicy = LiquidGlassEffect::PositionPolicy::ParentLocalGeometry;
		LiquidGlassEffect::BackgroundPolicy backgroundPolicy = LiquidGlassEffect::BackgroundPolicy::Render;
	};

	/*!
		\class Visindigo::Widgets::LiquidGlassEffect
		\inheaderfile Widgets/LiquidGlassEffect.h
		\inmodule Visindigo
		\brief 一个提供液态玻璃效果的图形效果类。
		\since Visindigo 0.13.0

		LiquidGlassEffect 是一个 QGraphicsEffect 的子类，提供了一种模仿IOS26的液态玻璃的图形效果。
		这个实现认为液态玻璃效果主要由两部分组成：模仿液体流动的扭曲效果和模仿玻璃模糊的高斯模糊。用户可以通过设置不同的属性来调整这些效果的强度和外观。

		理论上，要完全模仿IOS26的液态玻璃，还至少需要边缘光，但这个实现暂时没有提供边缘光的功能。未来可能会考虑添加这个功能。

		此外，这个实现也仅适用于矩形、圆角矩形和圆形的QWidget，异形的QWidget可能无法正确显示效果，暂不考虑支持异形QWidget。
		\note 注意，要正确使用这个类，它的父对象必须是将被应用效果的QWidget，否则效果可能无法正确显示。

		此类的核心功能：扭曲 和 模糊 也可通过静态函数直接调用，允许用户在不使用QGraphicsEffect的情况下对任意图像应用这些效果。

		\section1 被模糊源的取得
		由于QWidget::render()方法的限制，很多时候其渲染的内容可能丢失下层窗口的内容，因此这个实现提供了两种获取被模糊源的方法：
		\list
		\li 1. LiquidGlassEffect::BackgroundPolicy::Render：通过调用QWidget::render()方法获取被模糊源的图像。这种方法在大多数情况下效果不错，但在某些特定情况下可能会丢失下层窗口的内容。
		尤其在下层窗口本身就含有透明要素时，这种方法可能会导致模糊效果不正确。
		\li 2. LiquidGlassEffect::BackgroundPolicy::CustomImage：允许用户直接设置一个自定义的图像作为被模糊源。这种方法可以确保模糊效果的正确性，但需要用户自己提供一个合适的图像。
		一般来说，如果用户的窗体含有透明要素并存在多级层叠，这是唯一可能可以正确显示模糊效果的方法。
		\endlist
		
		\section1 CustomImage 模式下的PositionPolicy
		当BackgroundPolicy设置为CustomImage时，PositionPolicy的设置会影响模糊效果的位置，即模糊效果会根据PositionPolicy的设置在CustomImage上进行定位和裁剪：
		\list
		\li 1. PositionPolicy::ParentLocalGeometry：模糊效果的位置和大小与父对象的本地几何形状相匹配。这意味着CustomImage可以被视为是父对象所在区域的背景。
		\li 2. PositionPolicy::ParentGlobalGeometry：模糊效果的位置和大小与父对象在屏幕上的全局几何形状相匹配。这意味着CustomImage可以被视为是整个窗口的背景。
		\li 3. PositionPolicy::CustomGeometry：模糊效果的位置和大小由用户通过setCustomGeometry()方法设置的自定义几何形状决定。这提供了最大的灵活性，允许用户完全控制模糊效果在CustomImage上的位置和大小。
		\endlist
		指的指出的是，CustomGeometry应该至少和控件保持一致的长宽比例，否则可能会导致模糊效果的变形。

		\section1 效果类型
		默认情况下，LiquidGlassEffect会同时应用扭曲和模糊两种效果，但用户也可以通过setEffectTypes()方法选择只应用其中一种效果。这提供了更多的定制选项，允许用户根据自己的需求调整效果的外观。
		当扭曲和模糊同时应用时，扭曲效果会先应用于被模糊源的图像，然后模糊效果会应用于已经被扭曲的图像。这种顺序可以确保模糊效果能够正确地模仿液态玻璃的外观。

		\section1 性能考虑
		由于模糊和扭曲效果都可能比较耗费性能，尤其是在较大的半径设置下，因此建议用户在使用这个类时注意性能问题。特别是在需要频繁更新效果的情况下，过大的半径设置可能会导致界面卡顿。

		从实现代码的角度来看，模糊的性能永远且一定差于扭曲，因此用户在设置模糊半径时应该谨慎考虑是否应该使用过大的模糊半径，以避免性能问题。

		此类的模糊采用的是X-Y分解和整形计算的高斯模糊，虽然性能相较于直接卷积的高斯模糊有了很大的提升，但仍然是相当重的操作。
	*/

	/*!
		\enum Visindigo::Widgets::LiquidGlassEffect::EffectType
		\since Visindigo 0.13.0
		\brief 定义了液态玻璃效果的类型。
		\value Distort 扭曲效果，模仿液体流动的效果。
		\value Blur 模糊效果，模仿玻璃模糊的效果。
		\value All 同时应用扭曲和模糊效果，扭曲永远优先于模糊。
	*/

	/*!
		\enum Visindigo::Widgets::LiquidGlassEffect::BackgroundPolicy
		\since Visindigo 0.13.0
		\brief 定义了获取被模糊源的方法。
		\value Render 指示此类直接从QWidget::render()获取被模糊源
		\value CustomImage 指示此类使用用户提供的自定义图像作为被模糊源
	*/

	/*!
		\enum Visindigo::Widgets::LiquidGlassEffect::PositionPolicy
		\since Visindigo 0.13.0
		\brief 定义了模糊效果在CustomImage上的定位和裁剪方式。
		\value ParentLocalGeometry 模糊效果的位置和大小与父对象的本地几何形状相匹配
		\value ParentGlobalGeometry 模糊效果的位置和大小与父对象在屏幕上的全局几何形状相匹配
		\value CustomGeometry 模糊效果的位置和大小由用户通过setCustomGeometry()方法设置的自定义几何形状决定
	*/

	/*!
		\since Visindigo 0.13.0
		构造函数。
	*/
	LiquidGlassEffect::LiquidGlassEffect(QWidget* parent) : QGraphicsEffect(parent), d(new LiquidGlassEffectPrivate()) {
		d->parentWidget = parent;
	}

	/*!
		\since Visindigo 0.13.0
		析构函数。
	*/
	LiquidGlassEffect::~LiquidGlassEffect() {
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		设置效果类型
		\a types 要应用的效果类型，可以是EffectType的任意组合。
	*/
	void LiquidGlassEffect::setEffectTypes(EffectTypes effects) {
		d->effectTypes = effects;
		update();
	}

	/*!
		\since Visindigo 0.13.0
		设置被模糊源的图像，仅在BackgroundPolicy为CustomImage时有效。
		\a image 用于模糊的图像。
	*/
	void LiquidGlassEffect::setBackgroundImage(const QImage& image) {
		d->backgroundImage = image;
		update();
	}

	/*!
		\since Visindigo 0.13.0
		设置液体扭曲效果的半径，单位为像素。
		\a radius 扭曲效果的半径，值越大扭曲效果越明显。
		当扭曲半径超过控件最短边的一半时，内部会自动按最短边的一半进行扭曲，且不改变用户设置值。
	*/
	void LiquidGlassEffect::setLiquidDistortRadius(int radius) {
		if (radius <= 0) radius = 1;
		d->liquidDistortRadius = radius;
		update();
	}

	/*!
		\since Visindigo 0.13.0
		设置边框半径，单位为像素。
		\a radius 边框的半径，值越大边框越圆润。
		如果被应用效果的控件具有圆角，那么就应该同步设置此值以避免输出时溢出应有的视觉边界。
	*/
	void LiquidGlassEffect::setBorderRadius(int radius) {
		d->borderRadius = radius;
		update();
	}

	/*!
		\since Visindigo 0.13.0
		设置模糊效果的位置策略，仅在BackgroundPolicy为CustomImage时有效。
		\a policy 模糊效果的位置策略，决定了模糊效果在CustomImage上的定位和裁剪方式。
	*/
	void LiquidGlassEffect::setPositionPolicy(PositionPolicy policy) {
		d->positionPolicy = policy;
		update();
	}

	/*!
		\since Visindigo 0.13.0
		设置自定义几何形状，仅在PositionPolicy为CustomGeometry时有效。
		\a geometry 自定义的几何形状，决定了模糊效果在CustomImage上的位置和大小。
	*/
	void LiquidGlassEffect::setCustomGeometry(const QRect& geometry) {
		d->customGeometry = geometry;
		if (d->positionPolicy == PositionPolicy::CustomGeometry) {
			update();
		}
	}

	/*!
		\since Visindigo 0.13.0
		设置获取被模糊源的方法。
		\a policy 获取被模糊源的方法，决定了模糊效果的来源。
	*/
	void LiquidGlassEffect::setBackgroundPolicy(BackgroundPolicy policy) {
		d->backgroundPolicy = policy;
		update();
	}

	/*!
		\since Visindigo 0.13.0
		设置模糊效果的半径，单位为像素。
		\a radius 模糊效果的半径，值越大模糊效果越明显。
		出于性能考虑，建议不要设置过大的模糊半径，尤其是在较大的控件上。
	*/
	void LiquidGlassEffect::setBlurRadius(int radius) {
		d->blurRadius = radius;
		update();
	}

	using BGRAPtr = quint32*;
	using ByteColorPtr = quint8*;

	/*!
		\since Visindigo 0.13.0
		对图像进行高斯模糊处理。
		\a image 要模糊的图像。
		\a radius 模糊效果的半径，值越大模糊效果越明显。

		此函数不提供sigma的手动设置，其sigma采用
		\badcode
		sigma = 0.3 * ((radius - 1) * 0.5 - 1) + 0.8
		\endcode
		来计算，这个公式是OpenCV使用的经验公式，可以在不同半径下得到比较自然的模糊效果。
	*/
	QImage LiquidGlassEffect::blurImage(const QImage & image, int radius) {
		if (radius <= 0) return image;
		if (radius % 2 == 0) radius++;
		double* kernel1D = new double[radius];
		double sum = 0;
		double sigma = 0.3 * ((radius - 1) * 0.5 - 1) + 0.8;
		double sigma_2 = sigma * sigma;
		for (int i = 0; i < radius; i++) {
			double x = i - (radius - 1) / 2.0;
			kernel1D[i] = std::exp(-x * x / (2 * sigma_2)) / (std::sqrt(2 * M_PI) * sigma);
			sum += kernel1D[i];
		}
		for (int i = 0; i < radius; i++) {
			kernel1D[i] /= sum;
		}
		int* intKernel = new int[radius];
		for (int i = 0; i < radius; i++) {
			intKernel[i] = kernel1D[i] * 65536;
		}
		QImage blurred(image.size(), QImage::Format_ARGB32);
		BGRAPtr rawPtr = (BGRAPtr)image.bits();
		BGRAPtr blurredPtr = (BGRAPtr)blurred.bits();
		int halfRadius = radius / 2;
		for (int y = 0; y < image.height(); y++) { // horizontal blur
			for (int x = 0; x < image.width(); x++) {
				int r = 0, g = 0, b = 0, a = 0;
				for (int k = -halfRadius; k <= halfRadius; k++) {
					int sampleX = std::clamp(x + k, 0, image.width() - 1);
					quint32 pixel = rawPtr[y * image.width() + sampleX];
					r += ((pixel >> 16) & 0xFF) * intKernel[k + halfRadius];
					g += ((pixel >> 8) & 0xFF) * intKernel[k + halfRadius];
					b += (pixel & 0xFF) * intKernel[k + halfRadius];
					a += ((pixel >> 24) & 0xFF) * intKernel[k + halfRadius];
				}
				blurredPtr[y * image.width() + x] = (std::clamp(a / 65536, 0, 255) << 24) |
					(std::clamp(r / 65536, 0, 255) << 16) |
					(std::clamp(g / 65536, 0, 255) << 8) |
					std::clamp(b / 65536, 0, 255);
			}
		}
		for (int x = 0; x < image.width(); x++) { // vertical blur
			for (int y = 0; y < image.height(); y++) {
				int r = 0, g = 0, b = 0, a = 0;
				for (int k = -halfRadius; k <= halfRadius; k++) {
					int sampleY = std::clamp(y + k, 0, image.height() - 1);
					quint32 pixel = blurredPtr[sampleY * image.width() + x];
					r += ((pixel >> 16) & 0xFF) * intKernel[k + halfRadius];
					g += ((pixel >> 8) & 0xFF) * intKernel[k + halfRadius];
					b += (pixel & 0xFF) * intKernel[k + halfRadius];
					a += ((pixel >> 24) & 0xFF) * intKernel[k + halfRadius];
				}
				blurredPtr[y * image.width() + x] = (std::clamp(a / 65536, 0, 255) << 24) |
					(std::clamp(r / 65536, 0, 255) << 16) |
					(std::clamp(g / 65536, 0, 255) << 8) |
					std::clamp(b / 65536, 0, 255);
			}
		}
		delete[] kernel1D;
		delete[] intKernel;
		return blurred;
	}

	/*!
		\since Visindigo 0.13.0
		对图像进行液体扭曲处理。
		\a coverdArea 要扭曲的图像。
		\a liquidDistortRadius 扭曲效果的半径，值越大扭曲效果越明显。
		此函数通过对图像的边缘区域进行基于正弦函数的坐标偏移来实现液体流动的效果。
		扭曲半径决定了边缘区域的宽度，半径越大，扭曲效果越明显。

		为了保持性能，扭曲效果仅应用于图像的边缘区域，而图像的中心部分保持不变。
		这种方法可以在不显著增加计算量的情况下提供明显的液体流动效果。

		当扭曲半径超过图像最短边的一半时，内部会自动按最短边的一半进行扭曲，且不改变用户设置值。

		此外，这个扭曲不为不同颜色分量提供不同强度的扭曲以形成色散，因为总的来说色散对于液态玻璃效果的贡献非常有限，
		但性能损耗却相对较大，因此这个实现没有提供色散效果。

		与此同时，由于液态玻璃通常需要模糊效果，本质上算是一种平滑处理，因此考虑到综合性能，
		这个扭曲实现在采样的时候没有单独进行插值计算，因此单独使用时可能会出现锯齿状的边缘。
	*/
	QImage LiquidGlassEffect::distortImage(const QImage& coverdArea, int liquidDistortRadius) {
		QImage result(coverdArea.size(), QImage::Format_ARGB32);
		QPainter painter(&result);
		painter.drawImage(0, 0, coverdArea);
		painter.end();
		liquidDistortRadius = std::min(liquidDistortRadius, std::min(coverdArea.width(), coverdArea.height()) / 2);

		static int LDRd2 = (float)liquidDistortRadius / 2;
		BGRAPtr resColor = (BGRAPtr)result.bits();
		BGRAPtr covColor = (BGRAPtr)coverdArea.bits();
		// left - top
		for (int y = 0; y < liquidDistortRadius; y++) {
			int rY = LDRd2 + (1.0 + std::sin((float)(y + 1) / liquidDistortRadius * M_PI_2 - M_PI_2)) * LDRd2;
			for (int x = 0; x < liquidDistortRadius; x++) {
				int rX = LDRd2 + (1.0 + std::sin((float)(x + 1) / liquidDistortRadius * M_PI_2 - M_PI_2)) * LDRd2;
				resColor[y * result.width() + x] = covColor[rY * result.width() + rX];
			}
		}
		// right - top
		for (int y = 0; y < liquidDistortRadius; y++) {
			int rY = LDRd2 + (1.0 + std::sin((float)(y + 1) / liquidDistortRadius * M_PI_2 - M_PI_2)) * LDRd2;
			int iXL = coverdArea.width() - liquidDistortRadius;
			int iX = 0;
			for (int x = iXL; x < coverdArea.width(); x++) {
				int rX = iXL + std::sin((float)(iX + 1) / liquidDistortRadius * M_PI_2) * LDRd2;
				resColor[y * result.width() + x] = covColor[rY * result.width() + rX];
				iX++;
			}
		}
		// left - bottom
		int iY = 0;
		int iYT = coverdArea.height() - liquidDistortRadius;
		for (int y = iYT; y < coverdArea.height(); y++) {
			int rY = iYT + std::sin((float)(iY + 1) / liquidDistortRadius * M_PI_2) * LDRd2;
			for (int x = 0; x < liquidDistortRadius; x++) {
				int rX = LDRd2 + (1.0 + std::sin((float)(x + 1) / liquidDistortRadius * M_PI_2 - M_PI_2)) * LDRd2;
				resColor[y * result.width() + x] = covColor[rY * result.width() + rX];
			}
			iY++;
		}
		// right - bottom
		iY = 0;
		iYT = coverdArea.height() - liquidDistortRadius;

		int iXL = coverdArea.width() - liquidDistortRadius;
		for (int y = iYT; y < coverdArea.height(); y++) {
			int rY = iYT + std::sin((float)(iY + 1) / liquidDistortRadius * M_PI_2) * LDRd2;
			int iX = 0;
			for (int x = iXL; x < coverdArea.width(); x++) {
				int rX = iXL + std::sin((float)(iX + 1) / liquidDistortRadius * M_PI_2) * LDRd2;
				resColor[y * result.width() + x] = covColor[rY * result.width() + rX];
				iX++;
			}
			iY++;
		}
		// top
		for (int y = 0; y < liquidDistortRadius; y++) {
			int rY = LDRd2 + (1.0 + std::sin((float)(y + 1) / liquidDistortRadius * M_PI_2 - M_PI_2)) * LDRd2;
			for (int x = liquidDistortRadius; x < coverdArea.width() - liquidDistortRadius; x++) {
				resColor[y * result.width() + x] = covColor[rY * result.width() + x];
			}
		}
		// bottom
		iY = 0;
		iYT = coverdArea.height() - liquidDistortRadius;
		for (int y = iYT; y < coverdArea.height(); y++) {
			int rY = iYT + std::sin((float)(iY + 1) / liquidDistortRadius * M_PI_2) * LDRd2;
			for (int x = liquidDistortRadius; x < coverdArea.width() - liquidDistortRadius; x++) {
				resColor[y * result.width() + x] = covColor[rY * result.width() + x];
			}
			iY++;
		}
		// left
		for (int y = liquidDistortRadius; y < coverdArea.height() - liquidDistortRadius; y++) {
			for (int x = 0; x < liquidDistortRadius; x++) {
				int rX = LDRd2 + (1.0 + std::sin((float)(x + 1) / liquidDistortRadius * M_PI_2 - M_PI_2)) * LDRd2;
				resColor[y * result.width() + x] = covColor[y * result.width() + rX];
			}
		}
		// right
		iXL = coverdArea.width() - liquidDistortRadius;
		for (int y = liquidDistortRadius; y < coverdArea.height() - liquidDistortRadius; y++) {
			int iX = 0;
			for (int x = iXL; x < coverdArea.width(); x++) {
				int rX = iXL + std::sin((float)(iX + 1) / liquidDistortRadius * M_PI_2) * LDRd2;
				resColor[y * result.width() + x] = covColor[y * result.width() + rX];
				iX++;
			}
		}
		return result;
	}

	/*!
		\since Visindigo 0.13.0
		重写了QGraphicsEffect的draw()方法，在其中实现了液态玻璃效果的绘制逻辑。
	*/
	void LiquidGlassEffect::draw(QPainter* painter) {
		QPixmap sourcePixmap = this->sourcePixmap(Qt::LogicalCoordinates);
		if (sourcePixmap.isNull()) {
			return;
		}
		QImage coverdArea(d->parentWidget->size(), QImage::Format_ARGB32);
		qDebug() << QImage::Format_ARGB32;
		if (d->backgroundPolicy == BackgroundPolicy::Render) {
			QWidget* parent_parent = d->parentWidget->parentWidget();
			if (!parent_parent) {
				parent_parent->render(&coverdArea, QPoint(), QRegion(d->parentWidget->geometry()));
			}
		}
		else {
			switch (d->positionPolicy) {
			case PositionPolicy::ParentLocalGeometry:
				coverdArea = d->backgroundImage.copy(d->parentWidget->geometry());
				break;
			case PositionPolicy::ParentGlobalGeometry: {
				QPoint globalPos = d->parentWidget->mapToGlobal(QPoint(0, 0));
				QRect globalRect(globalPos, d->parentWidget->size());
				coverdArea = d->backgroundImage.copy(globalRect);
				break;
			}
			case PositionPolicy::CustomGeometry:
				coverdArea = d->backgroundImage.copy(d->customGeometry);
				break;
			}	
		}
		if (coverdArea.isNull()) {
			painter->drawPixmap(0, 0, sourcePixmap);
			return;
		}
		if (d->effectTypes.testAnyFlag(LiquidGlassEffect::EffectType::Distort)){
			coverdArea = distortImage(coverdArea, d->liquidDistortRadius);
		}
		if (d->effectTypes.testAnyFlag(LiquidGlassEffect::EffectType::Blur)) {
			coverdArea = blurImage(coverdArea, d->blurRadius);
		}
		QPixmap finalPixmap = sourcePixmap;
		QPainter finalPainter(&finalPixmap);
		QPainterPath roundPath;
		roundPath.addRoundedRect(0, 0, sourcePixmap.width() / sourcePixmap.devicePixelRatio(), sourcePixmap.height() / sourcePixmap.devicePixelRatio(), d->borderRadius, d->borderRadius);
		finalPainter.setClipPath(roundPath);
		finalPainter.drawImage(0, 0, coverdArea);
		finalPainter.drawPixmap(0, 0, sourcePixmap);
		finalPainter.end();
		painter->drawPixmap(0, 0, finalPixmap);
	}
}