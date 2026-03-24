#include "Widgets/StyleSheetTemplate.h"
#include <QtCore/qstring.h>
#include <QtCore/qmap.h>
#include <QtCore/qregularexpression.h>
#include <QtWidgets/qwidget.h>
#include "Utility/JsonConfig.h"
#include "Utility/ColorTool.h"
#include "General/Log.h"
#include "Widgets/ThemeManager.h"
namespace Visindigo::Widgets {
	class StyleSheetTemplatePrivate {
		friend class StyleSheetTemplate;
	protected:
		QMap<QString, QMap<QString, QString>> NamespacedTemplates;
		QString TemplateName;
		QString TemplateID;
	};

	/*!
		\class Visindigo::Widgets::StyleSheetTemplate
		\inheaderfile Widgets/StyleSheetTemplate.h
		\brief 提供对vst（Visindigo Stylesheet Template）样式表模板的解析与管理功能。
		\ingroup VITheme
		\inmoudle Visindigo
		\since Visindigo 0.13.0

		Visindigo样式表模板（vst）是一种用于定义和管理Qt样式表的文本格式。它允许开发者创建可重用的样式片段，并通过参数化的方式实现样式的动态生成。
		vst文件通常包含多个命名空间和键值对，每个键对应一个样式表片段，支持参数替换以适应不同的主题需求。

		有关VST文件格式的说明，请参见Visindigo::General::ThemeManager类的文档，其中有详细的介绍和示例。

		由于VST作为以QSS为主的脚本文件，其基本上以文本形式存在，嵌入代码中毫无意义，因此ThemeManager
		没有提供任何传递此类实例的接口，相关API都是以字符串形式传递VST内容或路径。所以在日常使用时，
		几乎不需要直接使用此类，除非是需要手动解析或生成vst文件内容的场景。例如从ThemeManager导出
		合并后的样式表模板。
	*/

	/*!
		\since Visindigo 0.13.0
		构造函数
	*/
	StyleSheetTemplate::StyleSheetTemplate() {
		d = new StyleSheetTemplatePrivate;
	}

	/*!
		\since Visindigo 0.13.0
		析构函数
	*/
	StyleSheetTemplate::~StyleSheetTemplate() {
		delete d;
	}

	/*!
		\fn Visindigo::Widgets::StyleSheetTemplate::StyleSheetTemplate(StyleSheetTemplate&& other) noexcept
		\since Visindigo 0.13.0
		移动构造函数
	*/

	/*!
		\fn Visindigo::Widgets::StyleSheetTemplate::StyleSheetTemplate(const StyleSheetTemplate& other)
		\since Visindigo 0.13.0
		拷贝构造函数
	*/

	/*!
		\fn Visindigo::Widgets::StyleSheetTemplate::operator=(StyleSheetTemplate&& other) noexcept
		\since Visindigo 0.13.0
		移动赋值运算符
	*/

	/*!
		\fn Visindigo::Widgets::StyleSheetTemplate::operator=(const StyleSheetTemplate& other)
		\since Visindigo 0.13.0
		拷贝赋值运算符
	*/
	VIMoveable_Impl(StyleSheetTemplate);
	VICopyable_Impl(StyleSheetTemplate);

	/*!
		\since Visindigo 0.13.0
		解析样式表模板字符串。
	*/
	bool StyleSheetTemplate::parse(QString& templateStr) {
		QStringList lines = templateStr.replace("\r\n", "\n").split("\n");
		bool header = true;
		int lastIndex = 0;
		QString namespaceStr;
		QString keyStr;
		for (int i = 0; i < lines.size(); i++) {
			if (lines[i] == "------") {
				header = false;
				if (d->TemplateName == "") {
					vgErrorF << "TemplateName not specified in stylesheet template.";
					return false;
				}
				if (d->TemplateID == "") {
					vgErrorF << "TemplateID not specified in stylesheet template.";
					return false;
				}
			}
			if (header) {
				if (lines[i].startsWith("!TemplateName")) {
					QStringList para = lines[i].split(":");
					if (para.length() != 2) {
						continue;
					}
					else {
						d->TemplateName = para[1].trimmed();
					}
				}
				else if (lines[i].startsWith("!TemplateID")) {
					QStringList para = lines[i].split(":");
					if (para.length() != 2) {
						continue;
					}
					else {
						d->TemplateID = para[1].trimmed();
						namespaceStr = d->TemplateID; // Use TemplateID as default namespace, if not specified. for backward compatibility.
					}
				}
			}
			else {
				if (lines[i].startsWith("@")) {
					if (lastIndex != 0) {
						QString temp;
						for (int j = lastIndex + 1; j < i; j++) {
							temp += lines[j] + "\n";
						}
						temp.removeLast();
						d->NamespacedTemplates[namespaceStr][keyStr] = temp;
					}
					lastIndex = i;
					keyStr = lines[i].mid(1);
				}
				else if (lines[i].startsWith("!Namespace")) {
					QStringList para = lines[i].split(":");
					if (para.length() != 2) {
						continue;
					}
					else {
						namespaceStr = para[1].trimmed();
					}
				}
				else if (lines[i].trimmed().startsWith("//")) {
					continue; // comment line
				}
			}
		}
		if (lastIndex != 0 && keyStr != "") {
			QString temp;
			for (int j = lastIndex + 1; j < lines.size(); j++) {
				temp += lines[j] + "\n";
			}
			temp.removeLast();
			d->NamespacedTemplates[namespaceStr][keyStr] = temp;
		}
		if (d->NamespacedTemplates.isEmpty()) {
			vgErrorF << "No templates found in stylesheet template.";
			return false;
		}
		return true;
	}

