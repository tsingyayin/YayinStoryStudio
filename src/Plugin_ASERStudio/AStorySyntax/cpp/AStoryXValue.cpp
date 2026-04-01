#include "AStorySyntax/AStoryXValue.h"
#include "AStorySyntax/AStoryXDiagnosticData.h"
#include <QtCore/qregularexpression.h>
namespace ASERStudio::AStorySyntax {
	class AStoryXValuePrivate {
		friend class AStoryXValue;
	protected:
		AStoryXValue::Type Type = AStoryXValue::Type::Undefined;
		QString ParameterName;
		QString StringCheckRegex;
		QPair<qint64, qint64> IntegerCheckRange;
		QPair<double, double> FloatCheckRange;
		QList<QPair<double, double>> VectorCheckRange;
		qint32 VectorCheckDimension;
		QStringList EnumCheckList;
		QString DefaultValue;

		AStoryXDiagnosticData::DiagnosticType isString(const QString& value) const {
			if (value.isEmpty()) {
				return AStoryXDiagnosticData::DiagnosticType::ParameterFormatError;
			}
			if (StringCheckRegex.isEmpty()) {
				return AStoryXDiagnosticData::DiagnosticType::Undefined;
			}
			QRegularExpression regex(StringCheckRegex);
			if (!regex.pattern().isEmpty() && !regex.match(value).hasMatch()) {
				return AStoryXDiagnosticData::DiagnosticType::ParameterFormatError;
			}
			return AStoryXDiagnosticData::DiagnosticType::Undefined;
		}
		AStoryXDiagnosticData::DiagnosticType isInteger(const QString& value) const {
			bool ok;
			qint64 intValue = value.toLongLong(&ok);
			if (!ok) {
				return AStoryXDiagnosticData::DiagnosticType::ParameterFormatError;
			}
			if (intValue < IntegerCheckRange.first || intValue > IntegerCheckRange.second) {
				return AStoryXDiagnosticData::DiagnosticType::ParameterOutOfRange;
			}
			return AStoryXDiagnosticData::DiagnosticType::Undefined;
		};
		AStoryXDiagnosticData::DiagnosticType isFloat(const QString& value) const {
			bool ok;
			double floatValue = value.toDouble(&ok);
			if (!ok) {
				return AStoryXDiagnosticData::DiagnosticType::ParameterFormatError;
			}
			if (floatValue < FloatCheckRange.first || floatValue > FloatCheckRange.second) {
				return AStoryXDiagnosticData::DiagnosticType::ParameterOutOfRange;
			}
			return AStoryXDiagnosticData::DiagnosticType::Undefined;
		}
		AStoryXDiagnosticData::DiagnosticType isEnum(const QString& value) const {
			if (!EnumCheckList.contains(value)) {
				return AStoryXDiagnosticData::DiagnosticType::ParameterOutOfRange;
			}
			return AStoryXDiagnosticData::DiagnosticType::Undefined;
		};
		AStoryXDiagnosticData::DiagnosticType isVector(const QString& value) const {
			QStringList components = value.split(',');
			if (components.size() != VectorCheckDimension) {
				return AStoryXDiagnosticData::DiagnosticType::ParameterFormatError;
			}
			int index = 0;
			for (const QString& component : components) {
				bool ok;
				double floatValue = component.trimmed().toDouble(&ok);
				if (!ok) {
					return AStoryXDiagnosticData::DiagnosticType::ParameterFormatError;
				}
				if (floatValue < VectorCheckRange[index].first || floatValue > VectorCheckRange[index].second) {
					return AStoryXDiagnosticData::DiagnosticType::ParameterOutOfRange;
				}
				index++;
			}
			return AStoryXDiagnosticData::DiagnosticType::Undefined;
		}
		AStoryXDiagnosticData::DiagnosticType isBool(const QString& value) const {
			static const QStringList acceptedValues = { "true", "false", "1", "0", "yes", "no" };
			if (!acceptedValues.contains(value.toLower())) {
				return AStoryXDiagnosticData::DiagnosticType::ParameterFormatError;
			}
			return AStoryXDiagnosticData::DiagnosticType::Undefined;
		}
	};

