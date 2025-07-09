#include "../ASRuleAdaptor.h"
#include <QtCore/qstring.h>
#include <QtCore/qregularexpression.h>
#include <Utility/JsonConfig.h>
#include <General/Log.h>
VImplClass(ASRuleSingleController) {
	friend class ASRuleSingleController;
protected:
	QString Name;
	QString RawRule;
	QStringList ParameterOrder;
	YSSCore::Utility::JsonConfig RuleMeta;
	QStringList SyntaxSigns;
	QString StartSign;
};

ASRuleSingleController::ASRuleSingleController() {
	d = new ASRuleSingleControllerPrivate;
}
ASRuleSingleController::ASRuleSingleController(const QString & name, const YSSCore::Utility::JsonConfig& config, const QString & rule) {
	d = new ASRuleSingleControllerPrivate;
	setControllerName(name);
	setControllerMeta(config);
	setControllerASRule(rule);
}

void ASRuleSingleController::setControllerName(const QString & name) {
	d->Name = name;
}

void ASRuleSingleController::setControllerASRule(const QString& rule) {
	d->RawRule = rule;
	QRegularExpression regex("__[\\D\\d]*?__");
	QRegularExpressionMatchIterator it = regex.globalMatch(rule);
	qint32 index = 0;
	for (auto match : it) {
		quint32 c_index = match.capturedStart();
		quint32 delta = c_index - index;
		if (delta > 0) {
			d->SyntaxSigns.append(rule.mid(index, delta));
		}
		index = match.capturedEnd();
		QString content = match.captured();
		d->ParameterOrder.append(content);
	}
	if (index != rule.length()) {
		d->SyntaxSigns.append(rule.mid(index));
	}
	if (!rule.startsWith("__")) {
		d->StartSign = d->SyntaxSigns.first();
		d->SyntaxSigns.removeFirst();
		yDebug << "Start sign detected:" << d->StartSign;
	}
	yDebug << "Rule" << d->Name << "has" << d->ParameterOrder.size() << "parameters:" << d->ParameterOrder;
	yDebug << "Syntax signs:" << d->SyntaxSigns;
}

QString ASRuleSingleController::getStartSign() {
	return d->StartSign;
}

void ASRuleSingleController::setControllerMeta(const YSSCore::Utility::JsonConfig& meta) {
	d->RuleMeta = meta;
}

QStringList ASRuleSingleController::parse(const QString& input) {
	yDebug << "Parsing input for rule" << d->Name << ":" << input;
	QRegularExpression protect("&\\{[\\d\\D]*?\\}");
	QRegularExpressionMatchIterator protectIt = protect.globalMatch(input);
	QMap<QString, QString> protectMap;
	quint32 i = 0;
	QString protectedInput = input;
	for (auto match : protectIt) {
		QString content = match.captured();
		QString key = "▲★▲" + QString::number(i++) + "▲★▲";
		protectedInput.replace(content, key);
		protectMap.insert(key, content.mid(2, content.length() - 3));
	}
	if (!d->StartSign.isEmpty() && protectedInput.startsWith(d->StartSign)) {
		protectedInput = protectedInput.mid(d->StartSign.length());
	}
	QStringList result;
	quint32 lastIndex = 0;
	for (const QString& sign : d->SyntaxSigns) {
		quint32 index = protectedInput.indexOf(sign);
		result.append(protectedInput.mid(lastIndex, index - lastIndex));
		lastIndex = index + sign.length();
	}
	if (lastIndex != protectedInput.length()) {
		result.append(protectedInput.mid(lastIndex));
	}
	yDebug << "Protected split finished" << d->Name << ":" << result;
	QStringList rtn;
	for (auto& item : result) {
		QRegularExpression regex("▲★▲\\d+▲★▲");
		QRegularExpressionMatchIterator it = regex.globalMatch(item);
		QString replaced = item;
		for (auto match : it) {
			replaced = replaced.replace(match.captured(), protectMap.value(match.captured()));
		}
		rtn.append(replaced);
	}
	return rtn;
}

VImplClass(ASRuleAdaptor) {
	friend class ASRuleAdaptor;
protected:
	
};
ASRuleAdaptor::ASRuleAdaptor() {
}
ASRuleAdaptor::~ASRuleAdaptor() {
}
void ASRuleAdaptor::setRulePath(const QString& path) {
	// This function is intended to set the path for rule files.
	// Implementation can be added later as needed.
	yDebug << "Setting rule path to:" << path;
}