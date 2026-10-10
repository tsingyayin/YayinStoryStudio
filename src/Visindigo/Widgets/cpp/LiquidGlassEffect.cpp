#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <QtCore/qhashfunctions.h>
#include <QtCore/qpointer.h>
#include <QtCore/qscopeguard.h>
#include <QtCore/qvector.h>
#include <QtGui/qcursor.h>
#include <QtGui/qevent.h>
#include <QtGui/qimage.h>
#include <QtGui/qpainter.h>
#include <QtGui/qpainterpath.h>
#include <QtGui/qwindow.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qwidget.h>
#include "General/Log.h"
#include "Widgets/LiquidGlassEffect.h"
namespace Visindigo::Widgets {
	class LiquidGlassEffectPrivate {
		friend class LiquidGlassEffect;
	protected:
		/*
			采样嵌套状态，只被renderParentBackground()使用。

			采样时要把“自己”从父控件的渲染里摘掉（做法与理由见renderParentBackground()的注释），
			但渲染父控件会顺带把压在同一父控件上的另一个玻璃控件也画一遍，于是触发对方的效果、
			对方也来这里采样：A采样 → 渲父控件 → 触发B → B采样。此时A还处于“隐藏”状态，
			B采到的画面里就没有A（表现为上层玻璃把下层内容吃掉，且随鼠标悬停/重绘路径时隐时现）。

			所以这里记一个采样栈：本轮只隐藏栈顶（也就是自己），栈里外层的控件临时恢复可见，
			退出时还原。栈非空期间，事件过滤器还会把发给被监听控件的Paint当成“采样造成的”忽略掉，
			否则嵌套的两次采样会互相作废对方的帧缓存，每次都重采一遍。

			嵌套的是两个不同的实例，谁也看不到对方的成员，所以这份状态只能是静态的；
			它描述的是“当前线程正在干什么”，因此按线程各存一份。
			hidden里的指针只在采样期间有效（渲染是同步的），不持有这些控件。
		*/
		inline static thread_local int samplingDepth = 0;
		inline static thread_local QVector<QWidget*> samplingHidden;
		// 采样作用域的RAII守卫：构造时接管采样栈，析构时无条件还原，
		// 这样“隐藏自己 → 渲染 → 恢复”三件事不会因为中途出岔子而配不上对
		struct SamplingScope {
			explicit SamplingScope(QWidget* self) : widget(self) {
				// 外层正在采样的控件在本轮必须可见，否则采到的画面里会缺掉它
				for (int i = 0; i < samplingHidden.size(); i++) {
					QWidget* outer = samplingHidden[i];
					if (outer && outer != widget && outer->testAttribute(Qt::WA_WState_Hidden)) {
						outer->setAttribute(Qt::WA_WState_Hidden, false);
						temporarilyShown.append(i);
					}
				}
				wasHidden = widget->testAttribute(Qt::WA_WState_Hidden);
				if (!wasHidden) {
					widget->setAttribute(Qt::WA_WState_Hidden, true);
				}
				samplingHidden.append(widget);
				samplingDepth++;
			}
			~SamplingScope() {
				samplingDepth--;
				samplingHidden.removeLast();
				if (!wasHidden) {
					widget->setAttribute(Qt::WA_WState_Hidden, false);
				}
				// 还原外层采样控件的隐藏状态
				for (int i = temporarilyShown.size() - 1; i >= 0; i--) {
					QWidget* outer = samplingHidden[temporarilyShown[i]];
					if (outer) {
						outer->setAttribute(Qt::WA_WState_Hidden, true);
					}
				}
			}
			SamplingScope(const SamplingScope&) = delete;
			SamplingScope& operator=(const SamplingScope&) = delete;
			QWidget* widget = nullptr;
			bool wasHidden = false;
			QVector<int> temporarilyShown;
		};
	protected:
		QImage backgroundImage;
		int liquidDistortRadius = 20;
		qreal liquidDistortDispersion = 0.2;
		int borderRadius = 20;
		QWidget* parentWidget = nullptr;
		QRect customGeometry;
		int blurRadius = 13;
		qreal percent = 0;
		QColor colorMask;
		LiquidGlassEffect::EffectTypes effectTypes = LiquidGlassEffect::EffectType::All;
		LiquidGlassEffect::PositionPolicy positionPolicy = LiquidGlassEffect::PositionPolicy::ParentLocalGeometry;
		LiquidGlassEffect::BackgroundPolicy backgroundPolicy = LiquidGlassEffect::BackgroundPolicy::Render;
	protected:
		QColor rimLightColor = QColor(255, 255, 255, 130);
		int rimLightThickness = 1;
		int rimLightRange = 80;
		int rimLightActivationDistance = 40;
		QPoint mousePosition;
		qreal rimLightIntensity = 0.0;
		QPointer<QWidget> trackedParent;
		QPointer<QWidget> trackedWidget;
		QPointer<QWindow> trackedWindow;
	protected:
		QPointF globalRimLightDirection = QPointF(-0.5, -0.8660254); // 光从左上角来，与竖直方向成30°
		QColor globalRimLightColor = QColor(255, 255, 255, 90);
		int globalRimLightThickness = 1;
		QColor globalRimShadowColor = QColor(0, 0, 0, 60);
		int globalRimShadowThickness = 2;
	protected:
		// 鼠标一动边缘光就要重绘，而除了边缘光以外的东西全都不变，边缘光又只影响边界附近的一小块。
		// 所以把“扭曲+全局光之后、边缘光之前”的图像和它的模糊结果缓存下来，
		// 鼠标移动时只重算被边缘光影响到的那一条带，不必把整张图重新模糊一遍。
		QImage frameBaseCache;
		QImage frameBlurredCache;
		QImage frameScratch;
		QSize frameCacheSize = QSize(-1, -1);
		quint64 frameCacheKey = 0;
		bool frameCacheValid = false;
		// Render策略下被模糊源是每次从父控件现渲染出来的，随时可能因为别人重绘而变，
		// 所以把采样矩形内所有控件都监听上：有人重绘就把帧缓存作废，没人重绘就一直用缓存。
		QPointer<QWidget> renderSource;
		QVector<QPointer<QWidget>> renderWatched;
		bool frameCacheDirty = false;
		bool drawing = false;
	protected:
		static QVector<QPoint> borderPoints(const QSize& size, int borderRadius, int stride);
		static QImage splatBoundaryGlow(const QSize& size, const QVector<QPoint>& points, const QVector<qreal>& weights, int thickness, const QColor& color, QRect* dirtyRect = nullptr);
		static QImage buildGlobalRimSide(const QSize& size, int borderRadius, const QPointF& lightDirection, int thickness, qreal polarity, const QColor& color, QRect* dirtyRect = nullptr);
		static QImage rimLightGlow(const QSize& size, int borderRadius, const QPoint& mousePosition, int thickness, int range, const QColor& color, QRect* dirtyRect = nullptr);
		static void modulateByLightMap(QImage& target, const QImage& lightMap, const QRect& area, bool brighten, qreal strength = 1.0, bool additive = false);
		static void buildGaussKernel(int radius, QVector<int>& kernel);
		static void blurAccumulate(const quint32* sourceRow, int x, int width, int halfRadius, const int* kernel, int* sums);
		static void blurAccumulateInner(const quint32* sourceRow, int x, int halfRadius, const int* kernel, int* sums);
		static void blurPass1D(const quint32* source, quint32* target, int width, int height, const int* kernel, int halfRadius, const QRect& area, bool horizontal);
		static QImage blurImageFull(const QImage& image, int radius);
		static QImage blurImageBand(const QImage& image, int radius, const QImage& blurredBase, const QRect& changedRect, QImage& scratch);
		qreal rimLightIntensityAt(const QPoint& localPosition) const;
		bool applyMousePosition(const QPoint& globalPosition);
		bool clearMousePosition();
		void syncMouseTracking(LiquidGlassEffect* q);
		void releaseMouseTracking(LiquidGlassEffect* q);
		quint64 frameKey(const QSize& size) const;
		void clearFrameCache();
		void syncRenderWatching(LiquidGlassEffect* q);
		void releaseRenderWatching(LiquidGlassEffect* q);
		void collectRenderWatched(QWidget* widget, QWidget* source, QWidget* exclude, const QRect& rect, QVector<QPointer<QWidget>>& out);
		bool watchesRenderWidget(const QObject* object) const;
		void renderParentBackground(QImage& target);
	};

