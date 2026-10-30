#include <algorithm>
#include <cmath>
#include <functional>
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
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qgroupbox.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qslider.h>
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

		// 往四行的参数表里加一行：名称、滑块、当前值标签。
		QSlider* addSliderRow(QWidget* parent, QGridLayout* grid, int& sliderIndex, const QString& name,
			int minimum, int maximum, int value, const std::function<void(int)>& apply) {
			const int column = (sliderIndex / 4) * 3;
			const int row = sliderIndex % 4;
			sliderIndex++;
			QSlider* slider = new QSlider(Qt::Horizontal, parent);
			slider->setRange(minimum, maximum);
			slider->setValue(value);
			slider->setMaximumWidth(110);
			QLabel* valueLabel = new QLabel(QString::number(value), parent);
			valueLabel->setMinimumWidth(34);
			valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			grid->addWidget(new QLabel(name, parent), row, column);
			grid->addWidget(slider, row, column + 1);
			grid->addWidget(valueLabel, row, column + 2);
			QObject::connect(slider, &QSlider::valueChanged, parent, [valueLabel, apply](int current) {
				valueLabel->setText(QString::number(current));
				apply(current);
			});
			return slider;
		}

		// 深度是「贴边处取样到多深」，与带宽无关：强度 = 1 + 深度 / 半径
		void applyDistortDepth(LGE* live, QSlider* radiusSlider, QSlider* depthSlider) {
			const int radius = std::max(1, radiusSlider->value());
			live->setLiquidDistortStrength(1.0 + depthSlider->value() / qreal(radius));
		}

		void applyGlassEffectTypes(LGE* live, QCheckBox* blurBox, QCheckBox* distortBox,
			QCheckBox* dispersionBox, QCheckBox* rimBox, QCheckBox* globalRimBox) {
			LGE::EffectTypes types;
			if (blurBox->isChecked()) {
				types |= LGE::EffectType::Blur;
			}
			if (distortBox->isChecked()) {
				types |= LGE::EffectType::NormalDistort;
			}
			if (dispersionBox->isChecked()) {
				types |= LGE::EffectType::DispersionDistort;
			}
			if (rimBox->isChecked()) {
				types |= LGE::EffectType::RimLight;
			}
			if (globalRimBox->isChecked()) {
				types |= LGE::EffectType::GlobalRim;
			}
			live->setEffectTypes(types);
		}

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
		const QSize winSize(1020, 800);
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
		// 要看的样例：打光细（亮面厚 1、阴影厚 2）+ 色散 0.5 + 模糊 15，扭曲半径放大到 24 让彩边更明显
		const PanelSpec sampleSpec = {
			"样例：打光细（亮 1 / 暗 2） + 色散 0.5 + 模糊 15，扭曲半径 24",
			true, true, defaultLight, QColor(255, 255, 255, 72), 2, 96, 40,
			QColor(255, 255, 255, 90), 1, QColor(0, 0, 0, 60), 2, 0.5, 15
		};
		buildPanel(this, background, sampleSpec, QRect((winSize.width() - sampleSize.width()) / 2, 80, sampleSize.width(), sampleSize.height()), 24);
		const int dragSize = 150;
		buildDragPanel(this, QRect(30, 275, dragSize, dragSize), 28, "拖我：圆角正方形");
		buildDragPanel(this, QRect(210, 275, dragSize, dragSize), dragSize / 2, "拖我：正圆");
		QLabel* hintLabel = new QLabel(this);
		hintLabel->setGeometry(0, 440, winSize.width(), 50);
		hintLabel->setAlignment(Qt::AlignCenter);
		hintLabel->setStyleSheet("color:#FFFFFF; font-size:13px; background:transparent;");
		hintLabel->setWordWrap(true);
		hintLabel->setText("上面是样例面板，左边两块可以按住鼠标拖来拖去；下面那块由右侧滑块实时调节。\n边缘光：从面板外靠近→淡入，贴边界滑动→跟着滑动，离开窗口→熄灭。全局光：光从哪边来就亮哪边，垂直两侧无光，对面是阴影。");

		// ---- 可动态调节的面板：下面的滑块直接改参数，拖动时立刻重绘 ----
		QWidget* livePanel = new QWidget(this);
		livePanel->setGeometry(30, 505, 300, 250);
		QLabel* liveLabel = new QLabel("可调面板（右侧滑块实时生效）", livePanel);
		liveLabel->setGeometry(0, 10, 300, 24);
		liveLabel->setAlignment(Qt::AlignCenter);
		liveLabel->setStyleSheet("color:#FFFFFF; font-size:13px; background:transparent;");
		LGE* live = new LGE(livePanel);
		live->setBackgroundPolicy(LGE::BackgroundPolicy::Render);
		live->setBorderRadius(20);
		live->setBlurRadius(13);
		live->setLiquidDistortRadius(20);
		live->setLiquidDistortStrength(1.5);
		live->setLiquidDistortDispersion(0.2);
		live->setColorMask(QColor(0, 0, 0), 0.1);
		livePanel->setGraphicsEffect(live);

		QGroupBox* controls = new QGroupBox("可调面板参数", this);
		controls->setGeometry(345, 505, 655, 250);
		QGridLayout* grid = new QGridLayout(controls);
		grid->setContentsMargins(10, 20, 10, 8);
		grid->setHorizontalSpacing(8);
		grid->setVerticalSpacing(3);
		int sliderIndex = 0;
		QSlider* radiusSlider = addSliderRow(controls, grid, sliderIndex,
			"扭曲半径", 1, 60, 6, [live](int value) { live->setLiquidDistortRadius(value); });
		QSlider* depthSlider = addSliderRow(controls, grid, sliderIndex,
			"深度(px)", 0, 300, 60, [](int) {});
		QObject::connect(radiusSlider, &QSlider::valueChanged, controls, [live, radiusSlider, depthSlider](int) {
			applyDistortDepth(live, radiusSlider, depthSlider);
		});
		QObject::connect(depthSlider, &QSlider::valueChanged, controls, [live, radiusSlider, depthSlider](int) {
			applyDistortDepth(live, radiusSlider, depthSlider);
		});
		applyDistortDepth(live, radiusSlider, depthSlider);
		addSliderRow(controls, grid, sliderIndex, "色散x100", 0, 100, 20, [live](int value) { live->setLiquidDistortDispersion(value / 100.0); });
		addSliderRow(controls, grid, sliderIndex, "模糊半径", 0, 160, 13, [live](int value) { live->setBlurRadius(value); });
		addSliderRow(controls, grid, sliderIndex, "圆角", 0, 100, 20, [live](int value) { live->setBorderRadius(value); });
		addSliderRow(controls, grid, sliderIndex, "不透明度", 0, 100, 100, [live](int value) { live->setOpacity(value / 100.0); });
		addSliderRow(controls, grid, sliderIndex, "着色", 0, 100, 10, [live](int value) { live->setColorMask(QColor(0, 0, 0), value / 100.0); });
		addSliderRow(controls, grid, sliderIndex, "能量x100", 0, 100, 50, [live](int value) { live->setLiquidDistortEnergy(value / 100.0); });
		addSliderRow(controls, grid, sliderIndex, "光源角度", 0, 360, 330, [live](int value) {
			const qreal pi = 3.14159265358979323846;
			const qreal radians = value * pi / 180.0;
			live->setGlobalRimLightDirection(QPointF(std::sin(radians), -std::cos(radians)));
		});
		addSliderRow(controls, grid, sliderIndex, "亮面厚度", 0, 10, 1, [live](int value) { live->setGlobalRimLightThickness(value); });
		addSliderRow(controls, grid, sliderIndex, "暗面厚度", 0, 10, 2, [live](int value) { live->setGlobalRimShadowThickness(value); });
		addSliderRow(controls, grid, sliderIndex, "边缘光厚度", 0, 10, 1, [live](int value) { live->setRimLightThickness(value); });
		addSliderRow(controls, grid, sliderIndex, "边缘光范围", 0, 200, 80, [live](int value) { live->setRimLightRange(value); });

		QCheckBox* blurBox = new QCheckBox("模糊", controls);
		QCheckBox* distortBox = new QCheckBox("普通扭曲", controls);
		QCheckBox* dispersionBox = new QCheckBox("色散", controls);
		QCheckBox* rimBox = new QCheckBox("边缘光", controls);
		QCheckBox* globalRimBox = new QCheckBox("全局光", controls);
		QHBoxLayout* typeLayout = new QHBoxLayout();
		typeLayout->setSpacing(10);
		for (QCheckBox* box : { blurBox, distortBox, dispersionBox, rimBox, globalRimBox }) {
			box->setChecked(true);
			typeLayout->addWidget(box);
		}
		typeLayout->addStretch(1);
		grid->addLayout(typeLayout, 4, 0, 1, 9);
		for (QCheckBox* box : { blurBox, distortBox, dispersionBox, rimBox, globalRimBox }) {
			QObject::connect(box, &QCheckBox::toggled, controls,
				[live, blurBox, distortBox, dispersionBox, rimBox, globalRimBox](bool) {
					applyGlassEffectTypes(live, blurBox, distortBox, dispersionBox, rimBox, globalRimBox);
				});
		}
	}
}