	/*!
		\class ASERStudio::AStorySyntax::AStoryXValue
		\brief AStoryXValue用于对AStoryX中的参数进行类型定义和检查。
		\since ASERStudio 2.0
		\inmodule ASERStudio

		AStoryXValue提供了对AStoryX参数类型的定义和检查功能。用户可以为参数设置类型、
		默认值以及各种检查规则（如正则表达式、数值范围等）。在解析AStoryX时，可以使用isTypeMatching方法来检查参数值是否符合预设的类型和规则，
		并获取相应的诊断信息。

		值得反复强调的是，AStoryXValue是用于语法检查和数据转换的工具类，
		而非类似于std::variant或QVariant的通用数据容器。它的设计目的是为了在AStoryX语法解析过程中提供类型定义和验证功能。
	*/

	/*!
		\since ASERStudio 2.0
		\enum ASERStudio::AStorySyntax::AStoryXValue::Type
		\value Undefined 未定义类型，表示尚未设置具体类型。
		\value String 字符串类型，参数值应为文本。
		\value Number 数字类型，参数值应为数值（整数或浮点数）。
		\value Integer 整数类型，参数值应为整数。
		\value Float 浮点数类型，参数值应为浮点数。
		\value Enum 枚举类型，参数值应为预定义的枚举值之一。
		\value Vector 向量类型，参数值应为逗号分隔的数值列表。
		\value Bool 布尔类型，参数值应为true/false、1/0、yes/no等表示布尔值的字符串。
		\value None 无类型，参数值应为空字符串。
	*/

	/*!
		\since ASERStudio 2.0
		构造函数
	*/
	AStoryXValue::AStoryXValue() : d(new AStoryXValuePrivate) {
	}

	/*!
		\since ASERStudio 2.0
		析构函数
	*/
	AStoryXValue::~AStoryXValue() {
		delete d;
	}

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXValue::AStoryXValue(AStoryXValue&& other) noexcept
		\since ASERStudio 2.0
		移动构造函数
	*/

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXValue::AStoryXValue(const AStoryXValue& other)
		\since ASERStudio 2.0
		复制构造函数
	*/

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXValue::operator=(const AStoryXValue& other)
		\since ASERStudio 2.0
		复制赋值运算符
	*/

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXValue::operator=(AStoryXValue&& other) noexcept
		\since ASERStudio 2.0
		移动赋值运算符
	*/
	VIMoveable_Impl(AStoryXValue);
	VICopyable_Impl(AStoryXValue);

	/*!
		\since ASERStudio 2.0
		设置元数据。
		\a metaData 元数据。
	*/
	void AStoryXValue::setMetaData(const QString& paramName, const Visindigo::Utility::JsonConfig& metaData) {
		d->ParameterName = paramName;
		if (metaData.isEmpty()) {
			return;
		}
		Visindigo::Utility::JsonConfig paramMeta = metaData.getObject(paramName);
		QString type = paramMeta.getString("type");
		d->Type = (AStoryXValue::Type)QMetaEnum::fromType<AStoryXValue::Type>().keyToValue(type.toStdString().c_str());
		d->DefaultValue = paramMeta.getString("defaultValue");
		switch (d->Type) {
			case AStoryXValue::Type::String:
				d->StringCheckRegex = paramMeta.getString("regex");
				break;
			case AStoryXValue::Type::Integer: {
				qint64 min = paramMeta.getInt("range.0");
				qint64 max = paramMeta.getInt("range.1");
				d->IntegerCheckRange = qMakePair(min, max);
				break;
			}
			case AStoryXValue::Type::Float: {
				double min = paramMeta.getDouble("range.0");
				double max = paramMeta.getDouble("range.1");
				d->FloatCheckRange = qMakePair(min, max);
				break;
			}
			case AStoryXValue::Type::Vector: {
				qint32 dimension = paramMeta.getInt("dimension");
				QList<QPair<double, double>> range;
				for (qint32 i = 0; i < dimension; i++) {
					double min = paramMeta.getDouble(QString("range.%1").arg(i*2));
					double max = paramMeta.getDouble(QString("range.%1").arg(i*2+1));
					range.append(qMakePair(min, max));
				}
				d->VectorCheckRange = range;
				d->VectorCheckDimension = dimension;
				break;
			}
			case AStoryXValue::Type::Enum: {
				QString enumListName = paramMeta.getString("enums");
				d->EnumCheckList = metaData.getStringList("__GeneralMetaData__." + enumListName);
				break;
			}
		}
	}
	/*!
		\since ASERStudio 2.0
		设置参数类型。
		\a type 参数类型，必须是AStoryXValue::Type枚举中的一个值。
	*/
	void AStoryXValue::setType(Type type) {
		d->Type = type;
	}

