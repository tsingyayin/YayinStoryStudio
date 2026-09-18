#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>
#include <QtCore/qsize.h>
#include <QtGui/qcolor.h>
#include <QtGui/qcursor.h>
#include <QtGui/qevent.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qimage.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qregion.h>
#include <QtGui/qscreen.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qwidget.h>
#include <Widgets/LiquidGlassEffect.h>
#include "Test/LiquidGlassDragTestWidget.h"

using LGE = Visindigo::Widgets::LiquidGlassEffect;

namespace YSS::Test {
	namespace {
		// 一块玻璃面板要用到的全部参数。列在结构里是为了让下面那张参数表一眼能对齐，
		// 否则十来个参数平铺在初始化列表里很难看出每块面板差在哪。
		struct PanelSpec {
			const char* title;
			bool rimLight;
			bool globalRim;
			QPointF lightDirection;
			QColor rimColor;
			int rimThickness;
			int rimRange;
			int rimActivation;
			QColor lightColor;
			int lightThickness;
			QColor shadowColor;
			int shadowThickness;
			qreal dispersion;
			int blur;
		};

		// 可以按住左键拖着走的玻璃面板，用来试“玻璃压到别的控件上会怎样”。
		class DragPanel :public QWidget {
		public:
			using QWidget::QWidget;
		protected:
			void mousePressEvent(QMouseEvent* event) override {
				if (event->button() != Qt::LeftButton) {
					QWidget::mousePressEvent(event);
					return;
				}
				// 这里只记鼠标的全局坐标：子控件的 frameGeometry()/geometry() 是父坐标系下的值，
				// 拿它和全局坐标相减会得到一个混了窗口位置的错误偏移，一按下去控件就会跳。
				lastGlobalPosition = event->globalPosition().toPoint();
				raise();
				event->accept();
			}
			void mouseMoveEvent(QMouseEvent* event) override {
				if (!(event->buttons() & Qt::LeftButton) || !parentWidget()) {
					return;
				}
				// 用两帧之间鼠标的位移累加，这样既不受控件自身移动的影响，也不需要做坐标系换算
				const QPoint globalPosition = event->globalPosition().toPoint();
				const QPoint moved = pos() + (globalPosition - lastGlobalPosition);
				lastGlobalPosition = globalPosition;
				move(qBound(0, moved.x(), qMax(0, parentWidget()->width() - width())),
					qBound(0, moved.y(), qMax(0, parentWidget()->height() - height())));
				event->accept();
			}
		private:
			QPoint lastGlobalPosition;
		};

		// 按一份参数搭一块 CustomImage 策略的玻璃面板，并给它挂上名字标签
		void buildPanel(QWidget* parent, const QImage& background, const PanelSpec& spec, const QRect& geometry, int distortRadius) {
			QWidget* panel = new QWidget(parent);
			panel->setGeometry(geometry);
			LGE* effect = new LGE(panel);
			effect->setBackgroundPolicy(LGE::BackgroundPolicy::CustomImage);
			effect->setPositionPolicy(LGE::PositionPolicy::ParentLocalGeometry);
			effect->setBackgroundImage(background);
			effect->setBorderRadius(16);
			effect->setBlurRadius(spec.blur);
			effect->setLiquidDistortRadius(distortRadius);
			effect->setColorMask(QColor(0, 0, 0), 0.1);
			effect->setLiquidDistortDispersion(spec.dispersion);
			LGE::EffectTypes effectTypes(LGE::EffectType::NormalDistort);
			if (spec.dispersion > 0.0) {
				effectTypes |= LGE::EffectType::DispersionDistort;
			}
			effectTypes |= LGE::EffectType::Blur;
			if (spec.rimLight) {
				effectTypes |= LGE::EffectType::RimLight;
			}
			if (spec.globalRim) {
				effectTypes |= LGE::EffectType::GlobalRim;
			}
			effect->setEffectTypes(effectTypes);
			if (spec.rimLight) {
				effect->setRimLightColor(spec.rimColor);
				effect->setRimLightThickness(spec.rimThickness);
				effect->setRimLightRange(spec.rimRange);
				effect->setRimLightActivationDistance(spec.rimActivation);
			}
			if (spec.globalRim) {
				effect->setGlobalRimLightDirection(spec.lightDirection);
				effect->setGlobalRimLightColor(spec.lightColor);
				effect->setGlobalRimLightThickness(spec.lightThickness);
				effect->setGlobalRimShadowColor(spec.shadowColor);
				effect->setGlobalRimShadowThickness(spec.shadowThickness);
			}
			panel->setGraphicsEffect(effect);
			QLabel* nameLabel = new QLabel(spec.title, panel);
			nameLabel->setGeometry(0, 14, geometry.width(), 60);
			nameLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
			nameLabel->setWordWrap(true);
			nameLabel->setStyleSheet("color:#FFFFFF; font-size:13px; background:transparent;");
		}

