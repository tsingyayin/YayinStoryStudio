#pragma once

class QString;

struct ASVector {
	float X = 0.0f;
	float Y = 0.0f;
	float Z = 0.0f;
};

class ASRuleParameter {
public:
	enum class Type {
		String,
		Number,
		Enum,
		Vector,
		Bool
	};
	ASRuleParameter();
	~ASRuleParameter();
	ASRuleParameter(const ASRuleParameter& other);
	ASRuleParameter(ASRuleParameter&& other);
	ASRuleParameter& operator=(const ASRuleParameter& other);
	ASRuleParameter& operator=(ASRuleParameter&& other);
	void parse(const QString& rawInput);
	void setParameterType(Type type);
	void setParameterName(const QString& name);
	QString getText();
	float getNumber();
	ASVector getVector();
};