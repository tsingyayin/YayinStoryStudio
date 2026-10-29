#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <QtCore/qhashfunctions.h>
#include <QtCore/qpointer.h>
#include <QtCore/qscopeguard.h>
#include <QtCore/qset.h>
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
		qreal liquidDistortStrength = 1.5;
		qreal liquidDistortEnergy = 0.5;
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
		QColor rimLightColor = QColor(255, 255, 255, 72);
		int rimLightThickness = 2;
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
		int globalRimLightThickness = 2;
		QColor globalRimShadowColor = QColor(0, 0, 0, 60);
		int globalRimShadowThickness = 2;
	protected:
		// 鼠标一动边缘光就要重绘，而除了边缘光以外的东西全都不变，边缘光又只影响边界附近的一小块。
		// 所以把“扭曲+全局光之后、边缘光之前”的图像和它的模糊结果缓存下来，
		// 鼠标移动时只重算被边缘光影响到的那一条带，不必把整张图重新模糊一遍。
		QImage frameBaseCache;
		// 先模糊后扭曲之后，边缘光不再需要“带内重模糊”，这两项保留给 blurImageBand() 使用
		QImage frameBlurredCache;
		QImage frameScratch;
		QSize frameCacheSize = QSize(-1, -1);
		quint64 frameCacheKey = 0;
		bool frameCacheValid = false;
		// Render策略下被模糊源是每次从父控件现渲染出来的，随时可能因为别人重绘而变，
		// 所以把采样矩形内所有控件都监听上：有人重绘就把帧缓存作废，没人重绘就一直用缓存。
		QPointer<QWidget> renderSource;
		QVector<QPointer<QWidget>> renderWatched;
		QSet<QWidget*> renderWatchedSet;
		bool frameCacheDirty = false;
		bool drawing = false;
		qreal opacity = 1.0;
	protected:
		static constexpr qreal maxRimLightBlend = 0.5;
		static void roundedRectField(const QSize& size, qreal radius, qreal px, qreal py,
			qreal& distance, qreal& normalX, qreal& normalY, qreal& boundaryX, qreal& boundaryY);
		static QImage buildGlobalRimSide(const QSize& size, int borderRadius, const QPointF& lightDirection, int thickness, qreal polarity, const QColor& color, QRect* dirtyRect = nullptr);
		static QImage rimLightGlow(const QSize& size, int borderRadius, const QPoint& mousePosition, int thickness, int range, const QColor& color,
			QRect* dirtyRect = nullptr);
		static void modulateByLightMap(QImage& target, const QImage& lightMap, const QRect& area, bool brighten,
			qreal strength = 1.0, bool additive = false, const QPoint& lightMapOrigin = QPoint());
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
		void clearBlurCache();
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

		LiquidGlassEffect 是一个 QGraphicsEffect 的子类，提供了一种模仿现代操作系统中液态玻璃的图形效果。
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
		模糊先于扭曲应用，扭曲压缩的是已经糊过的画面。
		边缘光比较特殊，它不是一直存在的效果，只在鼠标靠近控件时才会出现；而全局打光是一直存在的，它的光来自一个固定的方向，不需要鼠标参与。

		\section1 扭曲与色散
		扭曲模仿的是玻璃边缘对光的折射：只有边缘区域会被扭曲，图像中心保持不变，半径越大被扭曲的边缘越宽。扭曲有两种：
		\list
		\li EffectType::NormalDistort：三个颜色通道用同一套映射，就是普通的扭曲效果。
		\li EffectType::DispersionDistort：三个通道取样的深度略有不同（绿光不动，红光取得更深、蓝光更浅），
		于是边缘会出现彩色镶边，模仿真实玻璃的色散；它比普通扭曲略贵一点，不追求这个效果时用普通扭曲即可
		\endlist
		扭曲带的宽度（半径）由setLiquidDistortRadius()设置，扭曲带的强度由setLiquidDistortStrength()设置，
		简而言之就是将多少倍于半径的，靠近控件中心的区域以非均匀的映射压缩到半径内，
		越靠近控件边缘的位置，同等宽度下对应的采样区域越宽，也即图像被压缩的越厉害。

		色散强度通过setLiquidDistortDispersion()调整。

		\section1 边缘光
		边缘光模仿的是玻璃边缘被光打亮的样子，它通过EffectType::RimLight开启（包含在EffectType::All中）。
		与其他两种效果不同，边缘光只在鼠标靠近控件的时候才会出现，而且总是出现在离鼠标最近的那一段控件边界上：
		鼠标沿着边界移动时，边缘光会跟着在边界上滑动，看上去就像是控件边缘在反射鼠标位置的光。

		边缘光的颜色、厚度、沿边界延伸的距离以及激活距离分别通过setRimLightColor()、setRimLightThickness()、
		setRimLightRange()和setRimLightActivationDistance()调整。
		其中激活距离决定了鼠标离控件边界多远时边缘光开始出现，超过这个距离时边缘光完全消失。
		边缘光是叠加提亮的，不像全局打光那样与画面相乘，因此它不会被阴影压暗。

		\section1 全局打光
		全局打光模仿的是真实世界里的光源：控件不再是均匀发亮的，而是有一个固定的来光方向，
		迎光的那一面被照亮、与光垂直的两侧完全没有光、背光的那一面则压出一条暗边作为阴影，
		这样控件看上去就有了“厚度”和“被光照到”的体积感。它通过EffectType::GlobalRim开启（包含在EffectType::All中）。

		光的方向通过setGlobalRimLightDirection()设置，取值是一个从控件指向光源的向量（不需要是单位向量，
		以屏幕坐标为准、y轴向下），默认QPointF(-0.5, -0.866)，也就是从左上角来、与竖直方向成30度的光：
		上边界最亮，左边界次之，右边界是浅阴影，下边界是深阴影，四个圆角处自然地从明过渡到暗。
		亮面的颜色和厚度分别通过setGlobalRimLightColor()和setGlobalRimLightThickness()设置，
		阴影的颜色和厚度分别通过setGlobalRimShadowColor()和setGlobalRimShadowThickness()设置。

		\section1 效果透明度
		整体透明度通过opacity()和setOpacity()设置，取值范围为0到1，默认值为1。
		它会同时控制液态玻璃效果及其上层源控件内容的最终绘制透明度：
		0表示完全不绘制，1表示保持原始不透明度。
		此属性声明为Qt的opacity属性，可以直接使用QPropertyAnimation对其进行动画，
		适合实现浮动面板的渐入渐出。它与setColorMask()中的percent不同，
		后者只控制颜色遮罩层的强度，不会让整个效果及源控件一起淡出。

		总的来说，此功能合并了QOpacityGraphicsEffect的效果。

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
		\value All 同时应用普通扭曲、色散扭曲、模糊、边缘光和全局打光效果，模糊优先于扭曲。此项也是默认值。
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
		if (d->effectTypes == effects) {
			return;
		}
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
		if (d->backgroundImage.cacheKey() == image.cacheKey() && d->backgroundImage.size() == image.size()) {
			return;
		}
		d->backgroundImage = image;
		d->clearFrameCache();
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
		if (d->liquidDistortRadius == radius) {
			return;
		}
		d->liquidDistortRadius = radius;
		d->clearFrameCache();
		update();
	}

	/*!
		\since Visindigo 0.17.0
		设置扭曲的色散强度，仅在EffectType中包含DispersionDistort时有效。
		\a dispersion 色散强度，取值会被夹到 0 到 1 之间，默认0.2。
		它是色散分离量相对扭曲半径的比例：1 表示红蓝两个通道的取样深度相差一个完整的扭曲半径。
		色散就是让红绿蓝三个通道取样的深度略有不同（绿光不动，红光取得更深、蓝光取得更浅），
		于是边缘会出现彩色镶边，看起来就像真实玻璃把光拆开了。
		
		数值越大，色散越明显。

		色散会让扭曲略微变慢。
	*/
	void LiquidGlassEffect::setLiquidDistortDispersion(qreal dispersion) {
		dispersion = std::clamp(dispersion, 0.0, 1.0);
		if (qFuzzyCompare(d->liquidDistortDispersion, dispersion)) {
			return;
		}
		d->liquidDistortDispersion = dispersion;
		d->clearFrameCache();
		update();
	}

	/*!
		\since Visindigo 0.18.0
		设置液体扭曲的强度，仅在EffectType中包含NormalDistort或DispersionDistort时有效。
		\a strength 扭曲强度，取值会被夹到 1 到 64 之间，默认1.5。1 表示完全不扭曲。
		
		直观上讲，它是将多少倍于半径的，靠近控件中心的区域以非均匀的映射压缩到半径内，
		越靠近控件边缘的位置，同等宽度下对应的采样区域越宽，也即图像被压缩的越厉害。

		假设设置扭曲半径为5，强度为10，则就是将半径往里走50像素的区域压缩到5像素之内。
	*/
	void LiquidGlassEffect::setLiquidDistortStrength(qreal strength) {
		strength = std::clamp(strength, 1.0, 64.0);
		if (qFuzzyCompare(d->liquidDistortStrength, strength)) {
			return;
		}
		d->liquidDistortStrength = strength;
		d->clearFrameCache();
		update();
	}

	/*!
		\since Visindigo 0.18.0
		设置扭曲时的能量补偿强度，默认0.5。
		\a energy 取值 0 到 1。玻璃把一片区域压进窄条时，这一片的光其实是被挤到一起的，
		所以边缘处理应比周围略亮一点，0表示不补偿，1表示完全补偿。

		即使是完全补偿，也最多提亮18%左右。且只影响颜色，不影响透明度。
	*/
	void LiquidGlassEffect::setLiquidDistortEnergy(qreal energy) {
		energy = std::clamp(energy, 0.0, 1.0);
		if (qFuzzyCompare(d->liquidDistortEnergy, energy)) {
			return;
		}
		d->liquidDistortEnergy = energy;
		d->clearFrameCache();
		update();
	}

	/*!
		\since Visindigo 0.13.0
		设置边框半径，单位为像素。
		\a radius 边框的半径，值越大边框越圆润。
		如果被应用效果的控件具有圆角，那么就应该同步设置此值以避免输出时溢出应有的视觉边界。
	*/
	void LiquidGlassEffect::setBorderRadius(int radius) {
		radius = std::max(0, radius);
		if (d->borderRadius == radius) {
			return;
		}
		d->borderRadius = radius;
		d->clearFrameCache();
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
		percent = std::clamp(percent, 0.0, 1.0);
		if (d->colorMask == color && qFuzzyCompare(d->percent, percent)) {
			return;
		}
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
		if (d->positionPolicy == policy) {
			return;
		}
		d->positionPolicy = policy;
		d->clearFrameCache();
		update();
	}

	/*!
		\since Visindigo 0.13.0
		设置自定义几何形状，仅在PositionPolicy为CustomGeometry时有效。
		\a geometry 自定义的几何形状，决定了模糊效果在CustomImage上的位置和大小。
	*/
	void LiquidGlassEffect::setCustomGeometry(const QRect& geometry) {
		if (d->customGeometry == geometry) {
			return;
		}
		d->customGeometry = geometry;
		if (d->positionPolicy == PositionPolicy::CustomGeometry) {
			d->clearFrameCache();
			update();
		}
	}

	/*!
		\since Visindigo 0.13.0
		设置获取被模糊源的方法。
		\a policy 获取被模糊源的方法，决定了模糊效果的来源。
	*/
	void LiquidGlassEffect::setBackgroundPolicy(BackgroundPolicy policy) {
		if (d->backgroundPolicy == policy) {
			return;
		}
		d->backgroundPolicy = policy;
		d->clearFrameCache();
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
		radius = std::max(0, radius);
		if (d->blurRadius == radius) {
			return;
		}
		d->blurRadius = radius;
		d->clearBlurCache();
		update();
	}

	/*!
		\since Visindigo 0.17.0
		设置边缘光的颜色，仅在EffectType中包含RimLight时有效。
		\a color 边缘光的颜色，其alpha值决定了边缘光的最大强度，默认是很淡的半透明白色（alpha值72，约28%）。
		无论alpha给多大，边缘光实际叠加的不透明度最多只到 50%，不会把边缘烧成一条白线。
	*/
	void LiquidGlassEffect::setRimLightColor(const QColor& color) {
		d->rimLightColor = color;
		update();
	}

	/*!
		\since Visindigo 0.17.0
		设置边缘光的厚度，仅在EffectType中包含RimLight时有效。
		\a thickness 边缘光从控件边界向内扩散的范围，单位为像素，默认2。
		取 1 时是一条一像素宽的细线，在圆角和斜边上会因为逐像素覆盖不同而显得一段一段，2 以上才会连续。
	*/
	void LiquidGlassEffect::setRimLightThickness(int thickness) {
		if (thickness <= 0) thickness = 1;
		if (d->rimLightThickness == thickness) {
			return;
		}
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
		if (d->rimLightRange == range) {
			return;
		}
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
		if (d->rimLightActivationDistance == distance) {
			return;
		}
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
		if (d->globalRimLightDirection == direction) {
			return;
		}
		d->globalRimLightDirection = direction;
		d->clearFrameCache();
		update();
	}

	/*!
		\since Visindigo 0.17.0
		设置全局打光亮面的颜色，仅在EffectType中包含GlobalRim时有效。
		\a color 迎光面的颜色，其alpha值决定了亮面的最大强度，默认白色、alpha值90（刻意做得比较淡）。
		全局光只是一个效果元素，默认不应该盖过玻璃本身的颜色，需要更强的光时把alpha值调大即可。
	*/
	void LiquidGlassEffect::setGlobalRimLightColor(const QColor& color) {
		if (d->globalRimLightColor == color) {
			return;
		}
		d->globalRimLightColor = color;
		d->clearFrameCache();
		update();
	}

	/*!
		\since Visindigo 0.17.0
		设置全局打光亮面的厚度，仅在EffectType中包含GlobalRim时有效。
		\a thickness 亮面从控件边界向内扩散的范围，单位为像素，默认1。
	*/
	void LiquidGlassEffect::setGlobalRimLightThickness(int thickness) {
		if (thickness <= 0) thickness = 1;
		if (d->globalRimLightThickness == thickness) {
			return;
		}
		d->globalRimLightThickness = thickness;
		d->clearFrameCache();
		update();
	}

	/*!
		\since Visindigo 0.17.0
		设置全局打光的阴影颜色，仅在EffectType中包含GlobalRim时有效。
		\a color 背光面阴影的颜色，其alpha值决定了阴影的强度，默认半透明黑色（alpha值60）。
	*/
	void LiquidGlassEffect::setGlobalRimShadowColor(const QColor& color) {
		if (d->globalRimShadowColor == color) {
			return;
		}
		d->globalRimShadowColor = color;
		d->clearFrameCache();
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
		if (d->globalRimShadowThickness == thickness) {
			return;
		}
		d->globalRimShadowThickness = thickness;
		d->clearFrameCache();
		update();
	}

	/*!
		\since Visindigo 0.17.0
		\brief 获取液态玻璃效果的整体透明度。

		返回值范围为0到1，默认值为1。该值同时作用于液态玻璃效果及其上层源控件内容的最终绘制结果。
	*/
	qreal LiquidGlassEffect::opacity() const {
		return d->opacity;
	}

	/*!
		\since Visindigo 0.17.0
		\brief 设置液态玻璃效果的整体透明度。

		\a opacity 透明度，取值范围为0到1，超出范围的值会被自动限制。
		0表示完全不绘制，1表示保持原始不透明度。

		该属性可通过QPropertyAnimation进行动画，例如：
		\code
		auto* animation = new QPropertyAnimation(effect, "opacity", effect);
		animation->setStartValue(0.0);
		animation->setEndValue(1.0);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
		\endcode
	*/
	void LiquidGlassEffect::setOpacity(qreal opacity) {
		opacity = std::clamp(opacity, 0.0, 1.0);
		if (qFuzzyCompare(d->opacity, opacity)) {
			return;
		}
		d->opacity = opacity;
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
		\a liquidDistortRadius 扭曲效果的半径，值越大扭曲的绘制范围越大。
		\a liquidDistortDispersion 色散强度，取值 0 到 1，默认0（不做色散）。
		\a liquidDistortStrength 扭曲强度，取值 1 到 64，默认1.5。1 表示完全不扭曲（一个像素都不动）。
		\a borderRadius 图像四角的圆角半径，单位为像素，默认0。
		\a energyRatio 能量补偿强度，取值 0 到 1。
		
		有关各参数的详细说明，可以参见本类对应的set方法。
	*/
	QImage LiquidGlassEffect::distortImage(const QImage& coverdArea, int liquidDistortRadius, qreal dispersion, qreal strength, int borderRadius, qreal energyRatio) {
		if (coverdArea.isNull()) {
			return QImage();
		}
		const QImage source = coverdArea.format() == QImage::Format_ARGB32
			? coverdArea : coverdArea.convertToFormat(QImage::Format_ARGB32);
		QImage result = source;
		const int width = source.width();
		const int height = source.height();
		liquidDistortRadius = std::min(liquidDistortRadius, std::min(width, height) / 2);
		if (liquidDistortRadius <= 0) {
			return result;
		}
		const int radius = liquidDistortRadius;
		const qreal clampedStrength = std::clamp(strength, 1.0, 64.0);
		const qreal depthAtEdge = (clampedStrength - 1.0) * radius;
		if (depthAtEdge <= 0.0) {
			return result;
		}
		const qreal clampedDispersion = std::clamp(dispersion, 0.0, 1.0);

		const qreal dispersionShift = clampedDispersion * radius;
		const bool dispersive = dispersionShift > 0.0;
		const quint32* sourceBits = reinterpret_cast<const quint32*>(source.constBits());
		quint32* resultBits = reinterpret_cast<quint32*>(result.bits());
		const qsizetype rowStep = qsizetype(width);
		const qreal centerX = width / 2.0;
		const qreal centerY = height / 2.0;
		const qreal cornerRadius = std::clamp(qreal(borderRadius), 0.0, qreal(std::min(width, height)) / 2.0);
		const qreal coreHalfWidth = width / 2.0 - cornerRadius;
		const qreal coreHalfHeight = height / 2.0 - cornerRadius;

		const int margin = std::min(std::max(1, int(std::ceil(radius + cornerRadius))), std::max(width, height));
		const int maxTaps = 65;

		const qreal maxEnergyGain = 0.18;
		const qreal energy = std::clamp(energyRatio, 0.0, 1.0);

		auto fieldAt = [&](qreal px, qreal py, qreal& distance, qreal& normalX, qreal& normalY) {
			const qreal dx = px - centerX;
			const qreal dy = py - centerY;
			const qreal qx = std::abs(dx) - coreHalfWidth;
			const qreal qy = std::abs(dy) - coreHalfHeight;
			const qreal ox = std::max(qx, 0.0);
			const qreal oy = std::max(qy, 0.0);
			const qreal outside = std::sqrt(ox * ox + oy * oy);
			distance = cornerRadius - outside - std::min(std::max(qx, qy), 0.0);
			if (outside > 0.0) {
				normalX = ox * (dx < 0.0 ? 1.0 : -1.0) / outside;
				normalY = oy * (dy < 0.0 ? 1.0 : -1.0) / outside;
			}
			else if (qx > qy) {
				normalX = dx < 0.0 ? 1.0 : -1.0;
				normalY = 0.0;
			}
			else {
				normalX = 0.0;
				normalY = dy < 0.0 ? 1.0 : -1.0;
			}
		};

		struct SampleTap {
			int x0;
			int x1;
			int y0;
			int y1;
			qreal fx;
			qreal fy;
		};
		auto makeTap = [&](qreal sx, qreal sy) {
			const qreal cx = std::clamp(sx, 0.0, qreal(width - 1));
			const qreal cy = std::clamp(sy, 0.0, qreal(height - 1));
			const int xi = int(cx);
			const int yi = int(cy);
			return SampleTap{ xi, std::min(xi + 1, width - 1), yi, std::min(yi + 1, height - 1), cx - xi, cy - yi };
		};
		// 通道下标按 ARGB32 的字节序：0 = A、1 = R、2 = G、3 = B（与 quint32 的高位到低位一致）
		auto bilinearChannel = [&](const SampleTap& tap, int channel) {
			const int bits = 24 - channel * 8;
			const quint32* row0 = sourceBits + qsizetype(tap.y0) * rowStep;
			const quint32* row1 = sourceBits + qsizetype(tap.y1) * rowStep;
			const qreal v00 = qreal((row0[tap.x0] >> bits) & 0xFFu);
			const qreal v10 = qreal((row0[tap.x1] >> bits) & 0xFFu);
			const qreal v01 = qreal((row1[tap.x0] >> bits) & 0xFFu);
			const qreal v11 = qreal((row1[tap.x1] >> bits) & 0xFFu);
			const qreal top = v00 + (v10 - v00) * tap.fx;
			const qreal bottom = v01 + (v11 - v01) * tap.fx;
			return top + (bottom - top) * tap.fy;
		};
		auto bilinearPixel = [&](const SampleTap& tap, qreal channels[4]) {
			const quint32* row0 = sourceBits + qsizetype(tap.y0) * rowStep;
			const quint32* row1 = sourceBits + qsizetype(tap.y1) * rowStep;
			const quint32 p00 = row0[tap.x0];
			const quint32 p10 = row0[tap.x1];
			const quint32 p01 = row1[tap.x0];
			const quint32 p11 = row1[tap.x1];
			for (int channel = 0; channel < 4; channel++) {
				const int bits = 24 - channel * 8;
				const qreal v00 = qreal((p00 >> bits) & 0xFFu);
				const qreal v10 = qreal((p10 >> bits) & 0xFFu);
				const qreal v01 = qreal((p01 >> bits) & 0xFFu);
				const qreal v11 = qreal((p11 >> bits) & 0xFFu);
				const qreal top = v00 + (v10 - v00) * tap.fx;
				const qreal bottom = v01 + (v11 - v01) * tap.fx;
				channels[channel] = top + (bottom - top) * tap.fy;
			}
		};

		auto boxPixel = [&](qreal px, qreal py, qreal normalX, qreal normalY, qreal nearOffset, qreal farOffset, qreal channels[4]) {
			channels[0] = 0.0;
			channels[1] = 0.0;
			channels[2] = 0.0;
			channels[3] = 0.0;
			const qreal span = std::max(0.0, farOffset - nearOffset);
			const int taps = std::clamp(int(std::ceil(span)), 1, maxTaps);
			qreal sample[4] = { 0.0, 0.0, 0.0, 0.0 };
			for (int i = 0; i < taps; i++) {
				const qreal offset = nearOffset + span * (i + 0.5) / taps;
				bilinearPixel(makeTap(px + normalX * offset, py + normalY * offset), sample);
				channels[0] += sample[0];
				channels[1] += sample[1];
				channels[2] += sample[2];
				channels[3] += sample[3];
			}
			for (int channel = 0; channel < 4; channel++) {
				channels[channel] /= taps;
			}
		};
		auto boxChannel = [&](qreal px, qreal py, qreal normalX, qreal normalY, qreal nearOffset, qreal farOffset, int channel) {
			const qreal span = std::max(0.0, farOffset - nearOffset);
			const int taps = std::clamp(int(std::ceil(span)), 1, maxTaps);
			qreal sum = 0.0;
			for (int i = 0; i < taps; i++) {
				const qreal offset = nearOffset + span * (i + 0.5) / taps;
				sum += bilinearChannel(makeTap(px + normalX * offset, py + normalY * offset), channel);
			}
			return sum / taps;
		};
		auto toByte = [](qreal value) {
			return quint32(std::clamp<long>(std::lround(value), 0, 255));
		};
		auto writeSpan = [&](int y, int xBegin, int xEnd) {
			quint32* resultRow = resultBits + qsizetype(y) * rowStep;
			const qreal py = y + 0.5;
			for (int x = xBegin; x < xEnd; x++) {
				const qreal px = x + 0.5;
				qreal distance = 0.0;
				qreal normalX = 0.0;
				qreal normalY = 0.0;
				fieldAt(px, py, distance, normalX, normalY);
				if (distance <= 0.0 || distance >= radius) {
					continue;
				}
				qreal channels[4] = { 0.0, 0.0, 0.0, 0.0 };
				qreal green[4] = { 0.0, 0.0, 0.0, 0.0 };
				// falloff：该点到带内边界的相对位置，带内边界处为 0、最外那圈像素中心处为 1。
				// 按像素中心归一（最外一圈的 distance 恒为 0.5），这样贴边处的取样深度就是设定的那个值，
				// 不会被半径的取整吃掉。
				const qreal falloffSpan = std::max(0.5, qreal(radius) - 0.5);
				const qreal falloff = std::clamp(1.0 - (distance - 0.5) / falloffSpan, 0.0, 1.0);
				const qreal falloff2 = falloff * falloff;
				const qreal offset = depthAtEdge * falloff2;
				// 相邻输出像素跨过的源像素数（局部放大率）。按它做盒式平均：
				// 核比它窄会欠采样出锯齿，比它宽则会把几十像素的内容糊成一团。
				const qreal filterHalf = depthAtEdge * falloff / radius;
				// compression：这一段里「一个输出像素压进了多少个源像素」。
				// 玻璃把这么多份光通量挤到一个像素上，画面因此变亮；energy 控制把这个亮度还原多少。
				const qreal compression = 2.0 * filterHalf;
				const qreal channelShift = dispersionShift * falloff2;
				boxPixel(px, py, normalX, normalY, offset - filterHalf, offset + filterHalf, green);
				channels[0] = green[0];
				channels[2] = green[2];
				if (dispersive) {
					const qreal redOffset = (depthAtEdge + channelShift) * falloff2;
					const qreal blueOffset = std::max(0.0, depthAtEdge - channelShift) * falloff2;
					channels[1] = boxChannel(px, py, normalX, normalY, redOffset - filterHalf, redOffset + filterHalf, 1);
					channels[3] = boxChannel(px, py, normalX, normalY, blueOffset - filterHalf, blueOffset + filterHalf, 3);
				}
				else {
					channels[1] = green[1];
					channels[3] = green[3];
				}
				// 能量补偿：一个输出像素吃了 compression 个源像素的光，聚光会让它变亮。
				// 但真实玻璃的菲涅尔反射与吸收会吃掉大部分，而且人眼对边缘高光极敏感，
				// 所以这里不按压缩倍数线性提亮，而是一条很快饱和的曲线：最多只提 maxEnergyGain。
				// energy = 0 就是纯平均（亮度不变），只作用在颜色上，alpha 仍是覆盖率的平均。
				if (energy > 0.0 && compression > 1.0) {
					const qreal gain = 1.0 + energy * maxEnergyGain * (1.0 - std::exp(-(compression - 1.0) / 2.0));
					channels[0] *= gain;
					channels[1] *= gain;
					channels[2] *= gain;
				}
				resultRow[x] = (toByte(channels[0]) << 24) | (toByte(channels[1]) << 16)
					| (toByte(channels[2]) << 8) | toByte(channels[3]);
			}
		};
		for (int y = 0; y < height; y++) {
			if (y < margin || y >= height - margin) {
				writeSpan(y, 0, width);
			}
			else {
				writeSpan(y, 0, std::min(margin, width));
				writeSpan(y, std::max(width - margin, 0), width);
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
		\a thickness 边缘光的厚度，即从边界向内扩散的范围，单位为像素，默认2。一像素宽时在圆角上会断断续续，2 以上才连续。
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
		QImage result = image.format() == QImage::Format_ARGB32
			? image : image.convertToFormat(QImage::Format_ARGB32);
		QRect glowRect;
		const QImage glow = LiquidGlassEffectPrivate::rimLightGlow(result.size(), borderRadius, mousePosition, thickness, range, color, &glowRect);
		LiquidGlassEffectPrivate::modulateByLightMap(result, glow, glowRect, true, intensity, true, glowRect.topLeft());
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
		QImage result = image.format() == QImage::Format_ARGB32
			? image : image.convertToFormat(QImage::Format_ARGB32);
		const QSize size = result.size();
		// 阴影是高光的伴生产物：同一束光里点积为负的半边就是阴影面，两面用同一个光源方向和同一个圆角半径
		QRect shadowRect;
		const QImage shadow = LiquidGlassEffectPrivate::buildGlobalRimSide(size, borderRadius, lightDirection, shadowThickness, -1.0, shadowColor, &shadowRect);
		LiquidGlassEffectPrivate::modulateByLightMap(result, shadow, shadowRect, false, 1.0, false, shadowRect.topLeft());
		QRect lightRect;
		const QImage light = LiquidGlassEffectPrivate::buildGlobalRimSide(size, borderRadius, lightDirection, lightThickness, 1.0, lightColor, &lightRect);
		LiquidGlassEffectPrivate::modulateByLightMap(result, light, lightRect, true, 1.0, false, lightRect.topLeft());
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
		alpha = std::clamp(alpha, 0.0, 1.0);
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
		if (!d->parentWidget) {
			painter->drawPixmap(0, 0, sourcePixmap);
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
			base = sampled.format() == QImage::Format_ARGB32
				? sampled : sampled.convertToFormat(QImage::Format_ARGB32);
			// 先模糊、后扭曲：折射带取样的是已经糊过的画面，压缩时不会把细结构撕成锯齿，
			// 也才能看出“多个像素的光被压进一个像素”这个亮度效果。
			// 边缘光在这之后叠加，所以它是清晰的表面反光，不会被糊掉。
			if (d->effectTypes.testAnyFlag(LiquidGlassEffect::EffectType::Blur)) {
				base = blurImage(base, d->blurRadius);
			}
			const bool dispersionDistort = d->effectTypes.testAnyFlag(LiquidGlassEffect::EffectType::DispersionDistort);
			if (dispersionDistort || d->effectTypes.testAnyFlag(LiquidGlassEffect::EffectType::NormalDistort)) {
				base = distortImage(base, d->liquidDistortRadius, dispersionDistort ? d->liquidDistortDispersion : 0.0,
					d->liquidDistortStrength, d->borderRadius, d->liquidDistortEnergy);
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
			const QImage glow = LiquidGlassEffectPrivate::rimLightGlow(lit.size(), d->borderRadius, d->mousePosition,
				d->rimLightThickness, d->rimLightRange, d->rimLightColor, &rimLightRect);
			lit = base;
			LiquidGlassEffectPrivate::modulateByLightMap(lit, glow, rimLightRect, true, d->rimLightIntensity, true, rimLightRect.topLeft());
		}
		// 模糊已经在上面跟着帧缓存一起做完了（先模糊后扭曲），这里只叠边缘光：
		// 边缘光是表面反光，保持清晰，不参与模糊。
		QImage coverdArea = lit;
		QPixmap finalPixmap = sourcePixmap;
		QPainter finalPainter(&finalPixmap);
		// 圆角裁剪必须开抗锯齿。QPainter 的裁剪路径默认是 1 位掩码，圆角和斜边会被切成一格一格的阶梯，
		// 而边缘光/全局光正好贴着这条边界走，于是整条边看上去都是毛的。
		finalPainter.setRenderHint(QPainter::Antialiasing, true);
		// 玻璃图是按逻辑像素算的，pixmap 却是设备像素（高 DPI 下是 1.5 倍、2 倍）。
		// 不开平滑缩放时，一个像素宽的光带会被最近邻"放大"成 1 像素、2 像素交替的虚线，
		// 这就是边缘光看上去一段一段断开的直接原因。
		finalPainter.setRenderHint(QPainter::SmoothPixmapTransform, true);
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
		const qreal painterOpacity = painter->opacity();
		painter->setOpacity(painterOpacity * d->opacity);
		painter->drawPixmap(0, 0, finalPixmap);
		painter->setOpacity(painterOpacity);
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
				case QEvent::Paint: {
					QPaintEvent* paintEvent = static_cast<QPaintEvent*>(event);
					QWidget* paintedWidget = qobject_cast<QWidget*>(watched);
					if (!paintedWidget || !d->renderSource || !d->parentWidget) {
						break;
					}
					const QRegion sourceRegion = paintEvent->region().translated(
						paintedWidget->mapTo(d->renderSource, QPoint()));
					if (!sourceRegion.intersects(QRegion(d->parentWidget->geometry()))) {
						break;
					}
					d->frameCacheDirty = true;
					break;
				}
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
		圆角矩形内部的距离场：distance 是到形状边界的距离（形状内部为正），
		normal 是指向内部的单位法线，boundaryX/boundaryY 是最近的边界点（把该点沿内法线推回边界上的位置）。
		位置和法线都是连续函数，所以用它算出来的光晕天生抗锯齿：
		不会像「沿边界撒圆点」那样在斜边和圆角上留下台阶，也不会因为点距不足出现串珠。
	*/
	void LiquidGlassEffectPrivate::roundedRectField(const QSize& size, qreal radius, qreal px, qreal py,
		qreal& distance, qreal& normalX, qreal& normalY, qreal& boundaryX, qreal& boundaryY) {
		const qreal centerX = size.width() / 2.0;
		const qreal centerY = size.height() / 2.0;
		const qreal coreHalfWidth = centerX - radius;
		const qreal coreHalfHeight = centerY - radius;
		const qreal dx = px - centerX;
		const qreal dy = py - centerY;
		const qreal qx = std::abs(dx) - coreHalfWidth;
		const qreal qy = std::abs(dy) - coreHalfHeight;
		const qreal ox = std::max(qx, 0.0);
		const qreal oy = std::max(qy, 0.0);
		const qreal outside = std::sqrt(ox * ox + oy * oy);
		distance = radius - outside - std::min(std::max(qx, qy), 0.0);
		if (outside > 0.0) {
			normalX = ox * (dx < 0.0 ? 1.0 : -1.0) / outside;
			normalY = oy * (dy < 0.0 ? 1.0 : -1.0) / outside;
		}
		else if (qx > qy) {
			normalX = dx < 0.0 ? 1.0 : -1.0;
			normalY = 0.0;
		}
		else {
			normalX = 0.0;
			normalY = dy < 0.0 ? 1.0 : -1.0;
		}
		boundaryX = px - normalX * distance;
		boundaryY = py - normalY * distance;
	}

	/*
		鼠标边缘光。逐像素按距离场解析求值，不再沿边界撒圆点：
		  · 往里：离边界越远越暗，到 thickness 处平滑归零（平方衰减，外沿没有硬切边）；
		  · 沿边界：以离鼠标最近的边界点为 1，沿边界走开 range 之后归零。
		两个因子都是位置的连续函数，所以在斜边、圆角上都不会出现台阶，光带的粗细和亮度也是连续的。
	*/
	QImage LiquidGlassEffectPrivate::rimLightGlow(const QSize& size, int borderRadius, const QPoint& mousePosition,
		int thickness, int range, const QColor& color, QRect* dirtyRect) {
		if (dirtyRect) {
			*dirtyRect = QRect();
		}
		const int width = size.width();
		const int height = size.height();
		if (width <= 0 || height <= 0 || thickness <= 0 || range <= 0 || color.alpha() <= 0) {
			return QImage();
		}
		const qreal radius = std::clamp(qreal(borderRadius), 0.0, std::min(width, height) / 2.0);
		const qreal thicknessValue = qreal(thickness);
		const qreal rangeValue = qreal(range);
		// 鼠标的像素中心与它到边界的距离：鼠标在形状内部时距离为 0，此时最近点就是它脚下的边界
		const qreal mouseX = mousePosition.x() + 0.5;
		const qreal mouseY = mousePosition.y() + 0.5;
		qreal mouseDistance = 0.0;
		qreal ignoreNormalX = 0.0;
		qreal ignoreNormalY = 0.0;
		qreal ignoreBoundaryX = 0.0;
		qreal ignoreBoundaryY = 0.0;
		roundedRectField(size, radius, mouseX, mouseY, mouseDistance, ignoreNormalX, ignoreNormalY, ignoreBoundaryX, ignoreBoundaryY);
		const qreal nearestDistance = std::max(0.0, mouseDistance);
		// 沿着边界走不出 range、往里走不出 thickness，所以发光区域一定落在以鼠标为心的这个圆里
		const qreal reach = nearestDistance + rangeValue + thicknessValue + 1.0;
		const int left = std::max(0, int(std::floor(mouseX - reach)));
		const int right = std::min(width - 1, int(std::ceil(mouseX + reach)));
		const int top = std::max(0, int(std::floor(mouseY - reach)));
		const int bottom = std::min(height - 1, int(std::ceil(mouseY + reach)));
		if (right < left || bottom < top) {
			return QImage();
		}
		QImage image(QSize(right - left + 1, bottom - top + 1), QImage::Format_ARGB32);
		image.fill(0);
		const qreal baseAlpha = color.alphaF() * 255.0;
		const quint32 rgb = (quint32(color.red()) << 16) | (quint32(color.green()) << 8) | quint32(color.blue());
		quint32* imageBits = reinterpret_cast<quint32*>(image.bits());
		const qsizetype rowStep = image.width();
		int writtenLeft = right + 1;
		int writtenTop = bottom + 1;
		int writtenRight = left - 1;
		int writtenBottom = top - 1;
		// 每个像素在 sampleGrid×sampleGrid 个子位置上各算一次再平均。
		// 光带只有一个像素宽时，按像素中心单点采样会让它在圆弧上变成一个一个方块的阶梯；
		// 子采样拿到的是这一像素真正覆盖了多少光带，圆弧才会是平滑的一条。
		const int sampleGrid = 4;
		const qreal sampleStep = 1.0 / sampleGrid;
		for (int y = top; y <= bottom; y++) {
			quint32* line = imageBits + qsizetype(y - top) * rowStep;
			for (int x = left; x <= right; x++) {
				qreal coverage = 0.0;
				for (int subY = 0; subY < sampleGrid; subY++) {
					const qreal sampleY = y + (subY + 0.5) * sampleStep;
					for (int subX = 0; subX < sampleGrid; subX++) {
						const qreal sampleX = x + (subX + 0.5) * sampleStep;
						qreal distance = 0.0;
						qreal normalX = 0.0;
						qreal normalY = 0.0;
						qreal boundaryX = 0.0;
						qreal boundaryY = 0.0;
						roundedRectField(size, radius, sampleX, sampleY, distance, normalX, normalY, boundaryX, boundaryY);
						if (distance <= 0.0 || distance >= thicknessValue + 0.5) {
							continue;
						}
						const qreal along = 1.0 - (std::hypot(boundaryX - mouseX, boundaryY - mouseY) - nearestDistance) / rangeValue;
						if (along <= 0.0) {
							continue;
						}
						// 深度按「到边界的那半格像素」算起：贴边那一圈取满强度，再往里平方衰减，
						// 否则厚度取 1 时最外圈只有 25% 强度，一像素宽的环会碎成断断续续的虚线。
						const qreal depth = std::clamp(1.0 - (distance - 0.5) / thicknessValue, 0.0, 1.0);
						if (depth <= 0.0) {
							continue;
						}
						coverage += depth * depth * along * along;
					}
				}
				if (coverage <= 0.0) {
					continue;
				}
				coverage /= qreal(sampleGrid) * sampleGrid;
				const quint8 alpha = quint8(std::clamp(baseAlpha * coverage, 0.0, 255.0));
				if (alpha == 0) {
					continue;
				}
				line[x - left] = (quint32(alpha) << 24) | rgb;
				writtenLeft = std::min(writtenLeft, x);
				writtenRight = std::max(writtenRight, x);
				writtenTop = std::min(writtenTop, y);
				writtenBottom = std::max(writtenBottom, y);
			}
		}
		if (writtenRight < writtenLeft || writtenBottom < writtenTop) {
			return QImage();
		}
		const QRect written(writtenLeft, writtenTop, writtenRight - writtenLeft + 1, writtenBottom - writtenTop + 1);
		if (dirtyRect) {
			*dirtyRect = written;
		}
		return image.copy(QRect(written.topLeft() - QPoint(left, top), written.size()));
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
	void LiquidGlassEffectPrivate::modulateByLightMap(QImage& target, const QImage& lightMap, const QRect& area, bool brighten, qreal strength, bool additive, const QPoint& lightMapOrigin) {
		const QRect rect = area.intersected(QRect(QPoint(0, 0), target.size()));
		if (rect.isEmpty() || lightMap.isNull() || strength <= 0.0) {
			return;
		}
		if (strength > 1.0) {
			strength = 1.0;
		}
		// 边缘光是叠加在玻璃表面的高光，混合到 100% 就成了不透明的白线。
		// 这里给个硬上限：不管颜色 alpha 和激活强度给多大，实际混合最多只到 50%。
		if (additive && strength > maxRimLightBlend) {
			strength = maxRimLightBlend;
		}
		const int scale = int(strength * 256.0); // 强度也折成定点，鼠标边缘光的淡入淡出就靠它
		for (int y = rect.top(); y <= rect.bottom(); y++) {
			const int mapY = y - lightMapOrigin.y();
			if (mapY < 0 || mapY >= lightMap.height()) {
				continue;
			}
			const quint32* mapLine = reinterpret_cast<const quint32*>(lightMap.constScanLine(mapY));
			quint32* targetLine = reinterpret_cast<quint32*>(target.scanLine(y));
			for (int x = rect.left(); x <= rect.right(); x++) {
				const int mapX = x - lightMapOrigin.x();
				if (mapX < 0 || mapX >= lightMap.width()) {
					continue;
				}
				const quint32 mapPixel = mapLine[mapX];
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
					// 加半个分母再除：截断会让缓慢变化的渐变出现一条条色阶
					const int amountR = (lightR * alpha * scale + 255 * 128) / (255 * 256);
					const int amountG = (lightG * alpha * scale + 255 * 128) / (255 * 256);
					const int amountB = (lightB * alpha * scale + 255 * 128) / (255 * 256);
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
		全局打光的单侧生成：逐像素用圆角矩形的距离场求出内法线，再用「法线与光方向」的点积决定
		它属于迎光面还是背光面。polarity 为 +1 时只画亮面，为 -1 时只画阴影面，
		两侧的强度都取点积的平方：正对光源最亮、与光垂直处归零、另一面完全不参与。
		法线来自解析距离场（直边得到轴向法线、圆角得到指向圆角圆心的径向法线），并且是位置的连续函数，
		所以明暗过渡和边界都不会出现台阶；往里则按厚度做平方衰减，外沿平滑归零。
		一束光的两面由LiquidGlassEffect::globalRimImage()配对：本函数只生成其中一面，不负责调制。
	*/
	QImage LiquidGlassEffectPrivate::buildGlobalRimSide(const QSize& size, int borderRadius, const QPointF& lightDirection, int thickness, qreal polarity, const QColor& color, QRect* dirtyRect) {
		if (dirtyRect) {
			*dirtyRect = QRect();
		}
		const int width = size.width();
		const int height = size.height();
		if (width <= 0 || height <= 0 || thickness <= 0 || color.alpha() <= 0) {
			return QImage();
		}
		const qreal directionLength = std::hypot(lightDirection.x(), lightDirection.y());
		if (directionLength <= 0.0) {
			return QImage(); // 光方向为零向量，等于没有光源
		}
		const qreal directionX = lightDirection.x() / directionLength;
		const qreal directionY = lightDirection.y() / directionLength;
		const qreal radius = std::clamp(qreal(borderRadius), 0.0, std::min(width, height) / 2.0);
		const qreal thicknessValue = qreal(thickness);
		// 只在贴着边界的这一圈里扫：宽度取「厚度 + 圆角半径」，圆角外侧的斜向部分也包得住，
		// 图中间的大片区域连距离场都不用算。
		const int band = std::min(std::max(1, int(std::ceil(thicknessValue + radius))), std::max(width, height) / 2);
		const int sampleGrid = 4;
		const qreal sampleStep = 1.0 / sampleGrid;
		QImage image(size, QImage::Format_ARGB32);
		image.fill(0);
		const qreal baseAlpha = color.alphaF() * 255.0;
		const quint32 rgb = (quint32(color.red()) << 16) | (quint32(color.green()) << 8) | quint32(color.blue());
		quint32* imageBits = reinterpret_cast<quint32*>(image.bits());
		const qsizetype rowStep = qsizetype(width);
		int writtenLeft = width;
		int writtenTop = height;
		int writtenRight = -1;
		int writtenBottom = -1;
		auto paintPixel = [&](int x, int y) {
			// 每个像素在 sampleGrid×sampleGrid 个子位置上各算一次再平均，理由同边缘光：
			// 一像素宽的光带按中心单点采样会在圆弧上变成方块台阶，子采样得到的是真实覆盖量。
			qreal coverage = 0.0;
			for (int subY = 0; subY < sampleGrid; subY++) {
				const qreal sampleY = y + (subY + 0.5) * sampleStep;
				for (int subX = 0; subX < sampleGrid; subX++) {
					const qreal sampleX = x + (subX + 0.5) * sampleStep;
					qreal distance = 0.0;
					qreal normalX = 0.0;
					qreal normalY = 0.0;
					qreal boundaryX = 0.0;
					qreal boundaryY = 0.0;
					roundedRectField(size, radius, sampleX, sampleY, distance, normalX, normalY, boundaryX, boundaryY);
					if (distance <= 0.0 || distance >= thicknessValue + 0.5) {
						continue;
					}
					// roundedRectField 给的是指向内部的法线，全局光要的是朝外的那一面：
					// 光从控件指向光源，所以外法线与光方向点积为正的那条边就是迎光面。
					const qreal dot = -(normalX * directionX + normalY * directionY);
					const qreal facing = polarity > 0.0 ? dot : -dot;
					if (facing <= 0.0) {
						continue;
					}
					const qreal depth = std::clamp(1.0 - (distance - 0.5) / thicknessValue, 0.0, 1.0);
					if (depth <= 0.0) {
						continue;
					}
					coverage += depth * depth * facing * facing;
				}
			}
			if (coverage <= 0.0) {
				return;
			}
			coverage /= qreal(sampleGrid) * sampleGrid;
			const quint8 alpha = quint8(std::clamp(baseAlpha * coverage, 0.0, 255.0));
			if (alpha == 0) {
				return;
			}
			imageBits[qsizetype(y) * rowStep + x] = (quint32(alpha) << 24) | rgb;
			writtenLeft = std::min(writtenLeft, x);
			writtenRight = std::max(writtenRight, x);
			writtenTop = std::min(writtenTop, y);
			writtenBottom = std::max(writtenBottom, y);
		};
		for (int y = 0; y < height; y++) {
			const bool inVerticalBand = y < band || y >= height - band;
			if (inVerticalBand) {
				for (int x = 0; x < width; x++) {
					paintPixel(x, y);
				}
				continue;
			}
			for (int x = 0; x < std::min(band, width); x++) {
				paintPixel(x, y);
			}
			for (int x = std::max(width - band, 0); x < width; x++) {
				paintPixel(x, y);
			}
		}
		if (writtenRight < writtenLeft || writtenBottom < writtenTop) {
			return QImage();
		}
		const QRect written(writtenLeft, writtenTop, writtenRight - writtenLeft + 1, writtenBottom - writtenTop + 1);
		if (dirtyRect) {
			*dirtyRect = written;
		}
		return image.copy(written);
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
		int kernelSum = 0;
		for (int i = 0; i < radius; i++) {
			kernel[i] = qRound(weights[i] / sum * 65536.0);
			kernelSum += kernel[i];
		}
		// 让定点核的系数和精确等于 65536，避免小半径下反复卷积后逐渐变暗。
		kernel[radius / 2] += 65536 - kernelSum;
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
				targetRow[x] = (quint32(std::clamp((sums[3] + 32768) >> 16, 0, 255)) << 24)
					| (quint32(std::clamp((sums[0] + 32768) >> 16, 0, 255)) << 16)
					| (quint32(std::clamp((sums[1] + 32768) >> 16, 0, 255)) << 8)
					| quint32(std::clamp((sums[2] + 32768) >> 16, 0, 255));
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
		const QImage source = image.format() == QImage::Format_ARGB32
			? image : image.convertToFormat(QImage::Format_ARGB32);
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
		帧缓存的指纹：只要下面这些参数不变，"打光 + 模糊 + 扭曲"之后的那张图就完全一样。
		边缘光在这张图之后才叠加（它跟着鼠标走，不能进缓存），所以这里不含鼠标状态。
	*/
	quint64 LiquidGlassEffectPrivate::frameKey(const QSize& size) const {
		const QRect widgetGeometry = parentWidget ? parentWidget->geometry() : QRect();
		return quint64(qHashMulti(size_t(0),
			quint32(effectTypes),
			quint32(positionPolicy),
			size.width(),
			size.height(),
			widgetGeometry.x(),
			widgetGeometry.y(),
			customGeometry.x(),
			customGeometry.y(),
			customGeometry.width(),
			customGeometry.height(),
			liquidDistortRadius,
			borderRadius,
			liquidDistortDispersion,
			liquidDistortStrength,
			liquidDistortEnergy,
			blurRadius,
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

	void LiquidGlassEffectPrivate::clearBlurCache() {
		frameBlurredCache = QImage();
		frameScratch = QImage();
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
				renderWatchedSet.insert(wanted[i].data());
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
		renderWatchedSet.clear();
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
		return renderWatchedSet.contains(static_cast<QWidget*>(const_cast<QObject*>(object)));
	}
}