		// 拖动的两块：一块圆角正方形、一块纯圆形（圆角半径取边长的一半就是正圆）
		void buildDragPanel(QWidget* parent, const QRect& geometry, int radius, const QString& title) {
			const int dragSize = geometry.width();
			DragPanel* panel = new DragPanel(parent);
			panel->setGeometry(geometry);
			if (radius * 2 >= dragSize) {
				// 正圆：连事件区域也裁成圆形，点圆外不会把它拖走
				panel->setMask(QRegion(QRect(0, 0, dragSize, dragSize), QRegion::Ellipse));
			}
			LGE* effect = new LGE(panel);
			// 用默认的Render策略：拖到别的控件上时能把它们（未经过效果处理的原始内容）也叠进玻璃里。
			// BackgroundPolicy::Render 的被模糊源每次都是现渲染父窗口取来的，压在下层的玻璃若也需要重新采样，
			// 那次采样会撞上“渲染父控件的中途再渲染父控件”，于是只看得到它未经效果的原始内容。
			effect->setBorderRadius(radius);
			panel->setGraphicsEffect(effect);
			QLabel* nameLabel = new QLabel(title, panel);
			nameLabel->setGeometry(0, 12, geometry.width(), 30);
			nameLabel->setAlignment(Qt::AlignCenter);
			nameLabel->setStyleSheet("color:#FFFFFF; font-size:13px; background:transparent;");
		}
	}

