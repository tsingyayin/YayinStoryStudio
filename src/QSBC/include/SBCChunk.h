#pragma once
#include "Macro.h"
#include <QtCore/qstring.h>
#include <QtCore/qtypes.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>

namespace Visindigo::QSBC {
	VIAPIClass(SBCChunk) {
		VImpl(SBCChunk);
public:
	enum HeadType : quint8 {
		T1 = 0b00000000,
		T2 = 0b00100000,
		T3 = 0b01000000,
		T4 = 0b01100000,
		T5 = 0b10000000,
		T6 = 0b10100000,
		T7 = 0b11000000,
		T8 = 0b11100000,
	};
	enum DataType : quint8 {
		unknown = 0b00000000,
		uint8 = 0b00000001,
		int8 = 0b00000010,
		uint16 = 0b00000011,
		int16 = 0b00000100,
		uint32 = 0b00000101,
		int32 = 0b00000110,
		uint64 = 0b00000111,
		int64 = 0b00001000,
		boolean = 0b00001001,
		float32 = 0b00001010,
		float64 = 0b00001011,
		character = 0b00001100,
		map = 0b00001101,
		array = 0b00001110,
		string = 0b00001111,
		none = 0b00010000,
		any = 0b00010001,
		binary = 0b00010010,
	};
public:
	SBCChunk();
	~SBCChunk();
	VICopyable(SBCChunk);
	VIMoveable(SBCChunk);
	SBCChunk(quint8 data, const QString & name = "");
	SBCChunk(quint16 data, const QString & name = "");
	SBCChunk(quint32 data, const QString & name = "");
	SBCChunk(quint64 data, const QString & name = "");
	SBCChunk(qint8 data, const QString & name = "");
	SBCChunk(qint16 data, const QString & name = "");
	SBCChunk(qint32 data, const QString & name = "");
	SBCChunk(qint64 data, const QString & name = "");
	SBCChunk(bool data, const QString & name = "");
	SBCChunk(float data, const QString & name = "");
	SBCChunk(double data, const QString & name = "");
	SBCChunk(const QChar & data, const QString & name = "");
	SBCChunk(const QString & data, const QString & name = "");
	SBCChunk(QList<SBCChunk> data, const QString & name = "");
	SBCChunk(QMap<QString, SBCChunk> data, const QString & name = "");
	void setName(const QString & name);
	QString getName() const;
	void setAsNone();
	void setAsAny();
	void setUInt8(quint8 data);
	quint8 getUInt8() const;
	void setInt8(qint8 data);
	qint8 getInt8() const;
	void setUInt16(quint16 data);
	quint16 getUInt16() const;
	void setInt16(qint16 data);
	qint16 getInt16() const;
	void setUInt32(quint32 data);
	quint32 getUInt32() const;
	void setInt32(qint32 data);
	qint32 getInt32() const;
	void setUInt64(quint64 data);
	quint64 getUInt64() const;
	void setInt64(qint64 data);
	qint64 getInt64() const;
	void setBoolean(bool data);
	bool getBoolean() const;
	void setFloat32(float data);
	float getFloat32() const;
	void setFloat64(double data);
	double getFloat64() const;
	void setChar(const QChar & data);
	QChar getChar() const;
	void setString(const QString & data);
	QString getString() const;
	void setArray(const QList<SBCChunk>&data);
	QList<SBCChunk> getArray() const;
	void setMap(const QMap<QString, SBCChunk>&data);
	QMap<QString, SBCChunk> getMap() const;
	void setBinary(const QByteArray & data);
	QByteArray getBinary() const;
	DataType getDataType() const;
	bool isDataType(DataType type) const;
	HeadType getHeadType() const;
	bool isHeadType(HeadType type) const;
	qint32 getDataSize() const;
	qint32 getNameSize() const;
	QByteArray generate();
	QByteArray parse(const QByteArray & data);
	};
}