	/*!
		\class Visindigo::Widgets::LiquidGlassEffect
		\inheaderfile Widgets/LiquidGlassEffect.h
		\inmodule Visindigo
		\brief 一个提供液态玻璃效果的图形效果类.
		\since Visindigo 0.13.0

		LiquidGlassEffect 是一个 QGraphicsEffect 的子类，提供了一种模仿IOS26的液态玻璃的图形效果。
		这个实现认为液态玻璃效果主要由四部分组成：模仿液体流动的扭曲效果、模仿玻璃模糊的高斯模糊、反应鼠标位置的边缘光和模仿真实光源的全局打光。
		用户可以通过设置不同的属性来调整这些效果的强度和外观。

		此外，这个实现也仅适用于矩形、圆角矩形和圆形的QWidget，异形的QWidget可能无法正确显示效果，暂不考虑支持异形QWidget。
		\note 注意，要正确使用这个类，它的父对象必须是将被应用效果的QWidget，否则效果可能无法正确显示。

		值得指出的是，此类的核心功能：扭曲、模糊和边缘光（鼠标/全局） 也可通过静态函数直接调用，允许用户在不使用QGraphicsEffect的情况下对任意图像应用这些效果；
		另外还有drawDebugBackground()，用来生成一张适合调试和演示这些效果的背景图。

		\section1 被模糊源的取得
		由于QWidget::render()方法的限制，很多时候其渲染的内容可能丢失下层窗口的内容，因此这个实现提供了两种获取被模糊源的方法：
		\list
		\li 1. LiquidGlassEffect::BackgroundPolicy::Render：把控件所在的父控件渲染一遍来取被模糊源。这种方法在大多数情况下效果不错，但在某些特定情况下可能会丢失下层窗口的内容。
		尤其在下层窗口本身就含有透明要素时，这种方法可能会导致模糊效果不正确。
		压在玻璃下面的玻璃是能透出来的（模糊、色散都能叠上去），但那只在下层用的是BackgroundPolicy::CustomImage时成立；
		下层若也用BackgroundPolicy::Render，上层看到的只是它未经效果的原始样子——看得见字和图形，看不见它的模糊。
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
		默认情况下，LiquidGlassEffect会同时应用色散扭曲、模糊、边缘光和全局打光四种效果，但用户也可以通过setEffectTypes()方法选择只应用其中一部分效果。这提供了更多的定制选项，允许用户根据自己的需求调整效果的外观。
		扭曲总是先于模糊应用。
		边缘光比较特殊，它不是一直存在的效果，只在鼠标靠近控件时才会出现；而全局打光是一直存在的，它的光来自一个固定的方向，不需要鼠标参与。

		\section1 扭曲与色散
		扭曲模仿的是玻璃边缘对光的折射：只有边缘区域会被扭曲，图像中心保持不变；半径越大，被扭曲的边缘越宽、越明显。扭曲有两种：
		\list
		\li EffectType::NormalDistort：三个颜色通道用同一套映射，就是普通的扭曲效果。
		\li EffectType::DispersionDistort：三个通道的偏折程度略有不同（绿光不动，红光偏折得少一点、蓝光多一点），
		于是边缘会出现彩色镶边，模仿真实玻璃的色散；它比普通扭曲略贵一点，不追求这个效果时用普通扭曲即可
		\endlist
		色散强度通过setLiquidDistortDispersion()调整，取值 0 到 0.5，为 0 时退化成普通扭曲。

		\section1 边缘光
		边缘光模仿的是玻璃边缘被光打亮的样子，它通过EffectType::RimLight开启（包含在EffectType::All中）。
		与其他两种效果不同，边缘光只在鼠标靠近控件的时候才会出现，而且总是出现在离鼠标最近的那一段控件边界上：
		鼠标沿着边界移动时，边缘光会跟着在边界上滑动，看上去就像是控件边缘在反射鼠标位置的光。

		边缘光的颜色、厚度、沿边界延伸的距离以及激活距离分别通过setRimLightColor()、setRimLightThickness()、setRimLightRange()和setRimLightActivationDistance()调整。
		其中激活距离决定了鼠标离控件边界多远时边缘光开始出现，超过这个距离时边缘光完全消失。
		边缘光是叠加提亮的，不像全局打光那样与画面相乘，因此它不会被阴影压暗，背光侧的边缘光同样清晰；
		代价是边缘一带会带出一点偏白的亮部。

		\section1 全局打光
		全局打光模仿的是真实世界里的光源：控件不再是均匀发亮的，而是有一个固定的来光方向，
		迎光的那一面被照亮、与光垂直的两侧完全没有光、背光的那一面则压出一条暗边作为阴影，
		这样控件看上去就有了“厚度”和“被光照到”的体积感。它通过EffectType::GlobalRim开启（包含在EffectType::All中）。

		光的方向通过setGlobalRimLightDirection()设置，取值是一个从控件指向光源的向量（不需要是单位向量，
		以屏幕坐标为准、y轴向下），默认QPointF(-0.5, -0.866)，也就是从左上角来、与竖直方向成30度的光：
		上边界最亮，左边界次之，右边界是浅阴影，下边界是深阴影，四个圆角处自然地从明过渡到暗。
		亮面的颜色和厚度分别通过setGlobalRimLightColor()和setGlobalRimLightThickness()设置，
		阴影的颜色和厚度分别通过setGlobalRimShadowColor()和setGlobalRimShadowThickness()设置。
		两者的默认强度刻意压得比较淡（亮面alpha值90、阴影alpha值60）：全局光只是一个效果元素，
		做满了会盖过玻璃本身的颜色，喧宾夺主。
		阴影是“吸掉一部分光”而不是盖一层黑：纯黑阴影把画面整体压暗，彩色阴影则只吸掉它的补色。

		\section1 性能考虑
		模糊是这类效果里最贵的操作，半径越大越明显，建议不要设置过大的模糊半径，尤其是在较大的控件上。
		扭曲比模糊便宜得多，色散会让扭曲略微变慢，但仍然远低于模糊；全局打光只处理边界附近，开销可以忽略。

		边缘光本身开销很小，但它会跟着鼠标持续重绘；实现内部为此做了缓存，鼠标移动时的开销只与边缘光的范围有关、与控件面积无关，
		代价是每个实例会多占几张同尺寸图像的内存。

		总的来说，BackgroundPolicy::Render适合内容相对静止的场景；像拖动这种内容一直在变的场景，
		用BackgroundPolicy::CustomImage便宜得多。
	*/

	/*!
		\enum Visindigo::Widgets::LiquidGlassEffect::EffectType
		\since Visindigo 0.13.0
		\brief 定义了液态玻璃效果的类型.
		\value NormalDistort 普通扭曲效果，模仿液体流动的效果，红绿蓝三个通道用同一套映射。
		\value Blur 模糊效果，模仿玻璃模糊的效果。
		\value RimLight 边缘光效果，在鼠标靠近控件时于控件边界上绘制跟随鼠标的边缘光。
		\value GlobalRim 全局打光效果，按固定方向的虚拟光源在控件边界上打出亮面和阴影。
		\value DispersionDistort 色散扭曲效果，在扭曲的同时让三个通道的映射距离不同，边缘会出现彩色镶边；与NormalDistort同时开启时以色散为准。
		\value All 同时应用普通扭曲、色散扭曲、模糊、边缘光和全局打光效果，扭曲永远优先于模糊，也是默认值。
	*/

	/*!
		\enum Visindigo::Widgets::LiquidGlassEffect::BackgroundPolicy
		\since Visindigo 0.13.0
		\brief 定义了获取被模糊源的方法.
		\value Render 指示此类直接从QWidget::render()获取被模糊源
		\value CustomImage 指示此类使用用户提供的自定义图像作为被模糊源
	*/

	/*!
		\enum Visindigo::Widgets::LiquidGlassEffect::PositionPolicy
		\since Visindigo 0.13.0
		\brief 定义了模糊效果在CustomImage上的定位和裁剪方式.
		\value ParentLocalGeometry 模糊效果的位置和大小与父对象的本地几何形状相匹配
		\value ParentGlobalGeometry 模糊效果的位置和大小与父对象在屏幕上的全局几何形状相匹配
		\value CustomGeometry 模糊效果的位置和大小由用户通过setCustomGeometry()方法设置的自定义几何形状决定
	*/

	/*!
		\since Visindigo 0.13.0
		\a parent 父窗口，且必须为将被应用效果的QWidget，否则效果可能无法正确显示。
		构造函数。
	*/
	LiquidGlassEffect::LiquidGlassEffect(QWidget* parent) : QGraphicsEffect(parent), d(new LiquidGlassEffectPrivate()) {
		d->parentWidget = parent;
		if (d->effectTypes.testAnyFlag(LiquidGlassEffect::EffectType::RimLight)) {
			d->syncMouseTracking(this);
		}
		d->syncRenderWatching(this);
	}