	LiquidGlassDragTestWidget::LiquidGlassDragTestWidget(QWidget* parent) :QWidget(parent) {
		const QSize winSize(1020, 710);
		const QSize panelSize(125, 150);
		const QSize sampleSize(820, 170);
		setWindowTitle("光效测试（边缘光 / 全局打光）");
		resize(winSize);
		// 居中到鼠标所在的屏幕：换显示器、改分辨率之后这个窗口也不会跑到屏幕外面去
		QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
		if (!screen) {
			screen = QGuiApplication::primaryScreen();
		}
		if (screen) {
			const QRect available = screen->availableGeometry();
			move(available.center() - QPoint(winSize.width() / 2, winSize.height() / 2));
		}
		// 这张底图既是窗口背景，也是下面各个玻璃面板的模糊源
		const QImage background = LGE::drawDebugBackground(winSize.width(), winSize.height());
		QLabel* backgroundLabel = new QLabel(this);
		backgroundLabel->setGeometry(0, 0, winSize.width(), winSize.height());
		backgroundLabel->setPixmap(QPixmap::fromImage(background));
		QLabel* titleLabel = new QLabel(this);
		titleLabel->setGeometry(0, 24, winSize.width(), 40);
		titleLabel->setAlignment(Qt::AlignCenter);
		titleLabel->setStyleSheet("color:#FFFFFF; font-size:19px; background:transparent;");
		titleLabel->setText("把鼠标移到下面的玻璃面板上，边缘光会贴着鼠标最近的那段边界");
		// 默认光方向：从左上角来、与竖直方向成30度
		const QPointF defaultLight(-0.5, -0.8660254);
		const PanelSpec specs[] = {
			{ "默认（细·柔）", true, true, defaultLight, QColor(255, 255, 255, 130), 1, 80, 40, QColor(255, 255, 255, 90), 1, QColor(0, 0, 0, 60), 2, 0.0, 16 },
			{ "对照：全关", false, false, defaultLight, QColor(255, 255, 255, 130), 1, 80, 40, QColor(255, 255, 255, 90), 1, QColor(0, 0, 0, 60), 2, 0.0, 16 },
			{ "柔光：厚 3", true, true, defaultLight, QColor(255, 255, 255, 130), 3, 80, 40, QColor(255, 255, 255, 90), 3, QColor(0, 0, 0, 60), 2, 0.0, 16 },
			{ "扭曲：普通", true, true, defaultLight, QColor(255, 255, 255, 130), 6, 80, 40, QColor(255, 255, 255, 90), 6, QColor(0, 0, 0, 60), 3, 0.0, 16 },
			{ "扭曲：色散 0.2", true, true, defaultLight, QColor(255, 255, 255, 130), 6, 80, 40, QColor(255, 255, 255, 90), 6, QColor(0, 0, 0, 60), 3, 0.2, 16 },
			{ "光从左（厚 2）", false, true, QPointF(-1, 0), QColor(255, 255, 255, 130), 2, 80, 40, QColor(255, 255, 255, 90), 2, QColor(0, 0, 0, 60), 2, 0.0, 16 },
		};
		// 要看的样例：打光细（亮面厚 1、阴影厚 2）+ 色散 0.5 + 模糊 15，扭曲半径放大到 24 让彩边更明显
		const PanelSpec sampleSpec = {
			"样例：打光细（亮 1 / 暗 2） + 色散 0.5 + 模糊 15，扭曲半径 24",
			true, true, defaultLight, QColor(255, 255, 255, 130), 1, 96, 40,
			QColor(255, 255, 255, 90), 1, QColor(0, 0, 0, 60), 2, 0.5, 15
		};
		// 全默认值面板：连被模糊源都走默认的Render策略（从父窗口取），一个setter都不调，
		// 所以它用的是默认的扭曲半径20、色散0.2、模糊13、圆角20，以及默认的光效强度
		QWidget* defaultPanel = new QWidget(this);
		defaultPanel->setGeometry(30, 115, panelSize.width(), panelSize.height());
		defaultPanel->setGraphicsEffect(new LGE(defaultPanel));
		QLabel* defaultLabel = new QLabel(defaultPanel);
		defaultLabel->setGeometry(0, 14, panelSize.width(), 60);
		defaultLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
		defaultLabel->setWordWrap(true);
		defaultLabel->setStyleSheet("color:#FFFFFF; font-size:13px; background:transparent;");
		defaultLabel->setText("全默认值（不调 setter）");
		for (int i = 0; i < int(sizeof(specs) / sizeof(specs[0])); i++) {
			buildPanel(this, background, specs[i], QRect(30 + (i + 1) * (panelSize.width() + 15), 115, panelSize.width(), panelSize.height()), 16);
		}
		buildPanel(this, background, sampleSpec, QRect((winSize.width() - sampleSize.width()) / 2, 300, sampleSize.width(), sampleSize.height()), 24);
		const int dragSize = 150;
		buildDragPanel(this, QRect(30, 490, dragSize, dragSize), 28, "拖我：圆角正方形");
		buildDragPanel(this, QRect(210, 490, dragSize, dragSize), dragSize / 2, "拖我：正圆");
		QLabel* hintLabel = new QLabel(this);
		hintLabel->setGeometry(0, 645, winSize.width(), 60);
		hintLabel->setAlignment(Qt::AlignCenter);
		hintLabel->setStyleSheet("color:#FFFFFF; font-size:13px; background:transparent;");
		hintLabel->setWordWrap(true);
		hintLabel->setText("第一块面板是全默认值（不调任何setter，被模糊源取自父窗口），后面是对照组和样例；左下两块可以按住鼠标拖来拖去。\n边缘光：从面板外靠近→淡入，贴边界滑动→跟着滑动，离开窗口→熄灭。全局光：光从哪边来就亮哪边，垂直两侧无光，对面是阴影。");
	}
}