	/*!
		\since ASERStudio 2.0
		获取参数类型。
		\return 参数类型，返回值为AStoryXValue::Type枚举中的一个值。
	*/
	AStoryXValue::Type AStoryXValue::getType() const {
		return d->Type;
	}

	/*!
		\since ASERStudio 2.0
		设置默认值。
		\a defaultValue 默认值，必须是一个QString对象。默认值的格式应与参数类型相匹配。
	*/
	void AStoryXValue::setDefaultValue(const QString& defaultValue) {
		d->DefaultValue = defaultValue;
	}

	/*!
		\since ASERStudio 2.0
		获取默认值。
		\return 默认值，返回一个QString对象。如果未设置默认值，则返回空字符串。
	*/
	QString AStoryXValue::getDefaultValue() const {
		return d->DefaultValue;
	}

	/*!
		\since ASERStudio 2.0
		设置字符串检查的正则表达式。
		\a regex 正则表达式，必须是一个有效的QString对象。该正则表达式将用于验证字符串类型参数的格式。
	*/
	void AStoryXValue::setStringCheckRegex(const QString& regex) {
		d->StringCheckRegex = regex;
	}

	/*!
		\since ASERStudio 2.0
		获取字符串检查的正则表达式。
		\return 正则表达式，返回一个QString对象。如果未设置正则表达式，则返回空字符串。
	*/
	QString AStoryXValue::getStringCheckRegex() const {
		return d->StringCheckRegex;
	}

	/*!
		\since ASERStudio 2.0
		设置整数检查的范围。
		\a min 最小值，必须是一个qint64类型的整数。
		\a max 最大值，必须是一个qint64类型的整数。max必须大于或等于min。
	*/
	void AStoryXValue::setIntegerCheckRange(qint64 min, qint64 max) {
		d->IntegerCheckRange = qMakePair(min, max);
	}

	/*!
		\since ASERStudio 2.0
		获取整数检查的范围。
		\return 整数检查范围，返回一个QPair<qint64, qint64>对象，其中first表示最小值，second表示最大值。
		如果未设置范围，则返回默认的QPair对象，其first和second均为0。
	*/
	QPair<qint64, qint64> AStoryXValue::getIntegerCheckRange() const {
		return d->IntegerCheckRange;
	}

	/*!
		\since ASERStudio 2.0
		设置浮点数检查的范围。
		\a min 最小值，必须是一个double类型的数值。
		\a max 最大值，必须是一个double类型的数值。max必须大于或等于min。
	*/
	void AStoryXValue::setFloatCheckRange(double min, double max) {
		d->FloatCheckRange = qMakePair(min, max);
	}

	/*!
		\since ASERStudio 2.0
		获取浮点数检查的范围。
		\return 浮点数检查范围，返回一个QPair<double, double>对象，其中first表示最小值，second表示最大值。
		如果未设置范围，则返回默认的QPair对象，其first和second均为0.0。
	*/
	QPair<double, double> AStoryXValue::getFloatCheckRange() const {
		return d->FloatCheckRange;
	}

	/*!
		\since ASERStudio 2.0
		设置向量检查的范围和维度。
		\a range 向量检查范围，必须是一个QList<QPair<double, double>>对象，其中每个QPair表示一个维度的最小值和最大值。
		range的大小必须与dimension参数一致。
		\a dimension 向量维度，必须是一个qint32类型的整数，表示向量的维度。dimension必须大于0。
	*/
	void AStoryXValue::setVectorCheckRange(const QList<QPair<double, double>>& range, qint32 dimension) {
		d->VectorCheckRange = range;
		d->VectorCheckDimension = dimension;
	}

	/*!
		\since ASERStudio 2.0
		获取向量检查的范围。
		\return 向量检查范围，返回一个QList<QPair<double, double>>对象，其中每个QPair表示一个维度的最小值和最大值。
		如果未设置范围，则返回一个空的QList对象。
	*/
	QList<QPair<double, double>> AStoryXValue::getVectorCheckRange() const {
		return d->VectorCheckRange;
	}

