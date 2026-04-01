#ifndef ASERStudio_AStorySyntax_AStoryXValue_h
#define ASERStudio_AStorySyntax_AStoryXValue_h
#include "ASERStudioCompileMacro.h"
#include "AStorySyntax/AStoryXDiagnosticData.h"
#include <VIMacro.h>
#include <QtCore/qstring.h>
#include <QtCore/qobject.h>
#include <Utility/JsonConfig.h>
namespace ASERStudio::AStorySyntax {
	class AStoryXValuePrivate;
	class ASERAPI AStoryXValue {
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
			None
		};
		Q_ENUM(Type);
	public:
		AStoryXValue();
		VIMoveable(AStoryXValue);
		VICopyable(AStoryXValue);
		~AStoryXValue();
	public:
		void setMetaData(const QString& paramName, const Visindigo::Utility::JsonConfig& metaData);
		void setParameterName(const QString& paramName);
		QString getParameterName() const;
		void setType(Type type);
		Type getType() const;
		void setDefaultValue(const QString& defaultValue);
		QString getDefaultValue() const;
		void setStringCheckRegex(const QString& regex);
		QString getStringCheckRegex() const;
		void setIntegerCheckRange(qint64 min, qint64 max);
		QPair<qint64, qint64> getIntegerCheckRange() const;
		void setFloatCheckRange(double min, double max);
		QPair<double, double> getFloatCheckRange() const;
		void setVectorCheckRange(const QList<QPair<double, double>>& range, qint32 dimension);
		QList<QPair<double, double>> getVectorCheckRange() const;
		qint32 getVectorCheckDimension() const;
		void setEnumCheckList(const QStringList& enumList);
		QStringList getEnumCheckList() const;
		AStoryXDiagnosticData::DiagnosticType isTypeMatching(const QString& value) const;
		qint64 toInteger(const QString& value) const;
		double toFloat(const QString& value) const;
		QList<double> toVector(const QString& value) const;
	private:
		AStoryXValuePrivate* d;
	};
}
#endif // ASERStudio_AStorySyntax_AStoryXValue_h