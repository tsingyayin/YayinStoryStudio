#include "../StyleSheetTemplate.h"
#include <QtCore/qstring.h>
#include <QtCore/qmap.h>
#include <QtCore/qregularexpression.h>
#include <QtWidgets/qwidget.h>
#include "../../Utility/JsonConfig.h"
namespace YSSCore::Widgets {
	class StyleSheetTemplatePrivate {
		friend class StyleSheetTemplate;
	protected:
		QString TemplateName;
		QString TemplateID;
		QMap<QString, QString> Templates;
	};
	StyleSheetTemplate::StyleSheetTemplate() {
		d = new StyleSheetTemplatePrivate;
	}
	StyleSheetTemplate::~StyleSheetTemplate() {
		delete d;
	}
	StyleSheetTemplate::StyleSheetTemplate(const StyleSheetTemplate& other) {
		d = new StyleSheetTemplatePrivate;
		d->TemplateName = other.d->TemplateName;
		d->Templates = other.d->Templates;
	}
	StyleSheetTemplate::StyleSheetTemplate(StyleSheetTemplate&& other) noexcept {
		d = other.d;
		other.d = nullptr;
	}
	StyleSheetTemplate& StyleSheetTemplate::operator=(const StyleSheetTemplate& other) {
		if (&other != this) {
			d->TemplateName = other.d->TemplateName;
			d->Templates = other.d->Templates;
		}
		return *this;
	}
	StyleSheetTemplate& StyleSheetTemplate::operator=(StyleSheetTemplate&& other) {
		if (&other != this) {
			if (d != nullptr) {
				delete d;
			}
			d = other.d;
			other.d = nullptr;
		}
		return *this;
	}
	void StyleSheetTemplate::parse(QString& templateStr) {
		QStringList lines = templateStr.replace("\r\n", "\n").split("\n");
		bool header = true;
		int lastIndex = 0;
		QString tempName;
		for (int i = 0; i < lines.size(); i++) {
			if (lines[i] == "------") {
				header = false;
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
			}
			else {
				if (lines[i].startsWith("@")) {
					if (lastIndex != 0) {
						QString temp;
						for (int j = lastIndex+1; j < i; j++) {
							temp += lines[j] + "\n";
						}
						temp.removeLast();
						d->Templates[tempName] = temp;
					}
					lastIndex = i;
					tempName = lines[i].mid(1);
				}
			}
		}
		if (lastIndex != 0 && tempName != "") {
			QString temp;
			for (int j = lastIndex + 1; j < lines.size(); j++) {
				temp += lines[j] + "\n";
			}
			temp.removeLast();
			d->Templates[tempName] = temp;
		}
	}
	QString StyleSheetTemplate::toString() {
		QString rtn;
		rtn += "!TemplateName: " + d->TemplateName+"\n";
		rtn += "------\n";
		for (QString key : d->Templates.keys()) {
			rtn += "@" + key + "\n";
			rtn += d->Templates[key] + "\n";
		}
		return rtn;
	}
	QString StyleSheetTemplate::getRawStyleSheet(const QString& key) {
		return d->Templates[key];
	}
	QString StyleSheetTemplate::getStyleSheet(const QString& key, YSSCore::Utility::JsonConfig* config, QWidget* getter) {
		if (config == nullptr && getter == nullptr) {
			return getRawStyleSheet(key);
		}
		else {
			QString temp = getRawStyleSheet(key);
			if (config != nullptr) {
				QStringList paras;
				QRegularExpression reg("\\$\\([\\D\\d]+?\\)");
				QRegularExpressionMatchIterator matchs = reg.globalMatch(temp);
				for (auto match : matchs ) {
					paras.append(match.captured());
				}
				for (QString para : paras) {
					temp.replace(para, config->getString(para.mid(2, para.length() - 3)));
				}
			}
			if (getter != nullptr) {
				//TODO
			}
			return temp;
		}
	}
	void StyleSheetTemplate::setStyleSheetTemplate(const QString& key, const QString& style) {
		d->Templates[key] = style;
	}
	void StyleSheetTemplate::setTemplateName(const QString& name) {
		d->TemplateName = name;
	}
	QString StyleSheetTemplate::getTemplateName() {
		return d->TemplateName;
	}
	QString StyleSheetTemplate::getTemplateID(){
		return d->TemplateID;
	}
	void StyleSheetTemplate::setTemplateID(const QString& id) {
		d->TemplateID = id;
	}
}