	/*!
		\since ASERStudio 2.0
		获取向量检查的维度。
		\return 向量检查维度，返回一个qint32类型的整数，表示向量的维度。
		如果未设置维度，则返回0。
	*/
	qint32 AStoryXValue::getVectorCheckDimension() const {
		return d->VectorCheckDimension;
	}

	/*!
		\since ASERStudio 2.0
		设置枚举检查的列表。
		\a enumList 枚举检查列表，必须是一个QStringList对象，包含所有有效的枚举值。
	*/
	void AStoryXValue::setEnumCheckList(const QStringList& enumList) {
		d->EnumCheckList = enumList;
	}

	/*!
		\since ASERStudio 2.0
		获取枚举检查的列表。
		\return 枚举检查列表，返回一个QStringList对象，包含所有有效的枚举值。
		如果未设置枚举检查列表，则返回一个空的QStringList对象。
	*/
	QStringList AStoryXValue::getEnumCheckList() const {
		return d->EnumCheckList;
	}

	/*!
		\since ASERStudio 2.0
		检查给定的参数值是否符合预设的类型和规则。
		\a value 参数值，必须是一个QString对象。该值将根据当前设置的类型和检查规则进行验证。
		\return 诊断结果，返回一个AStoryXDiagnosticData::DiagnosticType枚举值，表示参数值的验证结果。
		如果参数值符合预设的类型和规则，则返回AStoryXDiagnosticData::DiagnosticType::Undefined。
		如果参数值不符合预设的类型或规则，则返回相应的错误类型，如ParameterFormatError或ParameterOutOfRange等。
		对于数字类型的超出范围以及枚举类型的无效值，返回ParameterOutOfRange；对于格式错误的值，
		例如未满足维度要求的向量，返回ParameterFormatError；
	*/
	AStoryXDiagnosticData::DiagnosticType AStoryXValue::isTypeMatching(const QString& value) const {
		switch (d->Type) {
		case Type::Number:
			return d->isFloat(value);
		case Type::String:
			return d->isString(value);
		case Type::Integer:
			return d->isInteger(value);
		case Type::Float:
			return d->isFloat(value);
		case Type::Enum:
			return d->isEnum(value);
		case Type::Vector:
			return d->isVector(value);
		case Type::Bool:
			return d->isBool(value);
		case Type::None:
			if (value.isEmpty()) {
				return AStoryXDiagnosticData::DiagnosticType::Undefined;
			}
			else {
				return AStoryXDiagnosticData::DiagnosticType::ParameterFormatError;
			}
		default:
			return AStoryXDiagnosticData::DiagnosticType::Undefined;
		}
	}

	/*!
		\since ASERStudio 2.0
		将参数值转换为整数。
		\a value 参数值，必须是一个符合整数类型要求的QString对象。
		return 转换结果，返回一个qint64类型的整数。如果转换失败（例如参数值不符合整数格式），则返回0。

		这函数不做任何检查。
	*/
	qint64 AStoryXValue::toInteger(const QString& value) const {
		return value.toLongLong();
	}

	/*!
		\since ASERStudio 2.0
		将参数值转换为浮点数。
		\a value 参数值，必须是一个符合浮点数类型要求的QString对象。
		return 转换结果，返回一个double类型的数值。如果转换失败（例如参数值不符合浮点数格式），则返回0.0。

		这函数不做任何检查。
	*/
	double AStoryXValue::toFloat(const QString& value) const {
		return value.toDouble();
	}

	/*!
		\since ASERStudio 2.0
		将参数值转换为向量。
		\a value 参数值，必须是一个符合向量类型要求的QString对象，即逗号分隔的数值列表。
		return 转换结果，返回一个QList<double>对象，包含向量的各个分量。
		如果整体切分失败，可能返回一个包含唯一0值的列表；如果切分成功但某个分量转换失败，则该分量的值为0。

		这函数不做任何检查。
	*/
	QList<double> AStoryXValue::toVector(const QString& value) const {
		QStringList components = value.split(',');
		QList<double> result;
		for (const QString& component : components) {
			result.append(component.trimmed().toDouble());
		}
		return result;
	}
}