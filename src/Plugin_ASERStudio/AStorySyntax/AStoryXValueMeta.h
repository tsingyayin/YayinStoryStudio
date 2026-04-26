#ifndef ASERStudio_AStorySyntax_AStoryXValue_h
#define ASERStudio_AStorySyntax_AStoryXValue_h
#include "ASERStudioCompileMacro.h"
#include "AStorySyntax/AStoryXDiagnosticData.h"
#include <VIMacro.h>
#include <QtCore/qstring.h>
#include <QtCore/qobject.h>
#include <Utility/JsonConfig.h>
namespace ASERStudio::AStorySyntax {
	class AStoryXValueMetaPrivate;
	class ASERAPI AStoryXValueMeta {
		Q_GADGET;
	public:
		enum Type {
			Undefined = -1,
			String,
			Number,
			Integer,
			Float,
			Enum,
			Vector,
			Bool,
			None,
			__META__,
			Comment,
			Function,
			Parameter,
			Keyword,
			Macro,
			MacroParameter
		};
		Q_ENUM(Type);
	public:
		AStoryXValueMeta();
		AStoryXValueMeta(const QString& paramName, Type type = Type::Undefined, const QString& defaultValue = "");
		VIMoveable(AStoryXValueMeta);
		VICopyable(AStoryXValueMeta);
		~AStoryXValueMeta();
	public:
		void setMetaData(const QString& paramName, const Visindigo::Utility::JsonConfig& metaData);
		void setParameterName(const QString& paramName);
		QString getParameterName() const;
		void setType(Type type);
		Type getType() const;
		QString getTypeString() const;
		void setDefaultValue(const QString& defaultValue);
		QString getDefaultValue() const;
		void setStringCheckRegex(const QString& regex);
		QString getStringCheckRegex() const;
		void setIntegerCheckRange(qint64 min, qint64 max);
		QPair<qint64, qint64> getIntegerCheckRange() const;
		void setFloatCheckRange(double min, double max);
		QPair<double, double> getFloatCheckRange() const;
		void setVectorCheckRange(const QList<QPair<double, double>>& range, const QList<qint64>& dimensions);
		QList<QPair<double, double>> getVectorCheckRange() const;
		QList<qint64> getVectorCheckDimensions() const;
		void setEnumCheckList(const QStringList& enumList);
		QStringList getEnumCheckList() const;
		void setEnumCheckDimensions(const QList<qint64>& dimensions);
		QList<qint64> getEnumCheckDimensions() const;
		AStoryXDiagnosticData::DiagnosticType isTypeMatching(const QString& value) const;
		static Type guessType(const QString& value);
		static QString typeToString(Type type);
		static qint64 toInteger(const QString& value);
		static double toFloat(const QString& value);
		static QList<double> toVector(const QString& value);
		static bool toBool(const QString& value);
	private:
		AStoryXValueMetaPrivate* d;
	};
}
#endif // ASERStudio_AStorySyntax_AStoryXValue_h