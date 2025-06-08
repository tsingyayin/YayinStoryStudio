#pragma once
#include <QtCore/qtypes.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>

class QString;
namespace YSSCore::Utility {
	class VariablePrivate;
	class Variable {
	public:
		enum Type {
			None = 0,
			Int32 = 1,
			UInt32 = 2,
			Int64 = 3,
			UInt64 = 4,
			Bool = 5,
			Double = 6,
			String = 7,
			Dict = 8,
			Array = 9,
		};
	public:
		Variable();
		Variable(qint32 value);
		Variable(quint32 value);
		Variable(qint64 value);
		Variable(quint64 value);
		Variable(bool value);
		Variable(double value);
		Variable(const QString& value);
		Variable(const char* value);
		Variable(const Variable& other);
		Variable(const QMap<QString, Variable> value);
		Variable(const QList<Variable> value);
		Variable(Variable&& other) noexcept;
		~Variable();
		Variable& operator=(const Variable& other);
		Variable& operator=(Variable&& other) noexcept;
		Variable& operator=(qint32 value);
		Variable& operator=(quint32 value);
		Variable& operator=(qint64 value);
		Variable& operator=(quint64 value);
		Variable& operator=(bool value);
		Variable& operator=(double value);
		Variable& operator=(const QString& value);
		Variable& operator=(const char* value);
		Variable& operator=(const QMap<QString, Variable> value);
		Variable& operator=(const QList<Variable> value);
		void setValue(qint32 value);
		void setValue(quint32 value);
		void setValue(qint64 value);
		void setValue(quint64 value);
		void setValue(bool value);
		void setValue(double value);
		void setValue(const QString& value);
		void setValue(const char* value);
		void setValue(const QMap<QString, Variable> value);
		void setValue(const QList<Variable> value);
		bool isInt32();
		bool isUInt32();
		bool isInt64();
		bool isUInt64();
		bool isNumber();
		bool isBool();
		bool isDouble();
		bool isString();
		bool isDict();
		bool isArray();
		Type getType();
		qint32 getInt32();
		quint32 getUInt32();
		qint64 getInt64();
		quint64 getUInt64();
		bool getBool();
		double getDouble();
		QString getString();
		QMap<QString, Variable> getDict();
		QList<Variable> getArray();
	private:
		VariablePrivate* d;
	};
}