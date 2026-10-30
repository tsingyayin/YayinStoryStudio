#include <QtGui/qevent.h>
#include <QtGui/qfont.h>
#include <QtGui/qfontdatabase.h>
#include <QtGui/qpainter.h>
#include <QtGui/qpainterpath.h>
#include <QtGui/qpalette.h>
#include <QtGui/qpen.h>
#include <QtGui/qbrush.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include <General/TranslationHost.h>
#include <Utility/ColorTool.h>
#include <Widgets/LiquidGlassEffect.h>
#include <Widgets/ModernColorDialog.h>
#include "YSS/InlayColorHoverWidget.h"
#include "YSS/InlayColorProvider.h"
namespace ASERStudio::YSS {
	constexpr qreal SurfaceRadius = 6.0;
	// 浮窗自身表面的不透明度，剩下的部分留给下面的液态玻璃
	constexpr qreal SurfaceOpacity = 0.62;

	/*!
		\class ASERStudio::YSS::InlayColorHoverWidget
		\brief ASER 颜色内联提示的悬停浮窗。
		\since ASERStudio 2.5.0
		\inmodule ASERStudio

		浮窗内嵌 Visindigo::Widgets::ModernColorPicker，用户可以直接在浮窗里选色，按下“应用”后替换原文中的颜色值，
		按 Esc 关闭浮窗。

		颜色值同时兼容富文本标签的 \c{#RRGGBB} / \c{#AARRGGBB} 与 \c{ColorMask} 的
		\c{r,g,b[,a]} 向量，具体写法与量程沿用内联提示给出的参数。
	*/
	InlayColorHoverWidget::InlayColorHoverWidget(QWidget* parent) :InlayHintHoverWidget(parent) {
		// 自身的底由 paintEvent 画，QFrame 自带的边框会和玻璃圆角打架。
		this->setFrameShape(QFrame::NoFrame);
		buildUi();
	}

	InlayColorHoverWidget::~InlayColorHoverWidget() {
	}

