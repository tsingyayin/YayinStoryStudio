#include <QtCore/qstringlist.h>
#include <QtGui/qfont.h>
#include <QtGui/qpainter.h>
#include <QtGui/qpainterpath.h>
#include <QtGui/qpen.h>
#include "YSS/InlayColorHoverWidget.h"
#include "YSS/InlayColorProvider.h"
namespace ASERStudio::YSS {
	/*!
		\class ASERStudio::YSS::InlayColorProvider
		\brief 为ASER剧本中的颜色提供色块内联提示和交互式颜色选择。
		\since ASERStudio 2.2
		\inmodule ASERStudio

		当前支持两类颜色：
		\list
		\li 富文本标签 \c{<color=#RRGGBB>} 中的十六进制颜色；
		\li \c{ColorMask} 控制器必需的 \c{Color} 参数，其值为 \c{r,g,b,a} 向量。
		\endlist

		色块由 drawInlayHint 绘制，内容占绘制区域的60%，四周留出透明空隙；
		富文本标签的色块位于颜色值之后、标签结束符 \c{>} 之前。
		用户点击色块后由 InlayColorHoverWidget 打开颜色选择器，
		确认后通过 InlayHintHoverWidget::requestModify 替换原文中的颜色值。
	*/
	InlayColorProvider::InlayColorProvider(const QString& name, const QString& id, YSSCore::Editor::EditorPlugin* plugin) :
		YSSCore::Editor::InlayHintProvider(name, id, plugin) {
	}

	InlayColorProvider::~InlayColorProvider() {
	}

	QColor InlayColorProvider::colorFromString(const QString& value, double* scale) {
		const QString text = value.trimmed();
		if (text.isEmpty()) {
			return QColor();
		}
		if (text.startsWith(QLatin1Char('#'))) {
			if (scale != nullptr) {
				*scale = 255.0;
			}
			return QColor(text);
		}
		const QStringList parts = text.split(QLatin1Char(','));
		if (parts.size() < 3 or parts.size() > 4) {
			return QColor();
		}
		QList<double> values;
		for (const QString& part : parts) {
			bool ok = false;
			const double number = part.trimmed().toDouble(&ok);
			if (not ok) {
				return QColor();
			}
			values.append(number);
		}
		double componentScale = 1.0;
		for (qint32 i = 0; i < qMin<qint32>(3, values.size()); ++i) {
			if (qAbs(values[i]) > 1.000001) {
				componentScale = 255.0;
				break;
			}
		}
		if (values.size() >= 4 and qAbs(values[3]) > 1.000001) {
			componentScale = 255.0;
		}
		if (scale != nullptr) {
			*scale = componentScale;
		}
		auto toByte = [componentScale](double component) {
			return qBound(0, qRound(component * 255.0 / componentScale), 255);
			};
		const qint32 alpha = values.size() >= 4 ? toByte(values[3]) : 255;
		return QColor(toByte(values[0]), toByte(values[1]), toByte(values[2]), alpha);
	}

	QString InlayColorProvider::colorToHex(const QColor& color, bool withAlpha) {
		return withAlpha ? color.name(QColor::HexArgb) : color.name(QColor::HexRgb);
	}

	QString InlayColorProvider::colorToVector(const QColor& color, double scale, int components) {
		auto formatComponent = [scale](double component) {
			const double scaled = component * scale;
			if (scale > 1.5) {
				return QString::number(qRound(scaled));
			}
			QString text = QString::number(scaled, 'f', 3);
			while (text.contains(QLatin1Char('.')) and text.endsWith(QLatin1Char('0'))) {
				text.chop(1);
			}
			if (text.endsWith(QLatin1Char('.'))) {
				text.chop(1);
			}
			return text;
			};
		QStringList parts;
		parts << formatComponent(color.redF());
		parts << formatComponent(color.greenF());
		parts << formatComponent(color.blueF());
		if (components >= 4) {
			parts << formatComponent(color.alphaF());
		}
		return parts.join(QLatin1Char(','));
	}