	/*!
		\since Visindigo 0.13.0
		将样式表模板转换为字符串表示形式。
	*/
	QString StyleSheetTemplate::toString() {
		QString rtn;
		rtn += "!TemplateName: " + d->TemplateName + "\n";
		rtn += "!TemplateID: " + d->TemplateID + "\n";
		rtn += "------\n";
		for (QString namespaceStr : d->NamespacedTemplates.keys()) {
			rtn += "!Namespace: " + namespaceStr + "\n";
			for (QString key : d->NamespacedTemplates[namespaceStr].keys()) {
				rtn += "@" + key + "\n";
				rtn += d->NamespacedTemplates[namespaceStr][key] + "\n";
			}
			rtn += "\n";
		}
		return rtn;
	}

	/*!
		\since Visindigo 0.13.0
		根据键（可能包含命名空间前缀）获取原始样式表字符串。
	*/
	QString StyleSheetTemplate::getRawStyleSheet(const QString& keyWithNamespace) {
		QStringList parts = keyWithNamespace.split("::");
		if (parts.size() == 2) {
			return d->NamespacedTemplates[parts[0]][parts[1]];
		}
		else {
			return d->NamespacedTemplates[d->TemplateID][keyWithNamespace];
		}
	}

	/*!
		\since Visindigo 0.13.0
		根据命名空间和键获取原始样式表字符串。
	*/
	QString StyleSheetTemplate::getRawStyleSheet(const QString& namespaceStr, const QString& key) {
		return d->NamespacedTemplates[namespaceStr][key];
	}

	/*!
		\since Visindigo 0.13.0
		根据键（可能包含命名空间前缀）和可选的QWidget获取最终样式表字符串。

		值得注意的是，此方法自动从ThemeManager获取颜色参数，不能手动介入。
	*/
	QString StyleSheetTemplate::getStyleSheet(const QString& keyWithNamespace, QWidget* getter) {
		QString temp = getRawStyleSheet(keyWithNamespace);
		QStringList paras;
		QRegularExpression reg("\\$\\([\\D\\d]+?\\)");
		QRegularExpressionMatchIterator matchs = reg.globalMatch(temp);
		for (auto match : matchs) {
			paras.append(match.captured());
		}
		for (QString para : paras) {
			temp.replace(para, Visindigo::Utility::ColorTool::toColorString(VISTM->getColor(para.mid(2, para.length() - 3))));
		}
		if (getter != nullptr) {
			QStringList paras;
			QRegularExpression reg("\\$\\{[\\D\\d]+?\\}");
			QRegularExpressionMatchIterator matchs = reg.globalMatch(temp);
			for (auto match : matchs) {
				paras.append(match.captured());
			}
			for (QString para : paras) {
				temp.replace(para, getter->property(para.mid(2, para.length() - 3).toUtf8().constData()).toString());
			}
		}
		return temp;
	}

	/*!
		\since Visindigo 0.13.0
		获取所有命名空间的列表。
	*/
	QStringList StyleSheetTemplate::getNamespaces() const {
		return d->NamespacedTemplates.keys();
	}

	/*!
		\since Visindigo 0.13.0
		设置指定命名空间和键的样式表模板字符串。
	*/
	void StyleSheetTemplate::setStyleSheetTemplate(const QString& namespaceStr, const QString& key, const QString& style) {
		d->NamespacedTemplates[namespaceStr][key] = style;
	}

	/*!
		\since Visindigo 0.13.0
		设置指定键（可能包含命名空间前缀）的样式表模板字符串。
	*/
	void StyleSheetTemplate::setStyleSheetTemplate(const QString& keyWithNamespace, const QString& style) {
		QStringList parts = keyWithNamespace.split("::");
		if (parts.size() == 2) {
			d->NamespacedTemplates[parts[0]][parts[1]] = style;
		}
		else {
			d->NamespacedTemplates[d->TemplateID][keyWithNamespace] = style;
		}
	}

	/*!
		\since Visindigo 0.13.0
		获取样式表模板的ID。
	*/
	QString StyleSheetTemplate::getTemplateID() const {
		return d->TemplateID;
	}

	/*!
		\since Visindigo 0.13.0
		获取样式表模板的名称。
	*/
	QString StyleSheetTemplate::getTemplateName() const {
		return d->TemplateName;
	}

	/*!
		\since Visindigo 0.13.0
		设置样式表模板的ID。
	*/
	void StyleSheetTemplate::setTemplateID(const QString& id) {
		d->TemplateID = id;
	}

	/*!
		\since Visindigo 0.13.0
		设置样式表模板的名称。
	*/
	void StyleSheetTemplate::setTemplateName(const QString& name) {
		d->TemplateName = name;
	}

	/*!
		\since Visindigo 0.13.0
		将另一个样式表模板的内容合并到当前模板中。
		
		它会逐一检查所有的键并进行覆盖。
	*/
	void StyleSheetTemplate::merge(const StyleSheetTemplate& other) {
		for (QString namespaceStr : other.d->NamespacedTemplates.keys()) {
			for (QString key : other.d->NamespacedTemplates[namespaceStr].keys()) {
				d->NamespacedTemplates[namespaceStr][key] = other.d->NamespacedTemplates[namespaceStr][key];
			}
		}
	}
}