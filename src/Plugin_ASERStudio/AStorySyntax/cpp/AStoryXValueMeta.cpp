#include "AStorySyntax/AStoryXValueMeta.h"
#include "AStorySyntax/AStoryXDiagnosticData.h"
#include <QtCore/qregularexpression.h>
namespace ASERStudio::AStorySyntax {
	class AStoryXValueMetaPrivate {
		friend class AStoryXValueMeta;
	protected:
		AStoryXValueMeta::Type Type = AStoryXValueMeta::Type::Undefined;
		QString ParameterName;
		QString StringCheckRegex;
		QPair<qint64, qint64> IntegerCheckRange;
		QPair<double, double> FloatCheckRange;
		QList<QPair<double, double>> VectorCheckRange;
		QStringList StringListCheck;
		QList<qint64> VectorCheckDimensions;
		QStringList EnumCheckList;
		QString DefaultValue;

		AStoryXDiagnosticData::DiagnosticType isString(const QString& value) const {
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
				return AStoryXDiagnosticData::DiagnosticType::ParameterTypeMismatch;
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
				return AStoryXDiagnosticData::DiagnosticType::ParameterTypeMismatch;
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
			if (not VectorCheckDimensions.contains(components.size())) {
				return AStoryXDiagnosticData::DiagnosticType::ParameterFormatError;
			}
			int index = 0;
			for (const QString& component : components) {
				if (index >= VectorCheckRange.size()) {
					return AStoryXDiagnosticData::DiagnosticType::Undefined;
				}
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
		\class ASERStudio::AStorySyntax::AStoryXValueMeta
		\brief AStoryXValue用于对AStoryX中的参数进行类型定义和检查。
		\since ASERStudio 2.0
		\inmodule ASERStudio

		AStoryXValueMeta提供了对AStoryX参数类型的定义和检查功能。用户可以为参数设置类型、
		默认值以及各种检查规则（如正则表达式、数值范围等）。在解析AStoryX时，可以使用isTypeMatching方法来检查参数值是否符合预设的类型和规则，
		并获取相应的诊断信息。

		值得反复强调的是，AStoryXValueMeta是用于语法检查和数据转换的工具类，
		而非类似于std::variant或QVariant的通用数据容器。虽然它提供了从字符串解析获得
		其他数据类型的能力，但主要是为了方便在AStoryX语法解析过程中进行类型验证和错误诊断，
		而不是作为一个通用的值存储类。
	
		\section1 ValueMetaJSON
		AStoryXValueMeta的元数据可以通过JSON配置进行设置，JSON的基础结构如下：
		\badcode
		{
			"__ControllerName__":{
				"__ParameterName__":{
					"type": "__ParameterType__",
					"defaultValue": "__ParameterDefaultValue__"
				}
			}
		}
		\endcode
		即，对于每一个控制器，分别设置每个参数的元信息。其中ControllerName就是AStoryXController中给定的
		枚举量的名称。ParameterName则为ASE-Remake文档中定义的参数名称。

		值得指出的是，由于AStoryX的预处理部分不允许用户通过Rule.json自由定义，因此在本
		ASERStudio和ASE-Remake本体中均为硬编码的，并非数据驱动配置。因此，
		ASERStudio::AStoryXSyntax::AStoryXController::ControllerType中的Preprocessor在本类中无效。

		\section1 值的限制
		\section2 整型和浮点型的范围限制
		对于整型和浮点型量，如果同时存在上下限，则可以在参数中通过range字段，提供一个二元数组进行钳制，例如
		\badcode
		"__ParameterName__":{
			"type": "Float",
			"defaultValue": "__ParameterDefaultValue__"
			"range": [0.0, 1.0]
		}
		\endcode
		如果只有上限或只有下限，则只对需设限的上限（max）或下限（min）进行单独设置即可。
		\note 值得指出的是，虽然整型和浮点型在设置默认值时需要用字符串表示，但在设置钳制范围时则应使用数字类型。

		如果range和min、max同时存在，则range优先级更高，min和max将被忽略。
		注意，即使没有设置对应的上限/下限，程序内部也会自动初始化为该类型的理论容限。
		如整型上下限为int64的最小值和最大值，浮点型上下限为double的最小值和最大值。
		因此，可以通过检查上下限的钳制设置是否为默认值来判断是否对该参数进行了钳制。

		\section2 向量形的限制
		对于向量类型，使用dimension字段指定向量的维度。不指定时默认为三维。

		对于向量中的每一个分量，通过定义limit数组进行钳制，其中每一个数组内容都和设置单一浮点型/整型的钳制类似。
		不需要设限的分量留空（但不能置为null）即可，例如：
		\badcode
		"__ParameterName__":{
			"type": "Vector",
			"defaultValue": "0.0,0.0,0.0",
			"dimension": 3,
			"limit": [
				{
					"min": 0.0
				},
				{ },
				{
					"range": [-1.0, 1.0]
				}
			]
		}
		\endcode
		每个分量的钳制规则、优先级策略和默认值都和单一的整型/浮点型参数相同。

		\section2 枚举型的限制
		对于枚举型，则需要通过”enums"字段声明该枚举量的枚举列表名称，然后直接在MetaJson的根节点上
		定义__GeneralMetaData__，并在其中用该名称定义具有全部枚举值的枚举列表，例如
		\badcode
		{
			"__ControllerName__":{
				"__ParameterName__":{
					"type": "Enum",
					"defaultValue": "Value1",
					"enums": "MyEnum1"
				}
			},
			"__GeneralMetaData__":{
				"MyEnum1": ["Value1", "Value2", "Value3"]
			}
		}
		\endcode

		\section2 字符串的限制
		对于字符串类型，可以通过regex字段提供一个正则表达式来限制字符串的格式，例如
		\badcode
		"__ParameterName__":{
			"type": "String",
			"defaultValue": "__ParameterDefaultValue__",
			"regex": "^[a-zA-Z0-9_]+$"
		}
		\endcode
		不提供regex时，不进行任何限制。
	*/

	/*!
		\since ASERStudio 2.0
		\enum ASERStudio::AStorySyntax::AStoryXValueMeta::Type
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
	AStoryXValueMeta::AStoryXValueMeta() : d(new AStoryXValueMetaPrivate) {
	}

	AStoryXValueMeta::AStoryXValueMeta(const QString& paramName, Type type, const QString& defaultValue) : d(new AStoryXValueMetaPrivate) {
		d->ParameterName = paramName;
		d->Type = type;
		d->DefaultValue = defaultValue;
	}

	/*!
		\since ASERStudio 2.0
		析构函数
	*/
	AStoryXValueMeta::~AStoryXValueMeta() {
		delete d;
	}

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXValueMeta::AStoryXValueMeta(AStoryXValueMeta&& other) noexcept
		\since ASERStudio 2.0
		移动构造函数
	*/

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXValueMeta::AStoryXValueMeta(const AStoryXValueMeta& other)
		\since ASERStudio 2.0
		复制构造函数
	*/

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXValueMeta::operator=(const AStoryXValueMeta& other)
		\since ASERStudio 2.0
		复制赋值运算符
	*/

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXValueMeta::operator=(AStoryXValueMeta&& other) noexcept
		\since ASERStudio 2.0
		移动赋值运算符
	*/
	VIMoveable_Impl(AStoryXValueMeta);
	VICopyable_Impl(AStoryXValueMeta);

	/*!
		\since ASERStudio 2.0
		设置元数据。
		\a metaData 元数据。
	*/
	void AStoryXValueMeta::setMetaData(const QString& paramName, const Visindigo::Utility::JsonConfig& metaData) {
		d->ParameterName = paramName;
		if (metaData.isEmpty()) {
			return;
		}
		Visindigo::Utility::JsonConfig paramMeta = metaData.getObject(paramName);
		QString type = paramMeta.getString("type");
		d->Type = (AStoryXValueMeta::Type)QMetaEnum::fromType<AStoryXValueMeta::Type>().keyToValue(type.toStdString().c_str());
		d->DefaultValue = paramMeta.getString("defaultValue");
		switch (d->Type) {
			case AStoryXValueMeta::Type::String:
				d->StringCheckRegex = paramMeta.getString("regex");
				break;
			case AStoryXValueMeta::Type::Integer: {
				qint64 min = std::numeric_limits<qint64>::min();
				qint64 max = std::numeric_limits<qint64>::max();
				if (paramMeta.contains("range")) {
					min = paramMeta.contains("range.0") ? paramMeta.getInt("range.0") : std::numeric_limits<qint64>::min();
					max = paramMeta.contains("range.1") ? paramMeta.getInt("range.1") : std::numeric_limits<qint64>::max();
				}
				else {
					min = paramMeta.contains("min") ? paramMeta.getInt("min") : std::numeric_limits<qint64>::min();
					max = paramMeta.contains("max") ? paramMeta.getInt("max") : std::numeric_limits<qint64>::max();
				}
				d->IntegerCheckRange = qMakePair(min, max);
				break;
			}
			case AStoryXValueMeta::Type::Float: {
				double min = -std::numeric_limits<double>::infinity();
				double max = std::numeric_limits<double>::infinity();
				if (paramMeta.contains("range")) {
					min = paramMeta.contains("range.0") ? paramMeta.getDouble("range.0") : -std::numeric_limits<double>::infinity();
					max = paramMeta.contains("range.1") ? paramMeta.getDouble("range.1") : std::numeric_limits<double>::infinity();
				}
				else {
					min = paramMeta.contains("min") ? paramMeta.getDouble("min") : -std::numeric_limits<double>::infinity();
					max = paramMeta.contains("max") ? paramMeta.getDouble("max") : std::numeric_limits<double>::infinity();
				}
				d->FloatCheckRange = qMakePair(min, max);
				break;
			}
			case AStoryXValueMeta::Type::Vector: {
				QList<qint64> dimensions = paramMeta.contains("dimensions") ? paramMeta.getIntArray("dimensions") : QList<qint64>{ 3 };
				qint64 maxDim = 0;
				for (qint64 dim : dimensions) {
					if (dim > maxDim) {
						maxDim = dim;
					}
				}
				QList<QPair<double, double>> range;
				for (qint32 i = 0; i < maxDim; i++) {
					auto dimConfig = paramMeta.getObject(QString("limit.%1").arg(i));
					double min =  -std::numeric_limits<double>::infinity();
					double max = std::numeric_limits<double>::infinity();
					if (dimConfig.contains("range")) {
						min = dimConfig.contains("range.0") ? dimConfig.getDouble("range.0") : -std::numeric_limits<double>::infinity();
						max = dimConfig.contains("range.1") ? dimConfig.getDouble("range.1") : std::numeric_limits<double>::infinity();
					}
					else {
						min = dimConfig.contains("min") ? dimConfig.getDouble("min") : -std::numeric_limits<double>::infinity();
						max = dimConfig.contains("max") ? dimConfig.getDouble("max") : std::numeric_limits<double>::infinity();
					}
				}
				d->VectorCheckRange = range;
				d->VectorCheckDimensions = dimensions;
				break;
			}
			case AStoryXValueMeta::Type::Enum: {
				QString enumListName = paramMeta.getString("enums");
				d->EnumCheckList = metaData.getStringList("__GeneralMetaData__." + enumListName);
				break;
			}
		}
	}

	/*!
		\since ASERStudio 2.0
		设置参数名称。
		\a paramName 参数名称，必须是一个有效的QString对象。
	*/
	void AStoryXValueMeta::setParameterName(const QString& paramName) {
		d->ParameterName = paramName;
	}

	/*!
		\since ASERStudio 2.0
		获取参数名称。
		\return 参数名称，返回一个QString对象。如果未设置参数名称，则返回空字符串。
	*/
	QString AStoryXValueMeta::getParameterName() const {
		return d->ParameterName;
	}
	/*!
		\since ASERStudio 2.0
		设置参数类型。
		\a type 参数类型，必须是AStoryXValue::Type枚举中的一个值。
	*/
	void AStoryXValueMeta::setType(Type type) {
		d->Type = type;
	}

	/*!
		\since ASERStudio 2.0
		获取参数类型。
		\return 参数类型，返回值为AStoryXValue::Type枚举中的一个值。
	*/
	AStoryXValueMeta::Type AStoryXValueMeta::getType() const {
		return d->Type;
	}

	/*!
		\since ASERStudio 2.0
		获取参数类型的字符串表示。
	*/
	QString AStoryXValueMeta::getTypeString() const {
		return typeToString(d->Type);
	}


	/*!
		\since ASERStudio 2.0
		设置默认值。
		\a defaultValue 默认值，必须是一个QString对象。默认值的格式应与参数类型相匹配。
	*/
	void AStoryXValueMeta::setDefaultValue(const QString& defaultValue) {
		d->DefaultValue = defaultValue;
	}

	/*!
		\since ASERStudio 2.0
		获取默认值。
		\return 默认值，返回一个QString对象。如果未设置默认值，则返回空字符串。
	*/
	QString AStoryXValueMeta::getDefaultValue() const {
		return d->DefaultValue;
	}

	/*!
		\since ASERStudio 2.0
		设置字符串检查的正则表达式。
		\a regex 正则表达式，必须是一个有效的QString对象。该正则表达式将用于验证字符串类型参数的格式。
	*/
	void AStoryXValueMeta::setStringCheckRegex(const QString& regex) {
		d->StringCheckRegex = regex;
	}

	/*!
		\since ASERStudio 2.0
		获取字符串检查的正则表达式。
		\return 正则表达式，返回一个QString对象。如果未设置正则表达式，则返回空字符串。
	*/
	QString AStoryXValueMeta::getStringCheckRegex() const {
		return d->StringCheckRegex;
	}

	/*!
		\since ASERStudio 2.0
		设置整数检查的范围。
		\a min 最小值，必须是一个qint64类型的整数。
		\a max 最大值，必须是一个qint64类型的整数。max必须大于或等于min。
	*/
	void AStoryXValueMeta::setIntegerCheckRange(qint64 min, qint64 max) {
		d->IntegerCheckRange = qMakePair(min, max);
	}

	/*!
		\since ASERStudio 2.0
		获取整数检查的范围。
		\return 整数检查范围，返回一个QPair<qint64, qint64>对象，其中first表示最小值，second表示最大值。
		如果未设置范围，则返回默认的QPair对象，其first和second均为0。
	*/
	QPair<qint64, qint64> AStoryXValueMeta::getIntegerCheckRange() const {
		return d->IntegerCheckRange;
	}

	/*!
		\since ASERStudio 2.0
		设置浮点数检查的范围。
		\a min 最小值，必须是一个double类型的数值。
		\a max 最大值，必须是一个double类型的数值。max必须大于或等于min。
	*/
	void AStoryXValueMeta::setFloatCheckRange(double min, double max) {
		d->FloatCheckRange = qMakePair(min, max);
	}

	/*!
		\since ASERStudio 2.0
		获取浮点数检查的范围。
		\return 浮点数检查范围，返回一个QPair<double, double>对象，其中first表示最小值，second表示最大值。
		如果未设置范围，则返回默认的QPair对象，其first和second均为0.0。
	*/
	QPair<double, double> AStoryXValueMeta::getFloatCheckRange() const {
		return d->FloatCheckRange;
	}

	/*!
		\since ASERStudio 2.1
		设置向量检查的范围和维度。
		\a range 向量检查范围，必须是一个QList<QPair<double, double>>对象，其中每个QPair表示一个维度的最小值和最大值。
		range的大小必须等于dimensions中的最大值。
		\a dimensions 向量维度，必须是一个QList<qint64>对象，表示允许的维度值列表。dimensions中的每个值必须大于0。
	*/
	void AStoryXValueMeta::setVectorCheckRange(const QList<QPair<double, double>>& range, const QList<qint64>& dimensions) {
		d->VectorCheckRange = range;
		d->VectorCheckDimensions = dimensions;
	}

	/*!
		\since ASERStudio 2.0
		获取向量检查的范围。
		\return 向量检查范围，返回一个QList<QPair<double, double>>对象，其中每个QPair表示一个维度的最小值和最大值。
		如果未设置范围，则返回一个空的QList对象。
	*/
	QList<QPair<double, double>> AStoryXValueMeta::getVectorCheckRange() const {
		return d->VectorCheckRange;
	}

	/*!
		\since ASERStudio 2.1
		获取向量检查的维度。
		\return 向量检查维度，返回一个qint64的列表，代表允许的维度值列表。
		如果未设置维度，则返回0。
	*/
	QList<qint64> AStoryXValueMeta::getVectorCheckDimensions() const {
		return d->VectorCheckDimensions;
	}

	/*!
		\since ASERStudio 2.0
		设置枚举检查的列表。
		\a enumList 枚举检查列表，必须是一个QStringList对象，包含所有有效的枚举值。
	*/
	void AStoryXValueMeta::setEnumCheckList(const QStringList& enumList) {
		d->EnumCheckList = enumList;
	}

	/*!
		\since ASERStudio 2.0
		获取枚举检查的列表。
		\return 枚举检查列表，返回一个QStringList对象，包含所有有效的枚举值。
		如果未设置枚举检查列表，则返回一个空的QStringList对象。
	*/
	QStringList AStoryXValueMeta::getEnumCheckList() const {
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

		注意，这函数只检查Type在__META__之前的类型。__META__之后的类型是作为token使用的，不进行类型检查。
		如果你检查它们，则永远得到Undefined的结果。

		此外，它对空参数永远返回Undefined。
	*/
	AStoryXDiagnosticData::DiagnosticType AStoryXValueMeta::isTypeMatching(const QString& value) const {
		if (value.isEmpty()) {
			return AStoryXDiagnosticData::DiagnosticType::Undefined;
		}
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
		根据参数值的格式猜测参数类型。

		注意，这个函数只会在如下类型中按顺序猜测：Integer、Float、Vector、Bool和String。
		该函数最差（即前四种假设均失败时）也会返回String类型，而不会返回Undefined。
	*/
	AStoryXValueMeta::Type AStoryXValueMeta::guessType(const QString& value) {
		if (value.isEmpty()) {
			return Type::String;
		}
		bool okInt;
		value.toLongLong(&okInt);
		if (okInt) {
			return Type::Integer;
		}
		bool okFloat;
		value.toDouble(&okFloat);
		if (okFloat) {
			return Type::Float;
		}
		QStringList components = value.split(',');
		if (components.size() > 1) {
			bool allFloat = true;
			for (const QString& component : components) {
				if (!component.trimmed().toDouble(&okFloat)) {
					allFloat = false;
					break;
				}
			}
			if (allFloat) {
				return Type::Vector;
			}
		}
		QString lowerValue = value.toLower();
		if (lowerValue == "true" || lowerValue == "false" || lowerValue == "1" || lowerValue == "0" || lowerValue == "yes" || lowerValue == "no") {
			return Type::Bool;
		}
		return Type::String;
	}

	/*!
		\since ASERStudio 2.0
		将参数类型枚举值转换为字符串表示。
	*/
	QString AStoryXValueMeta::typeToString(Type type) {
		return QMetaEnum::fromType<AStoryXValueMeta::Type>().valueToKey(type);
	}
	/*!
		\since ASERStudio 2.0
		将参数值转换为整数。
		\a value 参数值，必须是一个符合整数类型要求的QString对象。
		return 转换结果，返回一个qint64类型的整数。如果转换失败（例如参数值不符合整数格式），则返回0。

		这函数不做任何检查。
	*/
	qint64 AStoryXValueMeta::toInteger(const QString& value) {
		return value.toLongLong();
	}

	/*!
		\since ASERStudio 2.0
		将参数值转换为浮点数。
		\a value 参数值，必须是一个符合浮点数类型要求的QString对象。
		return 转换结果，返回一个double类型的数值。如果转换失败（例如参数值不符合浮点数格式），则返回0.0。

		这函数不做任何检查。
	*/
	double AStoryXValueMeta::toFloat(const QString& value) {
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
	QList<double> AStoryXValueMeta::toVector(const QString& value) {
		QStringList components = value.split(',');
		QList<double> result;
		for (const QString& component : components) {
			result.append(component.trimmed().toDouble());
		}
		return result;
	}

	/*!
		\since ASERStudio 2.0
		将参数值转换为布尔值。
		\a value 参数值，必须是一个符合布尔类型要求的QString对象，例如"true"、"false"、"1"、"0"、"yes"或"no"等。
		return 转换结果，返回一个bool类型的值。如果转换失败（例如参数值不符合布尔格式），则返回false。
		这函数不做任何检查。
	*/
	bool AStoryXValueMeta::toBool(const QString& value) {
		QString lowerValue = value.toLower();
		return (lowerValue == "true" || lowerValue == "1" || lowerValue == "yes");
	}
}