	QImage& InlayColorProvider::drawInlayHint(qint32 w, qint32 h, const YSSCore::General::YSSURI& file,
		const QString& name, Visindigo::Utility::JsonConfig& para) {
		Q_UNUSED(file);
		w = qMax(1, w);
		h = qMax(1, h);
		double scale = -1.0;
		bool scaleOk = false;
		const double paraScale = para.getDouble(InlayColorKeys::Scale, &scaleOk);
		if (scaleOk) {
			scale = paraScale;
		}
		const QColor color = colorFromString(name, &scale);
		const QString cacheKey = QStringLiteral("%1x%2:%3:%4").arg(w).arg(h).arg(name).arg(scale);
		auto cached = ImageCache.find(cacheKey);
		if (cached != ImageCache.end()) {
			return cached.value();
		}
		if (ImageCache.size() >= 128) {
			ImageCache.clear();
		}
		QImage image(w, h, QImage::Format_ARGB32_Premultiplied);
		image.fill(Qt::transparent);
		QPainter painter(&image);
		painter.setRenderHint(QPainter::Antialiasing, true);
		// 内容只占图片的60%，四周留出透明空隙，避免色块紧贴文字。
		const qreal contentWidth = w * 0.6;
		const qreal contentHeight = h * 0.6;
		const QRectF swatchRect((w - contentWidth) / 2.0, (h - contentHeight) / 2.0,
			contentWidth, contentHeight);
		if (swatchRect.width() < 1.0 or swatchRect.height() < 1.0) {
			painter.end();
			auto inserted = ImageCache.insert(cacheKey, image);
			return inserted.value();
		}
		if (not color.isValid()) {
			painter.setPen(Qt::NoPen);
			painter.setBrush(QColor(128, 128, 128, 180));
			painter.drawRoundedRect(swatchRect, 3, 3);
			painter.setPen(Qt::white);
			QFont font = painter.font();
			font.setPixelSize(qMax(6, qRound(h * 0.55)));
			font.setBold(true);
			painter.setFont(font);
			painter.drawText(swatchRect, Qt::AlignCenter, QStringLiteral("?"));
			painter.end();
			auto inserted = ImageCache.insert(cacheKey, image);
			return inserted.value();
		}
		QPainterPath clipPath;
		clipPath.addRoundedRect(swatchRect, 3, 3);
		if (color.alpha() < 255) {
			// 棋盘格只能画在色块内部，否则圆角外侧会留下一圈不透明像素。
			painter.save();
			painter.setClipPath(clipPath);
			const qint32 cell = qMax(3, qMin(w, h) / 4);
			for (qint32 y = 0; y < h; y += cell) {
				for (qint32 x = 0; x < w; x += cell) {
					const QColor checker = ((x / cell + y / cell) % 2 == 0)
						? QColor(235, 235, 235) : QColor(190, 190, 190);
					painter.fillRect(QRect(x, y, cell, cell), checker);
				}
			}
			painter.restore();
		}
		painter.save();
		painter.setClipPath(clipPath);
		painter.setPen(Qt::NoPen);
		painter.setBrush(color);
		painter.drawRoundedRect(swatchRect, 3, 3);
		painter.restore();
		painter.setPen(QPen(QColor(0, 0, 0, 160), 1));
		painter.setBrush(Qt::NoBrush);
		painter.drawRoundedRect(swatchRect, 3, 3);
		// 深色色块加一圈浅色内描边，避免在深色主题里糊成一团。
		if (color.lightnessF() < 0.25) {
			QRectF inner = swatchRect.adjusted(1.0, 1.0, -1.0, -1.0);
			if (inner.width() > 0.0 and inner.height() > 0.0) {
				painter.setPen(QPen(QColor(255, 255, 255, 90), 1));
				painter.setBrush(Qt::NoBrush);
				painter.drawRoundedRect(inner, 2, 2);
			}
		}
		painter.end();
		auto inserted = ImageCache.insert(cacheKey, image);
		return inserted.value();
	}

	YSSCore::Editor::InlayHintHoverWidget* InlayColorProvider::createHoverWidget(const YSSCore::General::YSSURI& targetFile) {
		Q_UNUSED(targetFile);
		return new InlayColorHoverWidget();
	}
}
