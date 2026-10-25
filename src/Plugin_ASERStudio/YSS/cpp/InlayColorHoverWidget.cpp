#include <QtGui/qfont.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcolordialog.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include <General/TranslationHost.h>
#include "YSS/InlayColorHoverWidget.h"
#include "YSS/InlayColorProvider.h"
namespace ASERStudio::YSS {
	InlayColorHoverWidget::InlayColorHoverWidget(QWidget* parent) :InlayHintHoverWidget(parent) {
		this->setObjectName(QStringLiteral("ASERInlayColorHoverWidget"));
		this->setMinimumWidth(240);
		auto* layout = new QVBoxLayout(this);
		layout->setContentsMargins(8, 8, 8, 8);
		layout->setSpacing(6);
		auto* title = new QLabel(VITRL("ASERStudio::inlayColor.title"), this);
		QFont titleFont = title->font();
		titleFont.setBold(true);
		title->setFont(titleFont);
		ValueLabel = new QLabel(this);
		ValueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
		PickButton = new QPushButton(VITRL("ASERStudio::inlayColor.pick"), this);
		PickButton->setMinimumHeight(32);
		PickButton->setCursor(Qt::PointingHandCursor);
		HintLabel = new QLabel(VITRL("ASERStudio::inlayColor.hint"), this);
		HintLabel->setWordWrap(true);
		QFont hintFont = HintLabel->font();
		hintFont.setPointSizeF(hintFont.pointSizeF() * 0.85);
		HintLabel->setFont(hintFont);
		layout->addWidget(title);
		layout->addWidget(ValueLabel);
		layout->addWidget(PickButton);
		layout->addWidget(HintLabel);
		connect(PickButton, &QPushButton::clicked, this, &InlayColorHoverWidget::openColorDialog);
	}

	InlayColorHoverWidget::~InlayColorHoverWidget() {
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
		CurrentColor = InlayColorProvider::colorFromString(value, &scale);
		Scale = scale;
		ValueLabel->setText(value.isEmpty() ? VITRL("ASERStudio::inlayColor.invalid") : value);
		updatePreview();
	}

	void InlayColorHoverWidget::openColorDialog() {
		QColor initial = CurrentColor.isValid() ? CurrentColor : QColor(Qt::white);
		QColorDialog dialog(initial, this);
		dialog.setWindowTitle(VITRL("ASERStudio::inlayColor.title"));
		dialog.setOption(QColorDialog::ShowAlphaChannel, HasAlpha);
		if (HasAlpha) {
			// 原生颜色对话框通常不提供透明度调节，带 Alpha 的颜色改用 Qt 自带实现。
			dialog.setOption(QColorDialog::DontUseNativeDialog, true);
		}
		if (dialog.exec() != QDialog::Accepted) {
			return;
		}
		const QColor selected = dialog.selectedColor();
		if (not selected.isValid()) {
			return;
		}
		QString newValue = buildNewValue(selected);
		if (newValue.isEmpty()) {
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

	void InlayColorHoverWidget::updatePreview() {
		if (not CurrentColor.isValid()) {
			PickButton->setStyleSheet(QString());
			return;
		}
		const QColor textColor = CurrentColor.lightnessF() > 0.55 ? QColor(0, 0, 0) : QColor(255, 255, 255);
		PickButton->setStyleSheet(QStringLiteral(
			"QPushButton { background-color: %1; color: %2; border: 1px solid rgba(0,0,0,100);"
			" border-radius: 4px; padding: 4px 10px; }"
			"QPushButton:hover { border-color: rgba(255,255,255,180); }")
			.arg(CurrentColor.name(QColor::HexRgb), textColor.name()));
	}
}
