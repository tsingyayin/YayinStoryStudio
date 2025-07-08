#include "../include/SBCChunk.h"

namespace Visindigo::QSBC {
	VImplClass(SBCChunk) {
		VIAPI(SBCChunk);
protected:
		SBCChunk::HeadType Head = SBCChunk::T1;
		SBCChunk::DataType Type = SBCChunk::unknown;
		QByteArray Data;
		QString Name;
		QByteArray parse(const QByteArray & data) {
			quint8 headByte = static_cast<quint8>(data.at(0));
			Head = static_cast<SBCChunk::HeadType>(headByte & 0b11100000);
			Type = static_cast<SBCChunk::DataType>(headByte & 0b00011111);
			quint64 dataSize = 0;
			quint32 nameSize = 0;
			quint8 headLength = 0;
			switch (Head) {
				case SBCChunk::T1:
					headLength = 2;
					nameSize = ((quint8)data.at(1)) >> 5;
					dataSize = ((quint8)data.at(1)) & 0b00011111;
				case SBCChunk::T2:
					headLength = 2;
					nameSize = ((quint8)data.at(1)) >> 4;
					dataSize = ((quint8)data.at(1)) & 0b00001111;
					break;
				case SBCChunk::T3:
					headLength = 3;
					nameSize = ((quint8)data.at(1)) >> 2;
					dataSize = ((quint8)data.at(2)) + (((quint8)data.at(1)) & 0b00000011) << 8; // 可能溢出，需要修改
					break;
				case SBCChunk::T4:
					headLength = 3;
					nameSize = ((quint8)data.at(1)) >> 1;
					dataSize = ((quint8)data.at(2)) + (((quint8)data.at(1)) & 0b00000001) << 8; // 可能溢出，需要修改
					break;
				case SBCChunk::T5:
					headLength = 4;
					memcpy(&nameSize, data.constData() + 1, sizeof(quint8));
					memcpy(&dataSize, data.constData() + 2, sizeof(quint16));
					break;
				case SBCChunk::T6:
					headLength = 5;
					memcpy(&nameSize, data.constData() + 1, sizeof(quint8));
					memcpy(&dataSize, data.constData() + 1, sizeof(quint32));
					dataSize = dataSize & 0x00FFFFFF; // 可能端序有误，需要验证
					break;
				case SBCChunk::T7:
					headLength = 6;
					memcpy(&nameSize, data.constData() + 1, sizeof(quint8));
					memcpy(&dataSize, data.constData() + 2, sizeof(quint32));
					break;
				case SBCChunk::T8:
					headLength = 8;
					memcpy(&nameSize, data.constData() + 1, sizeof(quint16));
					memcpy(&dataSize, data.constData(), sizeof(quint64));
					dataSize = dataSize & 0x000000FFFFFFFFFF; // 可能端序有误，需要验证
			}
			Name = QString::fromUtf8(data.mid(headLength, nameSize));
			Data = data.mid(headLength + nameSize, dataSize);
			if (headLength+ nameSize + dataSize < data.size()) {
				return data.mid(headLength + nameSize + dataSize);
			}
			else {
				return QByteArray();
			}
		}
		void setName(const QString& name) {
			Name = name;
			refreshHeadType();
		}
		void setData(const QByteArray& data) {
			Data = data;
			refreshHeadType();
		}
		void clearData() {
			Data.clear();
			refreshHeadType();
		}
		void refreshHeadType() {
			if (Data.size() <= 0b00000111 && Name.size() <= 0b00011111) {
				Head = SBCChunk::T1;
			}
			else if (Data.size() <= 0b00001111 && Name.size() <= 0b00001111) {
				Head = SBCChunk::T2;
			}
			else if (Data.size() <= 0b00111111 && Name.size() <= 0b1111111111) {
				Head = SBCChunk::T3;
			}
			else if (Data.size() <= 0b01111111 && Name.size() <= 0b111111111) {
				Head = SBCChunk::T4;
			}
			else if (Data.size() <= 0xFF && Name.size() <= 0xFFFF) {
				Head = SBCChunk::T5;
			}
			else if (Data.size() <= 0xFF && Name.size() <= 0xFFFFFF) {
				Head = SBCChunk::T6;
			}
			else if (Data.size() <= 0xFF && Name.size() <= 0xFFFFFFFF) {
				Head = SBCChunk::T7;
			}
			else if (Data.size() <= 0xFFFF && Name.size() <= 0xFFFFFFFFFF) {
				Head = SBCChunk::T8;
			}else{
				Head = SBCChunk::T8;
			}
		}
		void setArray(QList<SBCChunk> data) {
			Type = SBCChunk::array;
			Data.clear();
			for (SBCChunk& chunk : data) {
				Data.append(chunk.generate());
			}
		}
		QList<SBCChunk> getArray() const {
			QList<SBCChunk> result;
			QByteArray remainingData = Data;
			while (!remainingData.isEmpty()) {
				SBCChunk chunk;
				remainingData = chunk.parse(remainingData);
				result.append(chunk);
			}
			return result;
		}
		void setMap(const QMap<QString, SBCChunk>& data) {
			Type = SBCChunk::map;
			Data.clear();
			for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
				SBCChunk chunk(it.value());
				chunk.setName(it.key());
				Data.append(chunk.generate());
			}
		}
		QMap<QString, SBCChunk> getMap() const {
			QMap<QString, SBCChunk> result;
			QByteArray remainingData = Data;
			while (!remainingData.isEmpty()) {
				SBCChunk chunk;
				remainingData = chunk.parse(remainingData);
				result.insert(chunk.getName(), chunk);
			}
			return result;
		}
		QByteArray generate() {
			QByteArray result;
			quint8 headByte = static_cast<quint8>(Head) | static_cast<quint8>(Type);
			result.append(static_cast<char>(headByte));
			quint32 nameSize = Name.size();
			quint32 dataSize = Data.size();
			switch (Head) {
				case SBCChunk::T1:
					result.append(static_cast<char>((nameSize << 5) | (dataSize & 0b00011111)));
					break;
				case SBCChunk::T2:
					result.append(static_cast<char>((nameSize << 4) | (dataSize & 0b00001111)));
					break;
				case SBCChunk::T3:
					result.append(static_cast<char>((nameSize << 2) | ((dataSize >> 8) & 0b00000011)));
					result.append(static_cast<char>(dataSize & 0xFF));
					break;
				case SBCChunk::T4:
					result.append(static_cast<char>((nameSize << 1) | ((dataSize >> 8) & 0b00000001)));
					result.append(static_cast<char>(dataSize & 0xFF));
					break;
				case SBCChunk::T5:
					result.append(static_cast<char>(nameSize));
					result.append(reinterpret_cast<const char*>(&dataSize), sizeof(quint16)); // 可能端序有误，需要验证
					break;
				case SBCChunk::T6:
					result.append(static_cast<char>(nameSize));
					result.append(reinterpret_cast<const char*>(&dataSize), 3); // 可能端序有误，需要验证
					break;
				case SBCChunk::T7:
					result.append(static_cast<char>(nameSize));
					result.append(reinterpret_cast<const char*>(&dataSize), sizeof(quint32)); // 可能端序有误，需要验证
					break;
				case SBCChunk::T8:
					result.append(reinterpret_cast<const char*>(&nameSize), sizeof(quint16)); // 可能端序有误，需要验证
					result.append(reinterpret_cast<const char*>(&dataSize), 6); // 可能端序有误，需要验证
			}
			result.append(Name.toUtf8());
			result.append(Data);
			return result;
		}
	};

	SBCChunk::SBCChunk() {
		d = new SBCChunkPrivate();
	}

	SBCChunk::~SBCChunk() {
		delete d;
	}

	VICopyable_Impl(SBCChunk);
	VIMoveable_Impl(SBCChunk);

	SBCChunk::SBCChunk(quint8 data, const QString& name) {
		d = new SBCChunkPrivate();
		setUInt8(data);
		setName(name);
	}

	SBCChunk::SBCChunk(quint16 data, const QString& name) {
		d = new SBCChunkPrivate();
		setUInt16(data);
		setName(name);
	}

	SBCChunk::SBCChunk(quint32 data, const QString& name) {
		d = new SBCChunkPrivate();
		setUInt32(data);
		setName(name);
	}

	SBCChunk::SBCChunk(quint64 data, const QString& name) {
		d = new SBCChunkPrivate();
		setUInt64(data);
		setName(name);
	}

	SBCChunk::SBCChunk(qint8 data, const QString& name) {
		d = new SBCChunkPrivate();
		setInt8(data);
		setName(name);
	}

	SBCChunk::SBCChunk(qint16 data, const QString& name) {
		d = new SBCChunkPrivate();
		setInt16(data);
		setName(name);
	}

	SBCChunk::SBCChunk(qint32 data, const QString& name) {
		d = new SBCChunkPrivate();
		setInt32(data);
		setName(name);
	}

	SBCChunk::SBCChunk(qint64 data, const QString& name) {
		d = new SBCChunkPrivate();
		setInt64(data);
		setName(name);
	}

	SBCChunk::SBCChunk(bool data, const QString& name) {
		d = new SBCChunkPrivate();
		setBoolean(data);
		setName(name);
	}

	SBCChunk::SBCChunk(float data, const QString& name) {
		d = new SBCChunkPrivate();
		setFloat32(data);
		setName(name);
	}

	SBCChunk::SBCChunk(double data, const QString& name) {
		d = new SBCChunkPrivate();
		setFloat64(data);
		setName(name);
	}

	SBCChunk::SBCChunk(const QChar& data, const QString& name) {
		d = new SBCChunkPrivate();
		setChar(data);
		setName(name);
	}

	SBCChunk::SBCChunk(const QString& data, const QString& name) {
		d = new SBCChunkPrivate();
		setString(data);
		setName(name);
	}

	SBCChunk::SBCChunk(QList<SBCChunk> data, const QString& name) {
		d = new SBCChunkPrivate();
		setArray(data);
		setName(name);
	}

	SBCChunk::SBCChunk(QMap<QString, SBCChunk> data, const QString& name) {
		d = new SBCChunkPrivate();
		setMap(data);
		setName(name);
	}

	void SBCChunk::setName(const QString& name) {
		d->Name = name;
	}

	QString SBCChunk::getName() const {
		return d->Name;
	}

	void SBCChunk::setAsNone() {
		d->Type = SBCChunk::none;
		d->clearData();
	}

	void SBCChunk::setAsAny() {
		d->Type = SBCChunk::any;
		d->clearData();
	}

	void SBCChunk::setUInt8(quint8 data) {
		d->Type = SBCChunk::uint8;
		d->setData(QByteArray::fromRawData(reinterpret_cast<const char*>(&data), sizeof(data)));
	}

	quint8 SBCChunk::getUInt8() const {
		if (d->Type != SBCChunk::uint8) {
			return 0;
		}
		quint8 value;
		memcpy(&value, d->Data.constData(), sizeof(value));
		return value;
	}

	void SBCChunk::setInt8(qint8 data) {
		d->Type = SBCChunk::int8;
		d->setData(QByteArray::fromRawData(reinterpret_cast<const char*>(&data), sizeof(data)));
	}

	qint8 SBCChunk::getInt8() const {
		if (d->Type != SBCChunk::int8) {
			return 0;
		}
		qint8 value;
		memcpy(&value, d->Data.constData(), sizeof(value));
		return value;
	}

	void SBCChunk::setUInt16(quint16 data) {
		d->Type = SBCChunk::uint16;
		d->setData(QByteArray::fromRawData(reinterpret_cast<const char*>(&data), sizeof(data)));
	}

	quint16 SBCChunk::getUInt16() const {
		if (d->Type != SBCChunk::uint16) {
			return 0;
		}
		quint16 value;
		memcpy(&value, d->Data.constData(), sizeof(value));
		return value;
	}

	void SBCChunk::setInt16(qint16 data) {
		d->Type = SBCChunk::int16;
		d->setData(QByteArray::fromRawData(reinterpret_cast<const char*>(&data), sizeof(data)));
	}

	qint16 SBCChunk::getInt16() const {
		if (d->Type != SBCChunk::int16) {
			return 0;
		}
		qint16 value;
		memcpy(&value, d->Data.constData(), sizeof(value));
		return value;
	}

	void SBCChunk::setUInt32(quint32 data) {
		d->Type = SBCChunk::uint32;
		d->setData(QByteArray::fromRawData(reinterpret_cast<const char*>(&data), sizeof(data)));
	}

	quint32 SBCChunk::getUInt32() const {
		if (d->Type != SBCChunk::uint32) {
			return 0;
		}
		quint32 value;
		memcpy(&value, d->Data.constData(), sizeof(value));
		return value;
	}

	void SBCChunk::setInt32(qint32 data) {
		d->Type = SBCChunk::int32;
		d->setData(QByteArray::fromRawData(reinterpret_cast<const char*>(&data), sizeof(data)));
	}

	qint32 SBCChunk::getInt32() const {
		if (d->Type != SBCChunk::int32) {
			return 0;
		}
		qint32 value;
		memcpy(&value, d->Data.constData(), sizeof(value));
		return value;
	}

	void SBCChunk::setUInt64(quint64 data) {
		d->Type = SBCChunk::uint64;
		d->setData(QByteArray::fromRawData(reinterpret_cast<const char*>(&data), sizeof(data)));
	}

	quint64 SBCChunk::getUInt64() const {
		if (d->Type != SBCChunk::uint64) {
			return 0;
		}
		quint64 value;
		memcpy(&value, d->Data.constData(), sizeof(value));
		return value;
	}

	void SBCChunk::setInt64(qint64 data) {
		d->Type = SBCChunk::int64;
		d->setData(QByteArray::fromRawData(reinterpret_cast<const char*>(&data), sizeof(data)));
	}

	qint64 SBCChunk::getInt64() const {
		if (d->Type != SBCChunk::int64) {
			return 0;
		}
		qint64 value;
		memcpy(&value, d->Data.constData(), sizeof(value));
		return value;
	}

	void SBCChunk::setBoolean(bool data) {
		d->Type = SBCChunk::boolean;
		d->setData(QByteArray::fromRawData(reinterpret_cast<const char*>(&data), sizeof(data)));
	}

	bool SBCChunk::getBoolean() const {
		if (d->Type != SBCChunk::boolean) {
			return false;
		}
		bool value;
		memcpy(&value, d->Data.constData(), sizeof(value));
		return value;
	}

	void SBCChunk::setFloat32(float data) {
		d->Type = SBCChunk::float32;
		d->setData(QByteArray::fromRawData(reinterpret_cast<const char*>(&data), sizeof(data)));
	}

	float SBCChunk::getFloat32() const {
		if (d->Type != SBCChunk::float32) {
			return 0.0f;
		}
		float value;
		memcpy(&value, d->Data.constData(), sizeof(value));
		return value;
	}

	void SBCChunk::setFloat64(double data) {
		d->Type = SBCChunk::float64;
		d->setData(QByteArray::fromRawData(reinterpret_cast<const char*>(&data), sizeof(data)));
	}

	double SBCChunk::getFloat64() const {
		if (d->Type != SBCChunk::float64) {
			return 0.0;
		}
		double value;
		memcpy(&value, d->Data.constData(), sizeof(value));
		return value;
	}

	void SBCChunk::setChar(const QChar& data) {
		d->Type = SBCChunk::character;
		d->setData(QString(data).toUtf8());
	}

	QChar SBCChunk::getChar() const {
		if (d->Type != SBCChunk::character) {
			return QChar();
		}
		QString str = QString::fromUtf8(d->Data);
		return str.isEmpty() ? QChar() : str.at(0);
	}

	void SBCChunk::setString(const QString& data) {
		d->Type = SBCChunk::string;
		d->setData(data.toUtf8());
	}

	QString SBCChunk::getString() const {
		if (d->Type != SBCChunk::string) {
			return QString();
		}
		return QString::fromUtf8(d->Data);
	}

	void SBCChunk::setArray(const QList<SBCChunk>& data) {
		d->Type = SBCChunk::array;
		d->setArray(data);
	}

	QList<SBCChunk> SBCChunk::getArray() const {
		if (d->Type != SBCChunk::array) {
			return QList<SBCChunk>();
		}
		return d->getArray();
	}

	void SBCChunk::setMap(const QMap<QString, SBCChunk>& data) {
		d->Type = SBCChunk::map;
		d->setMap(data);
	}

	QMap<QString, SBCChunk> SBCChunk::getMap() const {
		if (d->Type != SBCChunk::map) {
			return QMap<QString, SBCChunk>();
		}
		return d->getMap();
	}

	void SBCChunk::setBinary(const QByteArray& data) {
		d->Type = SBCChunk::binary;
		d->setData(data);
	}

	QByteArray SBCChunk::getBinary() const {
		if (d->Type != SBCChunk::binary) {
			return QByteArray();
		}
		return d->Data;
	}

	SBCChunk::DataType SBCChunk::getDataType() const {
		return d->Type;
	}

	bool SBCChunk::isDataType(SBCChunk::DataType type) const {
		return d->Type == type;
	}

	SBCChunk::HeadType SBCChunk::getHeadType() const {
		return d->Head;
	}

	bool SBCChunk::isHeadType(SBCChunk::HeadType type) const {
		return d->Head == type;
	}

	qint32 SBCChunk::getDataSize() const {
		return d->Data.size();
	}

	qint32 SBCChunk::getNameSize() const {
		return d->Name.size();
	}

	QByteArray SBCChunk::generate() {
		return d->generate();
	}

	QByteArray SBCChunk::parse(const QByteArray& data) {
		return d->parse(data);
	}
}