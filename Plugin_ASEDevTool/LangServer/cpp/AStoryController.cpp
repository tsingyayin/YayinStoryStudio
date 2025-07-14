#include "../AStoryController.h"
#include "../AStoryControllerParseData.h"
#include "../private/AStoryControllerParseData_p.h"
#include <QtCore/qstring.h>
#include <QtCore/qregularexpression.h>
#include <Utility/JsonConfig.h>
#include <General/Log.h>

VImplClass(AStoryController) {
	friend class AStoryController;
protected:
	AStoryController::Name Name;
	QString RawRule;
	QStringList ParameterOrder;
	YSSCore::Utility::JsonConfig RuleMeta;
	QStringList SyntaxSigns;
	QString StartSign;
	QList<ASEParameter::Type> DefaultParameterTypes;
	AStoryControllerParseData ParseData;
};

AStoryController::Name AStoryController::fromNameString(const QString& name) {
	if (name == "talk") return AStoryController::Name::talk;
	if (name == "bg") return AStoryController::Name::bg;
	if (name == "music") return AStoryController::Name::music;
	if (name == "sound") return AStoryController::Name::sound;
	if (name == "cover") return AStoryController::Name::cover;
	if (name == "avg") return AStoryController::Name::avg;
	if (name == "flash") return AStoryController::Name::flash;
	if (name == "effect") return AStoryController::Name::effect;
	if (name == "shake") return AStoryController::Name::shake;
	if (name == "filter") return AStoryController::Name::filter;
	if (name == "delay") return AStoryController::Name::delay;
	if (name == "option") return AStoryController::Name::option;
	if (name == "jump") return AStoryController::Name::jump;
	if (name == "sign") return AStoryController::Name::sign;
	if (name == "hologram") return AStoryController::Name::hologram;
	if (name == "tele") return AStoryController::Name::tele;
	return AStoryController::Name::unknown; // Default case
}

QString AStoryController::toNameString(AStoryController::Name name) {
	switch (name) {
	case AStoryController::Name::talk: return "talk";
	case AStoryController::Name::bg: return "bg";
	case AStoryController::Name::music: return "music";
	case AStoryController::Name::sound: return "sound";
	case AStoryController::Name::cover: return "cover";
	case AStoryController::Name::avg: return "avg";
	case AStoryController::Name::flash: return "flash";
	case AStoryController::Name::effect: return "effect";
	case AStoryController::Name::shake: return "shake";
	case AStoryController::Name::filter: return "filter";
	case AStoryController::Name::delay: return "delay";
	case AStoryController::Name::option: return "option";
	case AStoryController::Name::jump: return "jump";
	case AStoryController::Name::sign: return "sign";
	case AStoryController::Name::hologram: return "hologram";
	case AStoryController::Name::tele: return "tele";
	default: return "unknown"; // Default case
	}
}

AStoryController::AStoryController() {
	d = new AStoryControllerPrivate;
}
AStoryController::AStoryController(const QString & name, YSSCore::Utility::JsonConfig& config, const QString & rule) {
	d = new AStoryControllerPrivate;
	setControllerName(name);
	setControllerMeta(config);
	setControllerASRule(rule);
}

VIMoveable_Impl(AStoryController);
VICopyable_Impl(AStoryController);

void AStoryController::setControllerName(const QString & name) {
	d->Name = AStoryController::fromNameString(name);
}

void AStoryController::setControllerASRule(const QString& rule) {
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
		content = content.mid(2, content.length() - 4); // Remove __ at the start and end
		d->ParameterOrder.append(content);
	}
	if (index != rule.length()) {
		d->SyntaxSigns.append(rule.mid(index));
	}
	//yDebugF << "Parsing rule:" << rule;
	if (!rule.startsWith("__")) {
		d->StartSign = d->SyntaxSigns.first();
		d->SyntaxSigns.removeFirst();
		//yDebugF << "Start sign detected:" << d->StartSign;
	}
	else {
		//yDebugF << "No start sign detected, maybe a talk controller";
	}
	//yDebugF << "Rule" << toNameString(d->Name) << "has" << d->ParameterOrder.size() << "parameters:" << d->ParameterOrder;
	//yDebugF << "Syntax signs:" << d->SyntaxSigns;
	d->DefaultParameterTypes.clear();
	for (auto order : d->ParameterOrder) {
		QString stype = d->RuleMeta.getString(order + ".Type");
		ASEParameter::Type type = ASEParameter::stringToType(stype);
		d->DefaultParameterTypes.append(type);
	}
	d->ParseData.d->ControllerName = d->Name;
	d->ParseData.d->setParameterTypes(d->DefaultParameterTypes);
	d->ParseData.d->setParameterNames(d->ParameterOrder);
	d->ParseData.d->setStartSign(d->StartSign);
}

QString AStoryController::getStartSign() {
	return d->StartSign;
}

void AStoryController::setControllerMeta(YSSCore::Utility::JsonConfig& meta) {
	d->RuleMeta = meta.getObject(AStoryController::toNameString(d->Name));
}

QList<ASEParameter::Type> AStoryController::getDefaultParameterTypes() {
	if (d->DefaultParameterTypes.isEmpty()) {
		yWarning << "Default parameter types are empty for controller" << d->Name;
	}
	return d->DefaultParameterTypes;
}

QStringList AStoryController::getDefaultParameterNames() {
	if (d->ParameterOrder.isEmpty()) {
		yWarning << "Parameter order is empty for controller" << d->Name;
	}
	return d->ParameterOrder;
}

AStoryControllerParseData AStoryController::parse(const QString& input) {
	static QRegularExpression protect("&\\{[\\d\\D]*?\\}");
	d->ParseData.d->clearParseData();
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
	quint32 removedLength = d->StartSign.length();
	if (!d->SyntaxSigns.isEmpty()) {
		for (const QString& sign : d->SyntaxSigns) {
			quint32 index = protectedInput.indexOf(sign);
			if (index == -1) {
				break; // No more syntax signs found
			}
			d->ParseData.d->addStartIndex(removedLength);
			result.append(protectedInput.left(index));
			protectedInput = protectedInput.mid(index+sign.size());
			removedLength += index + sign.size();
		}
	}
	if (!protectedInput.isEmpty()) {
		d->ParseData.d->addStartIndex(removedLength);
		result.append(protectedInput);
	}
	// yDebug << "Protected split finished" << d->Name << ":" << result;
	QStringList rtn;
	for (int i = 0; i < result.size(); i++) {
		QRegularExpression regex("▲★▲\\d+▲★▲");
		QRegularExpressionMatchIterator it = regex.globalMatch(result[i]);
		QString replaced = result[i];
		int rawSize = replaced.size();
		for (auto match : it) {
			replaced = replaced.replace(match.captured(), protectMap.value(match.captured()));
		}
		int delta = replaced.size() - rawSize;
		if (delta != 0) {
			replaced = "&{" + replaced + "}";
			//yDebugF << "Delta" << delta << "Detected";
			if (i + 1 != result.size()) {
				for (int j = i + 1; j < result.size(); j++) {
					d->ParseData.d->StartIndex[j] += delta +3; // 3 for &{}
				}
			}
		}
		rtn.append(replaced);
	}
	d->ParseData.d->setParameters(rtn);
	d->ParseData.d->setParameterNames(d->ParameterOrder);
	return d->ParseData;
}
