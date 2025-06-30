#include "../ASEParameter.h"

class ASEParameterPrivate {
	friend class ASEParameter;
protected:
	QString Name;
	ASEParameter::Type Type = ASEParameter::Type::Undefined;
	QString RawInput;
	QVariant Value;
};

ASEParameter::ASEParameter() {
	d = new ASEParameterPrivate;
}

ASEParameter::ASEParameter(const ASEParameter& other) {
	d = new ASEParameterPrivate(*other.d);
}

ASEParameter::ASEParameter(ASEParameter&& other) noexcept {
	d = other.d;
	other.d = nullptr;
}

ASEParameter& ASEParameter::operator=(const ASEParameter& other) {
	if (this != &other) {
		delete d;
		d = new ASEParameterPrivate(*other.d);
	}
	return *this;
}

ASEParameter& ASEParameter::operator=(ASEParameter&& other) noexcept {
	if (this != &other) {
		delete d;
		d = other.d;
		other.d = nullptr;
	}
	return *this;
}

ASEParameter::~ASEParameter() {
	delete d;
}

void ASEParameter::parse(const QString& rawInput) {
	d->RawInput = rawInput;
	if (rawInput.isEmpty()) {
		d->Type = ASEParameter::Type::Undefined;
		return;
	}
	if (d->RawInput.startsWith("\"") && d->RawInput.endsWith("\"")) {
		d->Type = ASEParameter::Type::String;
		d->Value = d->RawInput.mid(1, d->RawInput.length() - 2);
		return;
	}
	static const QStringList boolValues = {
		"True", "true", "TRUE", "Yes", "yes", "YES",
		"False", "false", "FALSE", "No", "no", "NO"
	};
	if (boolValues.contains(d->RawInput)) {
		d->Type = ASEParameter::Type::Bool;
		if (d->RawInput.startsWith("T", Qt::CaseInsensitive) || 
			d->RawInput.startsWith("Y", Qt::CaseInsensitive)) {
			d->Value = true;
		} else {
			d->Value = false;
		}
		return;
	}
	if (d->RawInput.startsWith("[") && d->RawInput.endsWith("]")) {
		d->Type = ASEParameter::Type::Vector;
		d->Value = QVariant::fromValue(QList<QVariant>()); // Placeholder for vector parsing
		return;
	}
	bool ok = false;
	qint32 value = d->RawInput.toInt(&ok);
	if (ok) {
		d->Type = ASEParameter::Type::Integer;
		d->Value = value;
		return;
	}
	qint32 floatValue = d->RawInput.toFloat(&ok);
	if (ok) {
		d->Type = ASEParameter::Type::Float;
		d->Value = floatValue;
		return;
	}
	// Parsing logic here (not implemented in this snippet)
}

QString ASEParameter::getRawInput() const {
	return d->RawInput;
}

QString ASEParameter::generate() {
	// Generation logic here (not implemented in this snippet)
	return QString();
}

void ASEParameter::setType(Type type) {
	d->Type = type;
}

ASEParameter::Type ASEParameter::getType() {
	return d->Type;
}

void ASEParameter::setName(const QString& name) {
	d->Name = name;
}

QString ASEParameter::getName() {
	return d->Name;
}

qint32 ASEParameter::toEnum(const QMetaEnum& metaEnum) {
	metaEnum.keyToValue(d->RawInput.toUtf8().constData());
}

void ASEParameter::fromEnum(const QMetaEnum& metaEnum, qint32 value) {
	d->RawInput = QString::fromUtf8(metaEnum.valueToKey(value));
	d->Type = ASEParameter::Type::Enum;
}

QString ASEParameter::toString() {
	if (d->Type == ASEParameter::Type::String) {
		return d->Value.toString();
	}
	return QString();
}

void ASEParameter::fromString(const QString& value) {
	d->Value = value;
	d->Type = ASEParameter::Type::String;
}

qint32 ASEParameter::toInt() {
	if (d->Type == ASEParameter::Type::Integer) {
		return d->Value.toInt();
	}
	return 0;
}

void ASEParameter::fromInt(qint32 value) {
	d->Value = value;
	d->Type = ASEParameter::Type::Integer;
}

float ASEParameter::toFloat() {
	if (d->Type == ASEParameter::Type::Float) {
		return d->Value.toFloat();
	}
	return 0.0f;
}

void ASEParameter::fromFloat(float value) {
	d->Value = value;
	d->Type = ASEParameter::Type::Float;
}

bool ASEParameter::toBool() {
	if (d->Type == ASEParameter::Type::Bool) {
		return d->Value.toBool();
	}
	return false;
}

void ASEParameter::fromBool(bool value) {
	d->Value = value;
	d->Type = ASEParameter::Type::Bool;
}

QList<QVariant> ASEParameter::toVector() {
	if (d->Type == ASEParameter::Type::Vector) {
		return d->Value.toList();
	}
	return QList<QVariant>();
}

void ASEParameter::fromVector(const QList<QVariant>& value) {
	d->Value = QVariant::fromValue(value);
	d->Type = ASEParameter::Type::Vector;
}

bool ASEParameter::isRichText() {
	// Check if the raw input contains rich text formatting
	return d->RawInput.contains("<") && d->RawInput.contains(">");
}