	void InlayColorHoverWidget::paintEvent(QPaintEvent* event) {
		Q_UNUSED(event);
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing, true);
		const QRectF box = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
		// 玻璃底下是被模糊的编辑器，如果这里不画表面，背景一平就完全看不出有一层玻璃。
		// 表面取主题的卡片色并压低不透明度：既能透出模糊，又能让浮窗的轮廓立起来。
		QColor surface = Visindigo::Utility::ColorTool::paletteColor(palette(),
			Visindigo::Utility::ColorTool::PaletteColor::Card);
		surface.setAlphaF(SurfaceOpacity);
		const QColor border = Visindigo::Utility::ColorTool::paletteColor(palette(),
			Visindigo::Utility::ColorTool::PaletteColor::Border);
		painter.setPen(QPen(border, 1.0));
		painter.setBrush(surface);
		painter.drawRoundedRect(box, SurfaceRadius, SurfaceRadius);
	}

	void InlayColorHoverWidget::buildUi() {
		this->setObjectName(QStringLiteral("ASERInlayColorHoverWidget"));
		this->setMinimumWidth(400);
		auto* layout = new QVBoxLayout(this);
		layout->setContentsMargins(10, 10, 10, 10);
		layout->setSpacing(8);
		auto* headerLayout = new QHBoxLayout();
		headerLayout->setContentsMargins(0, 0, 0, 0);
		headerLayout->setSpacing(8);
		auto* title = new QLabel(VITRL("ASERStudio::inlayColor.title"), this);
		QFont titleFont = title->font();
		titleFont.setBold(true);
		title->setFont(titleFont);
		ValueLabel = new QLabel(this);
		ValueLabel->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
		ValueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
		ValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		headerLayout->addWidget(title);
		headerLayout->addWidget(ValueLabel, 1);
		layout->addLayout(headerLayout);
		Picker = new Visindigo::Widgets::ModernColorPicker(this);
		layout->addWidget(Picker);
		auto* footerLayout = new QHBoxLayout();
		footerLayout->setContentsMargins(0, 0, 0, 0);
		footerLayout->setSpacing(8);
		HintLabel = new QLabel(VITRL("ASERStudio::inlayColor.hint"), this);
		HintLabel->setWordWrap(true);
		QFont hintFont = HintLabel->font();
		hintFont.setPointSizeF(hintFont.pointSizeF() * 0.9);
		HintLabel->setFont(hintFont);
		QPalette hintPalette = HintLabel->palette();
		QColor hintColor = hintPalette.color(QPalette::WindowText);
		hintColor.setAlpha(170);
		hintPalette.setColor(QPalette::WindowText, hintColor);
		HintLabel->setPalette(hintPalette);
		ApplyButton = new QPushButton(VITRL("ASERStudio::inlayColor.apply"), this);
		ApplyButton->setCursor(Qt::PointingHandCursor);
		ApplyButton->setMinimumHeight(26);
		footerLayout->addWidget(HintLabel, 1);
		footerLayout->addWidget(ApplyButton);
		layout->addLayout(footerLayout);
		updateApplyButtonStyle();
		connect(ApplyButton, &QPushButton::clicked, this, &InlayColorHoverWidget::onApplyClicked);

		// 与 TextEdit 里其他悬停浮窗同款的液态玻璃配置。
		auto* glass = new Visindigo::Widgets::LiquidGlassEffect(this);
		glass->setBorderRadius(6);
		glass->setBlurRadius(20);
		glass->setLiquidDistortRadius(5);
		glass->setLiquidDistortStrength(10);
		this->setGraphicsEffect(glass);
		Visindigo::Widgets::LiquidGlassEffect::applyForegroundAlpha(this);
	}

	void InlayColorHoverWidget::updateApplyButtonStyle() {
		const QColor accent = palette().color(QPalette::Highlight);
		const QColor accentText = palette().color(QPalette::HighlightedText);
		ApplyButton->setStyleSheet(QStringLiteral(
			"QPushButton { background-color: %1; color: %2; border: 1px solid %3;"
			" border-radius: 4px; padding: 4px 16px; }"
			"QPushButton:hover { background-color: %4; }"
			"QPushButton:pressed { background-color: %5; }")
			.arg(accent.name(), accentText.name(), accent.darker(130).name(),
				accent.lighter(110).name(), accent.darker(115).name()));
	}

	void InlayColorHoverWidget::onHoverRequest() {
		const Visindigo::Utility::JsonConfig para = getPara();
		bool ok = false;
		Start = para.getInt(InlayColorKeys::Start, &ok);
		if (not ok) {
			Start = 0;
		}
		Length = para.getInt(InlayColorKeys::Length, &ok);
		if (not ok) {
			Length = 0;
		}
		Kind = para.getString(InlayColorKeys::Kind);
		Scale = para.getDouble(InlayColorKeys::Scale, &ok);
		if (not ok or Scale <= 0.0) {
			Scale = 1.0;
		}
		Components = para.getInt(InlayColorKeys::Components, &ok);
		QString value = para.getString(InlayColorKeys::Value);
		if (value.isEmpty() and Start >= 0 and Length > 0) {
			value = getRawText().mid(Start, Length);
		}
		if (Kind.isEmpty()) {
			Kind = value.startsWith(QLatin1Char('#')) ? InlayColorKeys::KindHex : InlayColorKeys::KindVector;
		}
		if (para.contains(InlayColorKeys::Alpha)) {
			HasAlpha = para.getBool(InlayColorKeys::Alpha);
		}
		else {
			HasAlpha = value.startsWith(QLatin1Char('#'))
				? value.length() == 9 : value.count(QLatin1Char(',')) >= 3;
		}
		if (not ok or Components < 3 or Components > 4) {
			Components = HasAlpha ? 4 : 3;
		}
		double scale = Scale;
		const QColor color = InlayColorProvider::colorFromString(value, &scale);
		Scale = scale;
		OriginalValue = value;
		ValueLabel->setText(value.isEmpty() ? VITRL("ASERStudio::inlayColor.invalid") : value);
		const bool alphaEditable = (Kind == InlayColorKeys::KindVector)
			? Components >= 4 : HasAlpha;
		Picker->setAlphaEnabled(alphaEditable);
		Picker->setColor(color.isValid() ? color : QColor(255, 255, 255));
		HintLabel->setText(color.isValid() ? VITRL("ASERStudio::inlayColor.hint")
			: VITRL("ASERStudio::inlayColor.invalid"));
	}

	void InlayColorHoverWidget::onApplyClicked() {
		QString newValue = buildNewValue(Picker->getColor());
		if (newValue.isEmpty() or newValue == OriginalValue) {
			return;
		}
		emit requestModify(Start, Length, newValue);
	}

	QString InlayColorHoverWidget::buildNewValue(const QColor& color) const {
		if (Kind == InlayColorKeys::KindVector) {
			return InlayColorProvider::colorToVector(color, Scale, Components);
		}
		return InlayColorProvider::colorToHex(color, HasAlpha);
	}

	void InlayColorHoverWidget::keyPressEvent(QKeyEvent* event) {
		// Esc 关闭浮窗：由 TextEdit 收起浮窗并把焦点交还编辑器。
		if (event->key() == Qt::Key_Escape) {
			event->accept();
			emit requestClose();
			return;
		}
		InlayHintHoverWidget::keyPressEvent(event);
	}
}
