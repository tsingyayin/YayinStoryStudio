#pragma once
#include <QtCore/qlist.h>
#include <QtCore/qvariant.h>
#include <QtCore/qmetaobject.h>
class QString;

class ASEParameterPrivate;
class ASEParameter {
public:
	enum class Type {
		Undefined = -1,
		String = 1,
		Number,
		Integer,
		Float,
		Enum,
		Vector,
		Bool,
		None,
	};
public:
	ASEParameter();
	ASEParameter(const ASEParameter& other);
	ASEParameter(ASEParameter&& other) noexcept;
	ASEParameter& operator=(const ASEParameter& other);
	ASEParameter& operator=(ASEParameter&& other) noexcept;
	~ASEParameter();
	void parse(const QString& rawInput);
	QString getRawInput() const;
	QString generate();
	void setType(Type type);
	Type getType();
	void setName(const QString& name);
	QString getName();
	qint32 toEnum(const QMetaEnum& metaEnum);
	void fromEnum(const QMetaEnum& metaEnum, qint32 value);
	QString toString();
	void fromString(const QString& value);
	qint32 toInt();
	void fromInt(qint32 value);
	float toFloat();
	void fromFloat(float value);
	bool toBool();
	void fromBool(bool value);
	QList<QVariant> toVector();
	void fromVector(const QList<QVariant>& value);
	bool isRichText();
private:
	ASEParameterPrivate* d;
};