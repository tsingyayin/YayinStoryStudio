#include "../StyleSheetTemplate.h"
#include <QtCore/qstring.h>
#include <QtCore/qmap.h>
#include <QtCore/qregularexpression.h>
#include <QtWidgets/qwidget.h>
#include "../../Utility/JsonConfig.h"
#include "../../General/Log.h"
namespace Visindigo::Widgets {
	class StyleSheetTemplatePrivate {
		friend class StyleSheetTemplate;
	protected:
		QMap<QString, QMap<QString, QString>> NamespacedTemplates;
		QString TemplateName;
		QString TemplateID;
	};

	StyleSheetTemplate::StyleSheetTemplate() {
		d = new StyleSheetTemplatePrivate;
	}

	StyleSheetTemplate::~StyleSheetTemplate() {
		delete d;
	}

	VIMoveable_Impl(StyleSheetTemplate);
	VICopyable_Impl(StyleSheetTemplate);

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

	QString StyleSheetTemplate::toString() {
		QString rtn;
		rtn += "!TemplateName: " + d->TemplateName + "\n";
		rtn += "------\n";
		for (QString namespaceStr : d->NamespacedTemplates.keys()) {
			for (QString key : d->NamespacedTemplates[namespaceStr].keys()) {
				rtn += "!Namespace: " + namespaceStr + "\n";
				rtn += "@" + key + "\n";
				rtn += d->NamespacedTemplates[namespaceStr][key] + "\n";
			}
		}
		return rtn;
	}

	QString StyleSheetTemplate::getRawStyleSheet(const QString& keyWithNamespace) {
		QStringList parts = keyWithNamespace.split("::");
		if (parts.size() == 2) {
			return d->NamespacedTemplates[parts[0]][parts[1]];
		}
		else {
			return d->NamespacedTemplates[d->TemplateID][keyWithNamespace];
		}
	}

	QString StyleSheetTemplate::getRawStyleSheet(const QString& namespaceStr, const QString& key) {
		return d->NamespacedTemplates[namespaceStr][key];
	}

	QString StyleSheetTemplate::getStyleSheet(const QString& keyWithNamespace, const QString& themeID, Visindigo::Utility::JsonConfig* config, QWidget* getter) {
		if (config == nullptr && getter == nullptr) {
			return getRawStyleSheet(keyWithNamespace);
		}
		else {
			QString temp = getRawStyleSheet(keyWithNamespace);
			if (config != nullptr) {
				QStringList paras;
				QRegularExpression reg("\\$\\([\\D\\d]+?\\)");
				QRegularExpressionMatchIterator matchs = reg.globalMatch(temp);
				for (auto match : matchs) {
					paras.append(match.captured());
				}
				for (QString para : paras) {
					temp.replace(para, config->getString("Themes." + themeID + "." + para.mid(2, para.length() - 3)));
				}
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
	}

	QStringList StyleSheetTemplate::getNamespaces() const {
		return d->NamespacedTemplates.keys();
	}

	void StyleSheetTemplate::setStyleSheetTemplate(const QString& namespaceStr, const QString& key, const QString& style) {
		d->NamespacedTemplates[namespaceStr][key] = style;
	}

	void StyleSheetTemplate::setStyleSheetTemplate(const QString& keyWithNamespace, const QString& style) {
		QStringList parts = keyWithNamespace.split("::");
		if (parts.size() == 2) {
			d->NamespacedTemplates[parts[0]][parts[1]] = style;
		}
		else {
			d->NamespacedTemplates[d->TemplateID][keyWithNamespace] = style;
		}
	}

	QString StyleSheetTemplate::getTemplateID() const {
		return d->TemplateID;
	}

	QString StyleSheetTemplate::getTemplateName() const {
		return d->TemplateName;
	}

	void StyleSheetTemplate::setTemplateID(const QString& id) {
		d->TemplateID = id;
	}

	void StyleSheetTemplate::setTemplateName(const QString& name) {
		d->TemplateName = name;
	}

	void StyleSheetTemplate::merge(const StyleSheetTemplate& other) {
		for (QString namespaceStr : other.d->NamespacedTemplates.keys()) {
			for (QString key : other.d->NamespacedTemplates[namespaceStr].keys()) {
				d->NamespacedTemplates[namespaceStr][key] = other.d->NamespacedTemplates[namespaceStr][key];
			}
		}
	}
}