	/*!
		\since Visindigo 0.13.0
		析构函数。
	*/
	LiquidGlassEffect::~LiquidGlassEffect() {
		d->releaseMouseTracking(this);
		d->releaseRenderWatching(this);
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		设置效果类型
		\a effects 要应用的效果类型，可以是EffectType的任意组合。
		默认是EffectType::All，即普通扭曲、色散扭曲、模糊、边缘光和全局打光全开；
		只想用其中几种时，用|把需要的EffectType拼起来传给这个函数即可。
	*/
	void LiquidGlassEffect::setEffectTypes(EffectTypes effects) {
		d->effectTypes = effects;
		if (d->effectTypes.testAnyFlag(LiquidGlassEffect::EffectType::RimLight)) {
			d->syncMouseTracking(this);
		}
		else {
			d->releaseMouseTracking(this);
		}
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
		\since Visindigo 0.17.0
		设置扭曲的色散强度，仅在EffectType中包含DispersionDistort时有效。
		\a dispersion 色散强度，取值会被夹到 0 到 0.5 之间，默认0.2。
		色散就是让红绿蓝三个通道的映射距离略有不同（绿光不动，红光偏折得少一点、蓝光多一点），
		于是边缘会出现彩色镶边，看起来就像真实玻璃把光拆开了；0表示三个通道完全一致（等同于NormalDistort），越大彩色镶边越明显。
		色散会让扭曲略微变慢。
	*/
	void LiquidGlassEffect::setLiquidDistortDispersion(qreal dispersion) {
		d->liquidDistortDispersion = std::clamp(dispersion, 0.0, 0.5);
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
		\since Visindigo 0.15.0
		设置颜色遮罩。
		\a color 遮罩的颜色，决定了模糊效果的色调。
		\a percent 遮罩的透明度，值越大遮罩越透明，默认1。
		颜色遮罩会在模糊效果之后应用，可以用来调整模糊效果的整体色调和透明度，从而更好地模仿液态玻璃的外观。
	*/
	void LiquidGlassEffect::setColorMask(const QColor& color, qreal percent) {
		d->colorMask = color;
		d->percent = percent;
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
		d->syncRenderWatching(this);
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

	/*!
		\since Visindigo 0.17.0
		设置边缘光的颜色，仅在EffectType中包含RimLight时有效。
		\a color 边缘光的颜色，其alpha值决定了边缘光的最大强度，默认半透明白色（alpha值130）。
	*/
	void LiquidGlassEffect::setRimLightColor(const QColor& color) {
		d->rimLightColor = color;
		update();
	}

	/*!
		\since Visindigo 0.17.0
		设置边缘光的厚度，仅在EffectType中包含RimLight时有效。
		\a thickness 边缘光从控件边界向内扩散的范围，单位为像素，默认1。
	*/
	void LiquidGlassEffect::setRimLightThickness(int thickness) {
		if (thickness <= 0) thickness = 1;
		d->rimLightThickness = thickness;
		update();
	}

	/*!
		\since Visindigo 0.17.0
		设置边缘光沿控件边界延伸的距离，仅在EffectType中包含RimLight时有效。
		\a range 边缘光在边界上能够延伸的距离，单位为像素，默认80。
		边缘光总是出现在离鼠标最近的那一段边界上，并沿着边界向两侧衰减，这个值决定了它能够延伸多远。
	*/
	void LiquidGlassEffect::setRimLightRange(int range) {
		if (range <= 0) range = 1;
		d->rimLightRange = range;
		update();
	}

	/*!
		\since Visindigo 0.17.0
		设置边缘光的激活距离，仅在EffectType中包含RimLight时有效。
		\a distance 鼠标离控件边界多远之内才会出现边缘光，单位为像素，默认40。
		设置为0表示只在鼠标位于控件内部时才出现边缘光。
	*/
	void LiquidGlassEffect::setRimLightActivationDistance(int distance) {
		if (distance < 0) distance = 0;
		d->rimLightActivationDistance = distance;
		update();
	}

	/*!
		\since Visindigo 0.17.0
		设置全局打光的光源方向，仅在EffectType中包含GlobalRim时有效。
		\a direction 从控件指向光源的方向向量，不需要是单位向量，默认QPointF(-0.5, -0.866)，即光从左上角来、与竖直方向成30度。
		以屏幕坐标为准（y轴向下），因此(-0.5, -0.866)表示光从左上角来（偏上），(0, -1)表示光从正上方来，(-1, 0)表示光从左侧来，(1, 1)表示光从右下角来。
	*/
	void LiquidGlassEffect::setGlobalRimLightDirection(const QPointF& direction) {
		d->globalRimLightDirection = direction;
		update();
	}

	/*!
		\since Visindigo 0.17.0
		设置全局打光亮面的颜色，仅在EffectType中包含GlobalRim时有效。
		\a color 迎光面的颜色，其alpha值决定了亮面的最大强度，默认白色、alpha值90（刻意做得比较淡）。
		全局光只是一个效果元素，默认不应该盖过玻璃本身的颜色，需要更强的光时把alpha值调大即可。
	*/
	void LiquidGlassEffect::setGlobalRimLightColor(const QColor& color) {
		d->globalRimLightColor = color;
		update();
	}

	/*!
		\since Visindigo 0.17.0
		设置全局打光亮面的厚度，仅在EffectType中包含GlobalRim时有效。
		\a thickness 亮面从控件边界向内扩散的范围，单位为像素，默认1。
	*/
	void LiquidGlassEffect::setGlobalRimLightThickness(int thickness) {
		if (thickness <= 0) thickness = 1;
		d->globalRimLightThickness = thickness;
		update();
	}

	/*!
		\since Visindigo 0.17.0
		设置全局打光的阴影颜色，仅在EffectType中包含GlobalRim时有效。
		\a color 背光面阴影的颜色，其alpha值决定了阴影的强度，默认半透明黑色（alpha值60）。
	*/
	void LiquidGlassEffect::setGlobalRimShadowColor(const QColor& color) {
		d->globalRimShadowColor = color;
		update();
	}

	/*!
		\since Visindigo 0.17.0
		设置全局打光的阴影厚度，仅在EffectType中包含GlobalRim时有效。
		\a thickness 阴影从控件边界向内扩散的范围，单位为像素，默认2。
		阴影一般比亮面厚一些，这样控件看上去才像是一块有厚度的玻璃。
	*/
	void LiquidGlassEffect::setGlobalRimShadowThickness(int thickness) {
		if (thickness <= 0) thickness = 1;
		d->globalRimShadowThickness = thickness;
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
	QImage LiquidGlassEffect::blurImage(const QImage& image, int radius) {
		return LiquidGlassEffectPrivate::blurImageFull(image, radius);
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

		\a dispersion 色散强度，取值 0 到 0.5，默认0（不做色散）。
		色散通过给红绿蓝三个通道不同的映射距离实现：绿光不动，红光偏折得少一点、蓝光多一点，于是边缘会出现彩色镶边。
		它会让扭曲略微变慢，但仍然远低于模糊；为0时与普通扭曲一样。

		单独使用（不配套模糊）时，边缘可能出现锯齿。
	*/
	QImage LiquidGlassEffect::distortImage(const QImage& coverdArea, int liquidDistortRadius, qreal dispersion) {
		if (coverdArea.isNull()) {
			return QImage();
		}
		const QImage source = coverdArea.convertToFormat(QImage::Format_ARGB32);
		QImage result = source;
		liquidDistortRadius = std::min(liquidDistortRadius, std::min(source.width(), source.height()) / 2);
		if (liquidDistortRadius <= 0) {
			return result;
		}
		const int width = source.width();
		const int height = source.height();
		const int halfRadius = liquidDistortRadius / 2;
		const int rightStart = width - liquidDistortRadius;
		const int bottomStart = height - liquidDistortRadius;
		const qreal clampedDispersion = std::clamp(dispersion, 0.0, 0.5);
		const bool dispersive = clampedDispersion > 0.0;
		const qreal channelScale[3] = { 1.0 - clampedDispersion, 1.0, 1.0 + clampedDispersion };
		QVector<int> nearMaps[3];
		QVector<int> farMaps[3];
		for (int channel = 0; channel < 3; channel++) {
			nearMaps[channel].resize(liquidDistortRadius);
			farMaps[channel].resize(liquidDistortRadius);
			for (int k = 0; k < liquidDistortRadius; k++) {
				const qreal phase = float(k + 1) / liquidDistortRadius * M_PI_2;
				nearMaps[channel][k] = std::clamp(int(halfRadius + (1.0 + std::sin(phase - M_PI_2)) * halfRadius * channelScale[channel]), 0, std::min(width, height) - 1);
				farMaps[channel][k] = std::clamp(int(std::sin(phase) * halfRadius * channelScale[channel]), 0, liquidDistortRadius - 1);
			}
		}
		const QVector<int>& nearMapR = nearMaps[0];
		const QVector<int>& nearMapG = nearMaps[1];
		const QVector<int>& nearMapB = nearMaps[2];
		const QVector<int>& farMapR = farMaps[0];
		const QVector<int>& farMapG = farMaps[1];
		const QVector<int>& farMapB = farMaps[2];
		const quint32* sourceBits = reinterpret_cast<const quint32*>(source.constBits());
		quint32* resultBits = reinterpret_cast<quint32*>(result.bits());
		const qsizetype rowStep = qsizetype(width);
		auto combineChannels = [](quint32 pixelR, quint32 pixelG, quint32 pixelB) {
			return (pixelG & 0xFF000000u) | (pixelR & 0x00FF0000u) | (pixelG & 0x0000FF00u) | (pixelB & 0x000000FFu);
		};
		for (int y = 0; y < liquidDistortRadius; y++) {
			const quint32* nearRowG = sourceBits + qsizetype(nearMapG[y]) * rowStep;
			const quint32* farRowG = sourceBits + qsizetype(bottomStart + farMapG[y]) * rowStep;
			const quint32* nearRowR = dispersive ? sourceBits + qsizetype(nearMapR[y]) * rowStep : nearRowG;
			const quint32* nearRowB = dispersive ? sourceBits + qsizetype(nearMapB[y]) * rowStep : nearRowG;
			const quint32* farRowR = dispersive ? sourceBits + qsizetype(bottomStart + farMapR[y]) * rowStep : farRowG;
			const quint32* farRowB = dispersive ? sourceBits + qsizetype(bottomStart + farMapB[y]) * rowStep : farRowG;
			quint32* topRow = resultBits + qsizetype(y) * rowStep;
			quint32* bottomRow = resultBits + qsizetype(bottomStart + y) * rowStep;
			for (int x = 0; x < liquidDistortRadius; x++) {
				const quint32 pixelG = nearRowG[nearMapG[x]];
				const quint32 pixelGBottom = farRowG[nearMapG[x]];
				topRow[x] = dispersive ? combineChannels(nearRowR[nearMapR[x]], pixelG, nearRowB[nearMapB[x]]) : pixelG;
				bottomRow[x] = dispersive ? combineChannels(farRowR[nearMapR[x]], pixelGBottom, farRowB[nearMapB[x]]) : pixelGBottom;
			}
			if (dispersive) {
				for (int x = liquidDistortRadius; x < rightStart; x++) {
					topRow[x] = combineChannels(nearRowR[x], nearRowG[x], nearRowB[x]);
					bottomRow[x] = combineChannels(farRowR[x], farRowG[x], farRowB[x]);
				}
			}
			else if (rightStart > liquidDistortRadius) {
				const size_t middleBytes = size_t(rightStart - liquidDistortRadius) * sizeof(quint32);
				if (nearRowG != topRow) {
					memcpy(topRow + liquidDistortRadius, nearRowG + liquidDistortRadius, middleBytes);
				}
				if (farRowG != bottomRow) {
					memcpy(bottomRow + liquidDistortRadius, farRowG + liquidDistortRadius, middleBytes);
				}
			}
			for (int x = rightStart; x < width; x++) {
				const int index = x - rightStart;
				const quint32 pixelG = nearRowG[rightStart + farMapG[index]];
				const quint32 pixelGBottom = farRowG[rightStart + farMapG[index]];
				topRow[x] = dispersive ? combineChannels(nearRowR[rightStart + farMapR[index]], pixelG, nearRowB[rightStart + farMapB[index]]) : pixelG;
				bottomRow[x] = dispersive ? combineChannels(farRowR[rightStart + farMapR[index]], pixelGBottom, farRowB[rightStart + farMapB[index]]) : pixelGBottom;
			}
		}
		for (int y = liquidDistortRadius; y < bottomStart; y++) {
			const quint32* sourceRow = sourceBits + qsizetype(y) * rowStep;
			quint32* resultRow = resultBits + qsizetype(y) * rowStep;
			for (int x = 0; x < liquidDistortRadius; x++) {
				const quint32 pixelG = sourceRow[nearMapG[x]];
				resultRow[x] = dispersive ? combineChannels(sourceRow[nearMapR[x]], pixelG, sourceRow[nearMapB[x]]) : pixelG;
			}
			for (int x = rightStart; x < width; x++) {
				const int index = x - rightStart;
				const quint32 pixelG = sourceRow[rightStart + farMapG[index]];
				resultRow[x] = dispersive ? combineChannels(sourceRow[rightStart + farMapR[index]], pixelG, sourceRow[rightStart + farMapB[index]]) : pixelG;
			}
		}
		return result;
	}

	/*!
		\since Visindigo 0.15.0
		对图像进行颜色遮罩处理。
		\a image 要处理的图像。\a color 遮罩的颜色，决定了模糊效果的色调。\a percent 遮罩的透明度，值越大遮罩越透明，默认1。
	*/
	QImage LiquidGlassEffect::colorMaskImage(const QImage& image, const QColor& color, qreal percent) {
		QImage result(image.size(), QImage::Format_ARGB32);
		QPainter painter(&result);
		painter.setOpacity(percent);
		painter.fillRect(result.rect(), color);
		painter.end();
		return result;
	}

	/*!
		\since Visindigo 0.17.0
		生成一张用于调试和演示液态玻璃效果的背景图。
		\a width 图像的宽度，单位为像素。
		\a height 图像的高度，单位为像素。

	*/
	QImage LiquidGlassEffect::drawDebugBackground(qint32 width, qint32 height) {
		if (width <= 0 || height <= 0) {
			return QImage();
		}
		QImage background(QSize(width, height), QImage::Format_ARGB32);
		background.fill(Qt::transparent);
		QPainter painter(&background);
		painter.setRenderHint(QPainter::Antialiasing);
		QLinearGradient gradient(0, 0, width, height);
		gradient.setColorAt(0.0, QColor(0x63, 0x89, 0xF3));
		gradient.setColorAt(0.45, QColor(0x89, 0x87, 0xD8));
		gradient.setColorAt(1.0, QColor(0x10, 0x10, 0x14));
		painter.setPen(Qt::NoPen);
		painter.setBrush(gradient);
		painter.drawRect(background.rect());
		painter.setBrush(QColor(255, 255, 255, 40));
		painter.drawEllipse(QPointF(width * 0.159, height * 0.579), width * 0.205, height * 0.316);
		painter.setBrush(QColor(0, 0, 0, 70));
		painter.drawEllipse(QPointF(width * 0.795, height * 0.053), width * 0.239, height * 0.263);
		painter.setPen(QPen(QColor(255, 255, 255, 75), 3));
		for (int x = -height; x < width; x += 48) {
			painter.drawLine(x, height, x + height, 0);
		}
		// 另一个方向，两组交叉成网格
		for (int x = -height; x < width; x += 48) {
			painter.drawLine(x, 0, x + height, height);
		}
		painter.end();
		return background;
	}

	/*!
		\since Visindigo 0.17.0
		在图像上绘制边缘光，返回绘制后的图像。
		\a image 要绘制的图像，尺寸应当与要绘制边缘光的控件尺寸一致。
		\a borderRadius 控件的圆角半径，边缘光贴着这个圆角矩形的边界生成。
		\a mousePosition 鼠标位置，坐标系与 \a image 相同，允许取值在控件之外。
		\a thickness 边缘光的厚度，即从边界向内扩散的范围，单位为像素。
		\a range 边缘光沿边界延伸的距离，单位为像素。
		\a color 边缘光的颜色，其alpha值决定了边缘光的最大强度。
		\a intensity 整体强度，取值 0 到 1，默认1。鼠标靠近时的淡入、离开时的淡出就靠它。
		\a changedRect 可选输出，返回本次实际改动的区域；不需要就传nullptr。
	*/
	QImage LiquidGlassEffect::rimLightImage(const QImage& image, int borderRadius, const QPoint& mousePosition,
		int thickness, int range, const QColor& color, qreal intensity, QRect* changedRect) {
		if (image.isNull()) {
			return QImage();
		}
		// 先统一成32位格式，后面才可以按像素调制；如果本来就是该格式则不会真的转换
		QImage result = image.convertToFormat(QImage::Format_ARGB32);
		QRect glowRect;
		const QImage glow = LiquidGlassEffectPrivate::rimLightGlow(result.size(), borderRadius, mousePosition, thickness, range, color, &glowRect);
		LiquidGlassEffectPrivate::modulateByLightMap(result, glow, glowRect, true, intensity, true);
		if (changedRect) {
			*changedRect = glowRect;
		}
		return result;
	}

	/*!
		\since Visindigo 0.17.0
		在图像上绘制全局打光，返回绘制后的图像。
		\a image 要绘制的图像，尺寸应当与要打光的控件尺寸一致。
		\a borderRadius 控件的圆角半径，光效沿着这个圆角矩形的边界生成。
		\a lightDirection 从控件指向光源的方向向量，不需要是单位向量。
		\a lightThickness 亮面从边界向内扩散的范围，单位为像素。
		\a lightColor 亮面的颜色，其alpha值决定了亮面的最大强度。
		\a shadowThickness 阴影从边界向内扩散的范围，单位为像素。
		\a shadowColor 阴影的颜色，其alpha值决定了阴影的强度。
	*/
	QImage LiquidGlassEffect::globalRimImage(const QImage& image, int borderRadius, const QPointF& lightDirection,
		int lightThickness, const QColor& lightColor, int shadowThickness, const QColor& shadowColor) {
		if (image.isNull()) {
			return QImage();
		}
		QImage result = image.convertToFormat(QImage::Format_ARGB32);
		const QSize size = result.size();
		// 阴影是高光的伴生产物：同一束光里点积为负的半边就是阴影面，两面用同一个光源方向和同一个圆角半径
		QRect shadowRect;
		const QImage shadow = LiquidGlassEffectPrivate::buildGlobalRimSide(size, borderRadius, lightDirection, shadowThickness, -1.0, shadowColor, &shadowRect);
		LiquidGlassEffectPrivate::modulateByLightMap(result, shadow, shadowRect, false);
		QRect lightRect;
		const QImage light = LiquidGlassEffectPrivate::buildGlobalRimSide(size, borderRadius, lightDirection, lightThickness, 1.0, lightColor, &lightRect);
		LiquidGlassEffectPrivate::modulateByLightMap(result, light, lightRect, true);
		return result;
	}

	/*!
		\since Visindigo 0.17.0
		把控件自身的底色改成半透明，好让挂在这个控件上的液态玻璃透出来。
		\a widget 要处理的控件。
		\a alpha 透明度系数，默认0.7。

		液态玻璃效果会把控件自己画的内容叠在玻璃上面，底色不透明就会把玻璃整个盖住。这里只给调色板里
		的底色（Base/Window）乘上alpha，绘制方式仍然是原来的QStyle：如果改用样式表，Qt会把控件交给
		QStyleSheetStyle绘制，工具条、滚动条、按钮这些控件都会失去原生样式。调色板会传递给子控件，
		所以文本浏览器的视口、标签的底色也会一起变半透明，而文字和图标仍用原色绘制。
	*/
	void LiquidGlassEffect::applyForegroundAlpha(QWidget* widget, qreal alpha) {
		if (widget == nullptr) {
			return;
		}
		QPalette palette = widget->palette();
		for (QPalette::ColorRole role : { QPalette::Base, QPalette::Window }) {
			QColor color = palette.color(role);
			color.setAlphaF(color.alphaF() * alpha);
			palette.setColor(role, color);
		}
		widget->setPalette(palette);
	}

	/*!
		\since Visindigo 0.13.0
		重写了QGraphicsEffect的draw()方法，在其中实现了液态玻璃效果的绘制逻辑。
		\a painter 用于绘制效果的QPainter对象。
	*/
	void LiquidGlassEffect::draw(QPainter* painter) {
		if (d->drawing) {
			return;
		}
		d->drawing = true;
		auto drawingGuard = qScopeGuard([this] {
			d->drawing = false;
		});
		QPixmap sourcePixmap = this->sourcePixmap(Qt::LogicalCoordinates);
		if (sourcePixmap.isNull()) {
			return;
		}
		const QSize areaSize = d->parentWidget->size();
		const quint64 frameKey = d->frameKey(areaSize);
		if (d->frameCacheValid && (d->frameCacheSize != areaSize || d->frameCacheKey != frameKey || d->frameCacheDirty)) {
			d->clearFrameCache();
		}
		QImage base;
		if (d->frameCacheValid) {
			base = d->frameBaseCache;
		}
		else {
			QImage sampled(areaSize, QImage::Format_ARGB32);
			if (d->backgroundPolicy == BackgroundPolicy::Render) {
				d->renderParentBackground(sampled);
			}
			else {
				switch (d->positionPolicy) {
				case PositionPolicy::ParentLocalGeometry:
					sampled = d->backgroundImage.copy(d->parentWidget->geometry());
					break;
				case PositionPolicy::ParentGlobalGeometry: {
					QWidget* topLevelWidget = d->parentWidget->window();
					QPoint globalPos = d->parentWidget->mapTo(topLevelWidget, d->parentWidget->pos());
					QRect globalRect(globalPos, d->parentWidget->size());
					//vgDebug << globalRect;
					sampled = d->backgroundImage.copy(globalRect);
					break;
				}
				case PositionPolicy::CustomGeometry:
					sampled = d->backgroundImage.copy(d->customGeometry);
					break;
				}
			}
			if (sampled.isNull()) {
				painter->drawPixmap(0, 0, sourcePixmap);
				return;
			}
			base = sampled.convertToFormat(QImage::Format_ARGB32);
			const bool dispersionDistort = d->effectTypes.testAnyFlag(LiquidGlassEffect::EffectType::DispersionDistort);
			if (dispersionDistort || d->effectTypes.testAnyFlag(LiquidGlassEffect::EffectType::NormalDistort)) {
				base = distortImage(base, d->liquidDistortRadius, dispersionDistort ? d->liquidDistortDispersion : 0.0);
			}
			if (d->effectTypes.testAnyFlag(LiquidGlassEffect::EffectType::GlobalRim)) {
				base = LiquidGlassEffect::globalRimImage(base, d->borderRadius, d->globalRimLightDirection,
					d->globalRimLightThickness, d->globalRimLightColor, d->globalRimShadowThickness, d->globalRimShadowColor);
			}
			d->frameBaseCache = base;
			d->frameBlurredCache = QImage();
			d->frameCacheSize = areaSize;
			d->frameCacheKey = frameKey;
			d->frameCacheValid = true;
		}
		QRect rimLightRect;
		QImage lit = base;
		if (d->effectTypes.testAnyFlag(LiquidGlassEffect::EffectType::RimLight) && d->rimLightIntensity > 0.0) {
			lit = LiquidGlassEffect::rimLightImage(lit, d->borderRadius, d->mousePosition,
				d->rimLightThickness, d->rimLightRange, d->rimLightColor, d->rimLightIntensity, &rimLightRect);
		}
		QImage coverdArea;
		if (d->effectTypes.testAnyFlag(LiquidGlassEffect::EffectType::Blur)) {
			if (d->frameBlurredCache.isNull()) {
				d->frameBlurredCache = blurImage(base, d->blurRadius); 
			}
			coverdArea = LiquidGlassEffectPrivate::blurImageBand(lit, d->blurRadius, d->frameBlurredCache, rimLightRect, d->frameScratch);
		}
		else {
			coverdArea = lit;
		}
		QPixmap finalPixmap = sourcePixmap;
		QPainter finalPainter(&finalPixmap);
		const qreal devicePixelRatio = sourcePixmap.devicePixelRatio();
		const QSize logicalSize(qRound(sourcePixmap.width() / devicePixelRatio), qRound(sourcePixmap.height() / devicePixelRatio));
		QPainterPath roundPath;
		roundPath.addRoundedRect(0, 0, logicalSize.width(), logicalSize.height(), d->borderRadius, d->borderRadius);
		finalPainter.setClipPath(roundPath);
		finalPainter.drawImage(0, 0, coverdArea);
		finalPainter.drawPixmap(0, 0, sourcePixmap);
		if (d->percent != 0) {
			QPixmap colorMaskPixmap = finalPixmap;
			colorMaskPixmap.fill(d->colorMask);
			finalPainter.setOpacity(d->percent);
			finalPainter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
			finalPainter.drawPixmap(0, 0, colorMaskPixmap);
		}
		finalPainter.end();
		painter->drawPixmap(0, 0, finalPixmap);
	}

	/*!
		\since Visindigo 0.17.0
		\a watched 事件的目标对象。\a event 待处理的事件。

		重写了QObject的eventFilter()方法，用于在启用边缘光时跟踪鼠标位置。
	*/
	bool LiquidGlassEffect::eventFilter(QObject* watched, QEvent* event) {
		if (watched == d->trackedWindow.data()) {
			switch (event->type()) {
			case QEvent::MouseMove: {
				QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
				if (d->applyMousePosition(mouseEvent->globalPosition().toPoint())) {
					update();
				}
				break;
			}
			case QEvent::Enter:
				if (d->applyMousePosition(QCursor::pos())) {
					update();
				}
				break;
			case QEvent::Leave:
			case QEvent::Hide:
			case QEvent::Close:
				if (d->clearMousePosition()) {
					update();
				}
				break;
			default:
				break;
			}
		}
		else if ((watched == d->trackedParent.data() || watched == d->trackedWidget.data())
			&& (event->type() == QEvent::Show || event->type() == QEvent::WinIdChange || event->type() == QEvent::ParentChange)) {
			d->syncMouseTracking(this);
		}
		
		if (d->watchesRenderWidget(watched)) {
			const bool causedBySampling = LiquidGlassEffectPrivate::samplingDepth > 0 && event->type() == QEvent::Paint;
			if (!causedBySampling) {
				switch (event->type()) {
				case QEvent::Paint:
				case QEvent::Show:
				case QEvent::Hide:
				case QEvent::Resize:
				case QEvent::Move:
					d->frameCacheDirty = true;
					break;
				case QEvent::ParentChange:
				case QEvent::ChildAdded:
				case QEvent::ChildRemoved:
				case QEvent::LayoutRequest:
					d->frameCacheDirty = true;
					d->syncRenderWatching(this);
					break;
				default:
					break;
				}
			}
		}
		return QGraphicsEffect::eventFilter(watched, event);
	}

	/*
		用八分圆算法（中点圆算法）生成圆角矩形边界上的点。
		size 为圆角矩形的逻辑尺寸，borderRadius 为圆角半径，stride 为直边上取点的间隔。
		八分圆算法的要点是：圆上 0° 到 45° 之间的点确定以后，剩下的点全部可以由八分对称性映射出来，
		所以只需要解算一个小八分圆，再把它按象限镜像到四个圆角上即可，取整圈边界点的代价和一个八分圆相当。
		圆角部分的点始终逐像素生成（相邻点间隔很小），直边部分才按 stride 抽稀。
	*/
	QVector<QPoint> LiquidGlassEffectPrivate::borderPoints(const QSize& size, int borderRadius, int stride) {
		QVector<QPoint> points;
		if (size.width() <= 0 || size.height() <= 0) {
			return points;
		}
		if (stride < 1) {
			stride = 1;
		}
		const int width = size.width();
		const int height = size.height();
		const int radius = std::clamp(borderRadius, 0, std::min(width, height) / 2);
		if (radius > 0) {
			// 八分圆：只解算 x <= y 的那一份，即 0° 到 45° 的离散点
			QVector<QPoint> octant;
			octant.reserve(radius + 1);
			int x = 0;
			int y = radius;
			int decision = 1 - radius;
			while (x <= y) {
				octant.append(QPoint(x, y));
				x++;
				if (decision < 0) {
					decision += 2 * x + 1;
				}
				else {
					y--;
					decision += 2 * (x - y) + 1;
				}
			}
			// 八分圆及其转置拼成 0° 到 90° 的四分之一圆（接缝处去重），再镜像到四个圆角
			QVector<QPoint> quadrant;
			quadrant.reserve(octant.size() * 2);
			for (int i = 0; i < octant.size(); i++) {
				quadrant.append(QPoint(octant[i].y(), octant[i].x()));
			}
			for (int i = octant.size() - 2; i >= 0; i--) {
				quadrant.append(QPoint(octant[i].x(), octant[i].y()));
			}
			const int rightCenter = width - 1 - radius;
			const int bottomCenter = height - 1 - radius;
			for (int i = 0; i < quadrant.size(); i++) {
				const int offsetX = quadrant[i].x();
				const int offsetY = quadrant[i].y();
				points.append(QPoint(radius - offsetX, radius - offsetY)); // 左上角
				points.append(QPoint(rightCenter + offsetX, radius - offsetY)); // 右上角
				points.append(QPoint(rightCenter + offsetX, bottomCenter + offsetY)); // 右下角
				points.append(QPoint(radius - offsetX, bottomCenter + offsetY)); // 左下角
			}
		}
		// 四条直边，两端与圆角相接，因此直边只需要覆盖两个圆角之间
		const int xEnd = width - 1 - radius;
		for (int x = radius; ; x += stride) {
			x = std::min(x, xEnd);
			points.append(QPoint(x, 0));
			points.append(QPoint(x, height - 1));
			if (x >= xEnd) {
				break;
			}
		}
		const int yEnd = height - 1 - radius;
		for (int y = radius; ; y += stride) {
			y = std::min(y, yEnd);
			points.append(QPoint(0, y));
			points.append(QPoint(width - 1, y));
			if (y >= yEnd) {
				break;
			}
		}
		return points;
	}

	// 鼠标对边缘光的激活强度：鼠标在控件内部时为 1，在控件外部时随着远离边界而衰减到 0
	qreal LiquidGlassEffectPrivate::rimLightIntensityAt(const QPoint& localPosition) const {
		if (!parentWidget) {
			return 0.0;
		}
		const QRect rect(QPoint(0, 0), parentWidget->size());
		if (rect.isEmpty()) {
			return 0.0;
		}
		int deltaX = 0;
		if (localPosition.x() < rect.left()) {
			deltaX = rect.left() - localPosition.x();
		}
		else if (localPosition.x() > rect.right()) {
			deltaX = localPosition.x() - rect.right();
		}
		int deltaY = 0;
		if (localPosition.y() < rect.top()) {
			deltaY = rect.top() - localPosition.y();
		}
		else if (localPosition.y() > rect.bottom()) {
			deltaY = localPosition.y() - rect.bottom();
		}
		const qreal distance = std::hypot(qreal(deltaX), qreal(deltaY));
		if (distance <= 0.0) {
			return 1.0;
		}
		if (rimLightActivationDistance <= 0 || distance >= rimLightActivationDistance) {
			return 0.0;
		}
		const qreal ratio = 1.0 - distance / rimLightActivationDistance;
		return ratio * ratio;
	}

	// 更新缓存的鼠标位置与边缘光强度，返回是否需要重绘
	bool LiquidGlassEffectPrivate::applyMousePosition(const QPoint& globalPosition) {
		if (!parentWidget) {
			return false;
		}
		const QPoint localPosition = parentWidget->mapFromGlobal(globalPosition);
		const qreal intensity = rimLightIntensityAt(localPosition);
		if (intensity <= 0.0) {
			return clearMousePosition();
		}
		if (intensity == rimLightIntensity && localPosition == mousePosition) {
			return false;
		}
		mousePosition = localPosition;
		rimLightIntensity = intensity;
		return true;
	}

	// 清空鼠标位置与边缘光强度，返回是否需要重绘
	bool LiquidGlassEffectPrivate::clearMousePosition() {
		if (rimLightIntensity <= 0.0) {
			return false;
		}
		rimLightIntensity = 0.0;
		return true;
	}

	// 把事件过滤器挂到控件、控件所在窗口和窗口的QWindow上，重复调用是安全的
	void LiquidGlassEffectPrivate::syncMouseTracking(LiquidGlassEffect* q) {
		if (!parentWidget) {
			return;
		}
		QWidget* windowWidget = parentWidget->window();
		if (windowWidget == parentWidget) {
			windowWidget = nullptr; // 控件本身就是窗口，避免同一个对象被挂两次过滤器
		}
		QWindow* windowHandle = parentWidget->window() ? parentWidget->window()->windowHandle() : nullptr;
		if (trackedParent.data() != parentWidget) {
			if (trackedParent) {
				trackedParent->removeEventFilter(q);
			}
			trackedParent = parentWidget;
			trackedParent->installEventFilter(q);
		}
		if (trackedWidget.data() != windowWidget) {
			if (trackedWidget) {
				trackedWidget->removeEventFilter(q);
			}
			trackedWidget = windowWidget;
			if (trackedWidget) {
				trackedWidget->installEventFilter(q);
			}
		}
		if (trackedWindow.data() != windowHandle) {
			if (trackedWindow) {
				trackedWindow->removeEventFilter(q);
			}
			trackedWindow = windowHandle;
			if (trackedWindow) {
				trackedWindow->installEventFilter(q);
			}
		}
	}

	// 卸载全部事件过滤器并熄灭边缘光
	void LiquidGlassEffectPrivate::releaseMouseTracking(LiquidGlassEffect* q) {
		if (trackedParent) {
			trackedParent->removeEventFilter(q);
			trackedParent = nullptr;
		}
		if (trackedWidget) {
			trackedWidget->removeEventFilter(q);
			trackedWidget = nullptr;
		}
		if (trackedWindow) {
			trackedWindow->removeEventFilter(q);
			trackedWindow = nullptr;
		}
		rimLightIntensity = 0.0;
	}

	/*
		把一组边界点按各自的权重（0~1）向内侧扩散成半径 thickness 的软光晕，再按颜色输出成 ARGB 图像。
		同一个像素取所有覆盖它的光晕中的最大值，因此相邻边界点扩散出的光晕重叠时既不会出现接缝，也不会叠加变亮。
	*/
	QImage LiquidGlassEffectPrivate::splatBoundaryGlow(const QSize& size, const QVector<QPoint>& points, const QVector<qreal>& weights, int thickness, const QColor& color, QRect* dirtyRect) {
		const int width = size.width();
		const int height = size.height();
		const qreal thickness2 = qreal(thickness) * thickness;
		const qreal maxAlpha = color.alphaF() * 255.0;
		// 先在单通道缓冲上做最大值累积，最后再一次性转成颜色
		quint8* buffer = new quint8[width * height]();
		int top = height;
		int bottom = -1;
		int left = width;
		int right = -1;
		for (int i = 0; i < points.size() && i < weights.size(); i++) {
			const qreal weight = weights[i];
			if (weight <= 0.0) {
				continue;
			}
			const qreal alpha = weight * maxAlpha;
			const int xBegin = std::max(points[i].x() - thickness, 0);
			const int xEnd = std::min(points[i].x() + thickness, width - 1);
			const int yBegin = std::max(points[i].y() - thickness, 0);
			const int yEnd = std::min(points[i].y() + thickness, height - 1);
			top = std::min(top, yBegin);
			bottom = std::max(bottom, yEnd);
			left = std::min(left, xBegin);
			right = std::max(right, xEnd);
			for (int y = yBegin; y <= yEnd; y++) {
				const int offsetY = y - points[i].y();
				quint8* line = buffer + y * width;
				for (int x = xBegin; x <= xEnd; x++) {
					const int offsetX = x - points[i].x();
					const qreal falloff = 1.0 - qreal(offsetX * offsetX + offsetY * offsetY) / thickness2;
					if (falloff <= 0.0) {
						continue;
					}
					const quint8 value = quint8(std::clamp(alpha * falloff * falloff, 0.0, 255.0));
					if (value > line[x]) {
						line[x] = value;
					}
				}
			}
		}
		QImage image(size, QImage::Format_ARGB32);
		image.fill(0);
		if (bottom >= top) {
			const quint32 rgb = (quint32(color.red()) << 16) | (quint32(color.green()) << 8) | quint32(color.blue());
			quint32* imageBits = reinterpret_cast<quint32*>(image.bits());
			for (int y = top; y <= bottom; y++) {
				const quint8* line = buffer + y * width;
				for (int x = 0; x < width; x++) {
					const quint8 alpha = line[x];
					if (alpha != 0) {
						imageBits[y * width + x] = (quint32(alpha) << 24) | rgb;
					}
				}
			}
		}
		delete[] buffer;
		if (dirtyRect) {
			*dirtyRect = bottom >= top ? QRect(left, top, right - left + 1, bottom - top + 1) : QRect();
		}
		return image;
	}

	/*
		鼠标边缘光的核心：找出离鼠标最近的那一个边界点，以它为原点沿边界向两侧衰减。
	*/
	QImage LiquidGlassEffectPrivate::rimLightGlow(const QSize& size, int borderRadius, const QPoint& mousePosition, int thickness, int range, const QColor& color, QRect* dirtyRect) {
		if (dirtyRect) {
			*dirtyRect = QRect();
		}
		if (size.width() <= 0 || size.height() <= 0 || thickness <= 0 || range <= 0) {
			return QImage();
		}
		// 边界点的间隔不超过厚度的一半，这样相邻边界点扩散出的光晕才能连成一条连续的边缘光
		const QVector<QPoint> points = borderPoints(size, borderRadius, std::max(1, thickness / 2));
		if (points.isEmpty()) {
			return QImage();
		}
		// 离鼠标最近的边界点：边缘光以它为原点沿边界向两侧衰减
		int nearestDistance2 = std::numeric_limits<int>::max();
		for (int i = 0; i < points.size(); i++) {
			const int deltaX = points[i].x() - mousePosition.x();
			const int deltaY = points[i].y() - mousePosition.y();
			nearestDistance2 = std::min(nearestDistance2, deltaX * deltaX + deltaY * deltaY);
		}
		const qreal nearestDistance = std::sqrt(qreal(nearestDistance2));
		QVector<qreal> weights(points.size());
		for (int i = 0; i < points.size(); i++) {
			const int deltaX = points[i].x() - mousePosition.x();
			const int deltaY = points[i].y() - mousePosition.y();
			const qreal weight = 1.0 - (std::sqrt(qreal(deltaX * deltaX + deltaY * deltaY)) - nearestDistance) / range;
			weights[i] = weight > 0.0 ? weight * weight : 0.0; // 离鼠标太远的边界点不发光
		}
		return splatBoundaryGlow(size, points, weights, thickness, color, dirtyRect);
	}

	/*
		把亮度图按乘法调制作用到画面的一块区域上。亮度图的 alpha 是强度、RGB 是光的颜色：
		亮面把现有颜色按比例提亮（dst *= 1 + 光色 * 强度），阴影按比例压暗（dst *= 1 - 补色 * 强度，纯黑阴影即 dst *= 1 - 强度）。
		因为只是在原有颜色上做缩放，色相不会像直接叠白/叠黑那样被冲淡，阴影也等价于“吸掉一部分光”而不是糊一层灰。
		计算用 256 倍定点，避免逐像素浮点。

		additive 为 true 时改用叠加提亮（dst += 光色 * 强度），给鼠标边缘光用：
		边缘光落在哪里是由鼠标决定的，可能正落在全局打光的阴影那一侧，而乘法提亮会被那片已经压暗的画面一起压掉，
		看上去就像边缘光被阴影挡住了；叠加提亮与底色无关，加多少就亮多少。代价是边缘一带会偏白一点，
		不再像乘法那样保留原有色相，所以只用在边缘光这种面积很小的局部高光上。
	*/
	void LiquidGlassEffectPrivate::modulateByLightMap(QImage& target, const QImage& lightMap, const QRect& area, bool brighten, qreal strength, bool additive) {
		const QRect rect = area.intersected(QRect(QPoint(0, 0), target.size()));
		if (rect.isEmpty() || lightMap.isNull() || lightMap.size() != target.size() || strength <= 0.0) {
			return;
		}
		if (strength > 1.0) {
			strength = 1.0;
		}
		const int scale = int(strength * 256.0); // 强度也折成定点，鼠标边缘光的淡入淡出就靠它
		for (int y = rect.top(); y <= rect.bottom(); y++) {
			const quint32* mapLine = reinterpret_cast<const quint32*>(lightMap.constScanLine(y));
			quint32* targetLine = reinterpret_cast<quint32*>(target.scanLine(y));
			for (int x = rect.left(); x <= rect.right(); x++) {
				const quint32 mapPixel = mapLine[x];
				const int alpha = int(mapPixel >> 24) & 0xFF;
				if (alpha == 0) {
					continue;
				}
				const int lightR = int(mapPixel >> 16) & 0xFF;
				const int lightG = int(mapPixel >> 8) & 0xFF;
				const int lightB = int(mapPixel) & 0xFF;
				const int denominator = 255 * 255;
				const quint32 pixel = targetLine[x];
				if (additive) {
					// 叠加提亮：加多少就亮多少，与目标像素原本多暗无关。
					// 分母是 255*256：alpha 归一到 0~1 后乘颜色分量，scale 把强度也折成定点
					const int amountR = (lightR * alpha * scale) / (255 * 256);
					const int amountG = (lightG * alpha * scale) / (255 * 256);
					const int amountB = (lightB * alpha * scale) / (255 * 256);
					const int r = std::clamp(int((pixel >> 16) & 0xFF) + amountR, 0, 255);
					const int g = std::clamp(int((pixel >> 8) & 0xFF) + amountG, 0, 255);
					const int b = std::clamp(int(pixel & 0xFF) + amountB, 0, 255);
					targetLine[x] = (pixel & 0xFF000000) | (quint32(r) << 16) | (quint32(g) << 8) | quint32(b);
					continue;
				}
				int factorR = 0;
				int factorG = 0;
				int factorB = 0;
				if (brighten) {
					factorR = (lightR * alpha * scale) / denominator;
					factorG = (lightG * alpha * scale) / denominator;
					factorB = (lightB * alpha * scale) / denominator;
				}
				else {
					factorR = -((255 - lightR) * alpha * scale) / denominator;
					factorG = -((255 - lightG) * alpha * scale) / denominator;
					factorB = -((255 - lightB) * alpha * scale) / denominator;
				}
				const int r = std::clamp(((int(pixel >> 16) & 0xFF) * (256 + factorR)) >> 8, 0, 255);
				const int g = std::clamp(((int(pixel >> 8) & 0xFF) * (256 + factorG)) >> 8, 0, 255);
				const int b = std::clamp(((int(pixel) & 0xFF) * (256 + factorB)) >> 8, 0, 255);
				targetLine[x] = (pixel & 0xFF000000) | (quint32(r) << 16) | (quint32(g) << 8) | quint32(b);
			}
		}
	}

	/*
		全局打光的单侧生成：沿边界求每个边界点的外法线，再用法线与光方向的点积决定它属于迎光面还是背光面。
		polarity 为 +1 时只画迎光面（亮面），为 -1 时只画背光面（阴影面），两侧的亮度都取点积的平方，
		因此正对光源最亮、与光垂直处归零、背光面完全不参与亮面，明暗沿边界连续变化。
		圆角矩形的外法线直接由圆角矩形的有符号距离场求出：直边得到轴向法线，圆角得到指向圆角中心的径向法线。
		一束光的两面由LiquidGlassEffect::globalRimImage()配对：本函数只生成其中一面，不负责调制。
	*/
	QImage LiquidGlassEffectPrivate::buildGlobalRimSide(const QSize& size, int borderRadius, const QPointF& lightDirection, int thickness, qreal polarity, const QColor& color, QRect* dirtyRect) {
		if (dirtyRect) {
			*dirtyRect = QRect();
		}
		if (size.width() <= 0 || size.height() <= 0 || thickness <= 0 || color.alpha() <= 0) {
			return QImage();
		}
		const qreal directionLength = std::hypot(lightDirection.x(), lightDirection.y());
		if (directionLength <= 0.0) {
			return QImage(); // 光方向为零向量，等于没有光源
		}
		const qreal directionX = lightDirection.x() / directionLength;
		const qreal directionY = lightDirection.y() / directionLength;
		const int radius = std::clamp(borderRadius, 0, std::min(size.width(), size.height()) / 2);
		const QVector<QPoint> points = borderPoints(size, radius, std::max(1, thickness / 2));
		if (points.isEmpty()) {
			return QImage();
		}
		const qreal centerX = (size.width() - 1) / 2.0;
		const qreal centerY = (size.height() - 1) / 2.0;
		const qreal halfWidth = centerX - radius;
		const qreal halfHeight = centerY - radius;
		QVector<qreal> weights(points.size());
		for (int i = 0; i < points.size(); i++) {
			const qreal offsetX = points[i].x() - centerX;
			const qreal offsetY = points[i].y() - centerY;
			const qreal squareX = std::max(std::abs(offsetX) - halfWidth, 0.0);
			const qreal squareY = std::max(std::abs(offsetY) - halfHeight, 0.0);
			const qreal squareLength = std::hypot(squareX, squareY);
			if (squareLength <= 0.0) {
				continue;
			}
			const qreal normalX = (squareX / squareLength) * (offsetX < 0 ? -1.0 : 1.0);
			const qreal normalY = (squareY / squareLength) * (offsetY < 0 ? -1.0 : 1.0);
			const qreal dot = normalX * directionX + normalY * directionY;
			const qreal facing = polarity > 0 ? dot : -dot;
			if (facing <= 0.0) {
				continue; // 属于另一面，这次不画
			}
			weights[i] = facing * facing;
		}
		return splatBoundaryGlow(size, points, weights, thickness, color, dirtyRect);
	}

	/*
		高斯核：先把浮点核归一化，再折成 65536 倍的定点，避免逐像素浮点。
	*/
	void LiquidGlassEffectPrivate::buildGaussKernel(int radius, QVector<int>& kernel) {
		QVector<double> weights(radius);
		double sum = 0;
		const double sigma = 0.3 * ((radius - 1) * 0.5 - 1) + 0.8;
		const double sigma2 = sigma * sigma;
		for (int i = 0; i < radius; i++) {
			const double x = i - (radius - 1) / 2.0;
			weights[i] = std::exp(-x * x / (2 * sigma2)) / (std::sqrt(2 * M_PI) * sigma);
			sum += weights[i];
		}
		kernel.resize(radius);
		for (int i = 0; i < radius; i++) {
			kernel[i] = int(weights[i] / sum * 65536);
		}
	}

	// 贴着图像边界的那几个像素要夹住采样坐标（相当于把边界外的像素按边界像素处理）
	void LiquidGlassEffectPrivate::blurAccumulate(const quint32* sourceRow, int x, int width, int halfRadius, const int* kernel, int* sums) {
		int r = 0, g = 0, b = 0, a = 0;
		for (int k = -halfRadius; k <= halfRadius; k++) {
			const int sampleX = std::clamp(x + k, 0, width - 1);
			const quint32 pixel = sourceRow[sampleX];
			const int weight = kernel[k + halfRadius];
			r += int((pixel >> 16) & 0xFF) * weight;
			g += int((pixel >> 8) & 0xFF) * weight;
			b += int(pixel & 0xFF) * weight;
			a += int((pixel >> 24) & 0xFF) * weight;
		}
		sums[0] = r;
		sums[1] = g;
		sums[2] = b;
		sums[3] = a;
	}

	// 中间那一段采样不会越界，直接顺序取核，省掉逐像素的边界判断
	void LiquidGlassEffectPrivate::blurAccumulateInner(const quint32* sourceRow, int x, int halfRadius, const int* kernel, int* sums) {
		const quint32* pixel = sourceRow + (x - halfRadius);
		int r = 0, g = 0, b = 0, a = 0;
		for (int k = 0; k <= 2 * halfRadius; k++, pixel++) {
			const quint32 value = *pixel;
			const int weight = kernel[k];
			r += int((value >> 16) & 0xFF) * weight;
			g += int((value >> 8) & 0xFF) * weight;
			b += int(value & 0xFF) * weight;
			a += int((value >> 24) & 0xFF) * weight;
		}
		sums[0] = r;
		sums[1] = g;
		sums[2] = b;
		sums[3] = a;
	}

	/*
		一维卷积。卷积是逐点独立的：某个输出像素只依赖它周围 halfRadius 个源像素，
		因此对任意一块区域单独调用这个函数，得到的结果都与整张一起算时完全一致。
	*/
	void LiquidGlassEffectPrivate::blurPass1D(const quint32* source, quint32* target, int width, int height, const int* kernel, int halfRadius, const QRect& area, bool horizontal) {
		const QRect rect = area.intersected(QRect(0, 0, width, height));
		int sums[4] = { 0, 0, 0, 0 };
		for (int y = rect.top(); y <= rect.bottom(); y++) {
			const quint32* sourceRow = source + qsizetype(y) * width;
			quint32* targetRow = target + qsizetype(y) * width;
			for (int x = rect.left(); x <= rect.right(); x++) {
				if (horizontal) {
					if (x >= halfRadius && x + halfRadius < width) {
						blurAccumulateInner(sourceRow, x, halfRadius, kernel, sums);
					}
					else {
						blurAccumulate(sourceRow, x, width, halfRadius, kernel, sums);
					}
				}
				else {
					int r = 0, g = 0, b = 0, a = 0;
					for (int k = -halfRadius; k <= halfRadius; k++) {
						const int sampleY = std::clamp(y + k, 0, height - 1);
						const quint32 pixel = source[qsizetype(sampleY) * width + x];
						const int weight = kernel[k + halfRadius];
						r += int((pixel >> 16) & 0xFF) * weight;
						g += int((pixel >> 8) & 0xFF) * weight;
						b += int(pixel & 0xFF) * weight;
						a += int((pixel >> 24) & 0xFF) * weight;
					}
					sums[0] = r;
					sums[1] = g;
					sums[2] = b;
					sums[3] = a;
				}
				targetRow[x] = (quint32(std::clamp(sums[3] >> 16, 0, 255)) << 24)
					| (quint32(std::clamp(sums[0] >> 16, 0, 255)) << 16)
					| (quint32(std::clamp(sums[1] >> 16, 0, 255)) << 8)
					| quint32(std::clamp(sums[2] >> 16, 0, 255));
			}
		}
	}

	// 整张模糊：横向一遍、纵向一遍（先横后纵，与分块版本用的是同一段代码，因此结果完全一致）
	QImage LiquidGlassEffectPrivate::blurImageFull(const QImage& image, int radius) {
		if (radius <= 0) {
			return image;
		}
		if (radius % 2 == 0) {
			radius++;
		}
		const QImage source = image.convertToFormat(QImage::Format_ARGB32);
		const int width = source.width();
		const int height = source.height();
		if (width <= 0 || height <= 0) {
			return image;
		}
		QVector<int> kernel;
		buildGaussKernel(radius, kernel);
		const int halfRadius = radius / 2;
		const QRect full(0, 0, width, height);
		QImage scratch(source.size(), QImage::Format_ARGB32);
		QImage blurred(source.size(), QImage::Format_ARGB32);
		blurPass1D(reinterpret_cast<const quint32*>(source.constBits()), reinterpret_cast<quint32*>(scratch.bits()), width, height, kernel.constData(), halfRadius, full, true);
		blurPass1D(reinterpret_cast<const quint32*>(scratch.constBits()), reinterpret_cast<quint32*>(blurred.bits()), width, height, kernel.constData(), halfRadius, full, false);
		return blurred;
	}

	/*
		只重算被改动区域的分块模糊：blurredBase 是同一张图（未改动时）的完整模糊结果，changedRect 是这次改动的区域。
		纵向卷积的输出只依赖横向卷积在上下各 halfRadius 行内的值，横向卷积在每个点又只依赖左右各 halfRadius 列的源像素，
		所以横向要多算两圈、纵向多算一圈，这样分块的结果与整张重算的结果逐像素相同，不会在分块边界上留下接缝。
		changedRect 为空表示这次没有任何改动，直接把 blurredBase 交回去。
	*/
	QImage LiquidGlassEffectPrivate::blurImageBand(const QImage& image, int radius, const QImage& blurredBase, const QRect& changedRect, QImage& scratch) {
		if (radius <= 0) {
			return image;
		}
		if (blurredBase.isNull() || blurredBase.size() != image.size()) {
			return blurImageFull(image, radius);
		}
		if (changedRect.isEmpty()) {
			return blurredBase; // 这次没有任何改动，直接用缓存好的模糊结果
		}
		if (radius % 2 == 0) {
			radius++;
		}
		QVector<int> kernel;
		buildGaussKernel(radius, kernel);
		const int halfRadius = radius / 2;
		const int width = image.width();
		const int height = image.height();
		const QRect full(0, 0, width, height);
		const QRect changed = changedRect.intersected(full);
		if (changed.isEmpty()) {
			return blurredBase;
		}
		const QRect vertical = changed.adjusted(-halfRadius, -halfRadius, halfRadius, halfRadius).intersected(full);
		const QRect horizontal = changed.adjusted(-2 * halfRadius, -2 * halfRadius, 2 * halfRadius, 2 * halfRadius).intersected(full);
		if (scratch.size() != image.size()) {
			scratch = QImage(image.size(), QImage::Format_ARGB32);
		}
		blurPass1D(reinterpret_cast<const quint32*>(image.constBits()), reinterpret_cast<quint32*>(scratch.bits()), width, height, kernel.constData(), halfRadius, horizontal, true);
		QImage result = blurredBase; // 隐式共享，下面只写改动区域，其余部分沿用缓存的模糊结果
		blurPass1D(reinterpret_cast<const quint32*>(scratch.constBits()), reinterpret_cast<quint32*>(result.bits()), width, height, kernel.constData(), halfRadius, vertical, false);
		return result;
	}

	/*
		帧缓存的指纹：只要下面这些参数不变，“扭曲 + 全局光之后、边缘光之前”的图像就完全一样。
	*/
	quint64 LiquidGlassEffectPrivate::frameKey(const QSize& size) const {
		const QRect widgetGeometry = parentWidget ? parentWidget->geometry() : QRect();
		return quint64(qHashMulti(size_t(0),
			quint32(effectTypes),
			size.width(),
			size.height(),
			widgetGeometry.x(),
			widgetGeometry.y(),
			customGeometry.x(),
			customGeometry.y(),
			customGeometry.width(),
			customGeometry.height(),
			liquidDistortRadius,
			blurRadius,
			borderRadius,
			liquidDistortDispersion,
			quint32(backgroundPolicy),
			backgroundImage.cacheKey(),
			globalRimLightDirection.x(),
			globalRimLightDirection.y(),
			globalRimLightColor.rgba(),
			globalRimLightThickness,
			globalRimShadowColor.rgba(),
			globalRimShadowThickness));
	}

	void LiquidGlassEffectPrivate::clearFrameCache() {
		frameBaseCache = QImage();
		frameBlurredCache = QImage();
		frameScratch = QImage();
		frameCacheSize = QSize(-1, -1);
		frameCacheKey = 0;
		frameCacheValid = false;
		frameCacheDirty = false;
	}

	// 采样矩形里任何一个控件重绘，都可能让被模糊源跟着变，所以要把矩形内所有控件都监听上。
	// 与之无关的分支直接剪掉（不在矩形里，重绘它不影响我们），玻璃控件自己那一支也要排除：
	// 它画的是玻璃自己的内容（由本效果画在最上层），而且鼠标一动它就会重绘，监听了缓存就永远用不上。
	void LiquidGlassEffectPrivate::collectRenderWatched(QWidget* widget, QWidget* source, QWidget* exclude, const QRect& rect, QVector<QPointer<QWidget>>& out) {
		if (!widget || widget == exclude) {
			return;
		}
		if (widget != source) {
			const QRect localRect(widget->mapTo(source, QPoint(0, 0)), widget->size());
			if (!localRect.intersects(rect)) {
				return;
			}
		}
		out.append(widget);
		const QObjectList children = widget->children();
		for (int i = 0; i < children.size(); i++) {
			QWidget* child = qobject_cast<QWidget*>(children[i]);
			if (child) {
				collectRenderWatched(child, source, exclude, rect, out);
			}
		}
	}

	/*
		同步重绘监听。监听的对象是“被渲染出来的那个控件”（draw()里render的来源）以及它在采样矩形内的所有后代，
		任何一个重绘都会把帧缓存标记成过期。控件树、位置变化时这个集合会变，所以要重新收集一次；
		集合没变就什么都不做，免得布局一动就反复装卸事件过滤器。
	*/
	void LiquidGlassEffectPrivate::syncRenderWatching(LiquidGlassEffect* q) {
		QWidget* source = nullptr;
		if (parentWidget && backgroundPolicy == LiquidGlassEffect::BackgroundPolicy::Render) {
			source = parentWidget->parentWidget();
		}
		QVector<QPointer<QWidget>> wanted;
		if (source) {
			collectRenderWatched(source, source, parentWidget, parentWidget->geometry(), wanted);
		}
		if (renderSource.data() == source && renderWatched == wanted) {
			return;
		}
		releaseRenderWatching(q);
		renderSource = source;
		for (int i = 0; i < wanted.size(); i++) {
			if (wanted[i]) {
				wanted[i]->installEventFilter(q);
			}
		}
		renderWatched = wanted;
		// 监听范围变了说明控件树动过了，这一帧的缓存不能再信
		frameCacheDirty = true;
	}

	void LiquidGlassEffectPrivate::releaseRenderWatching(LiquidGlassEffect* q) {
		for (int i = 0; i < renderWatched.size(); i++) {
			if (renderWatched[i]) {
				renderWatched[i]->removeEventFilter(q);
			}
		}
		renderWatched.clear();
		renderSource = nullptr;
	}

	/*
		Render策略下取被模糊源：把玻璃所在的那个父控件渲染一遍，同时要把玻璃自己摘出去。
		Qt 渲染子控件时会照常应用它们的QGraphicsEffect（也就是会再进本效果的draw()），
		所以不摘的话，“玻璃自己应用效果之后的样子”会被一并采进被模糊源里，等于玻璃把自己叠了一层。
		摘自己顺带也避免了“在渲染父控件的中途又去渲染父控件”这种自反递归。

		摘的办法是把玻璃自己的Qt::WA_WState_Hidden临时置上：Qt渲染子控件时看的正是这个属性（isHidden()），
		它会连同子控件一起跳过，而且只改标志位、不请求重绘。这里**不能**用hide()或setUpdatesEnabled()：
		那两者都会顺手请求重绘，于是“重绘→采样→再请求重绘”会变成自持的循环（实测能把一个CPU核心跑满）。

		压在玻璃下面的兄弟控件和父控件自己的背景都照常被渲染，层次也和直接渲染父控件时一致；
		其中带QGraphicsEffect的兄弟控件是能透出来的（模糊、色散都能叠），因为Qt会照常应用它的效果。
		但它得能自己完成绘制：CustomImage策略的玻璃不需要再渲染父控件，所以能正常参与合成；
		Render策略的玻璃在被上层采样、而它自己也正好需要重新采样时，会一头撞进“渲染父控件的中途再渲染父控件”，
		那次采样拿不到东西，采样里就只剩它未经效果的原始内容（看得见字和图形，看不见它自己的模糊）。

		摘出去和恢复都由SamplingScope负责（采样会嵌套，掩细节见SamplingScope的说明）。
	*/
	void LiquidGlassEffectPrivate::renderParentBackground(QImage& target) {
		if (!parentWidget) {
			return;
		}
		QWidget* source = parentWidget->parentWidget();
		if (!source) {
			return;
		}
		// 守卫在自己构造时把本控件标为隐藏、把外层采样控件临时恢复可见，析构时无条件还原
		const SamplingScope scope(parentWidget);
		source->render(&target, QPoint(), QRegion(parentWidget->geometry()));
	}

	bool LiquidGlassEffectPrivate::watchesRenderWidget(const QObject* object) const {
		for (int i = 0; i < renderWatched.size(); i++) {
			if (renderWatched[i].data() == object) {
				return true;
			}
		}
		return false;
	}
}