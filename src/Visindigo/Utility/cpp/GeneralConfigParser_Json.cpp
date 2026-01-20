#include "Utility/GeneralConfigParser.h"
#include "General/Log.h"
#include <QtCore/qstack.h>
#include <QtCore/qstring.h>
#include <QtCore/qhash.h>
#include "General/Exception.h"

/*!
	\class Visindigo::Utility::GeneralConfigParser
	\brief GeneralConfig配套的解析与序列化操作类
	\ingroup GeneralConfig
	\inmodule Visindigo
	\since Visindigo 0.13.0

	GeneralConfigParser类提供Visindigo::Utility::GeneralConfig类
	对应的解析与序列化操作。当前仅支持Json，计划内支持YAML、
	XML、VBC。

*/
namespace Visindigo::Utility {
	
	class JsonMatchingBracketCache {
	public:
		QStack<QChar> LeftBacketStack;
		QStack<qint32> LeftIndex;
		QMap<qint32, qint32> RightIndex;
		JsonMatchingBracketCache(QString content) {
			bool inString = false;
			qint32 i = 0;
			for (auto it = content.begin(); it != content.end(); it++) {
				if(*it=='\"') {
					auto jt = it == content.begin() ? content.begin() : it - 1;
					if (*jt != '\\') {
						inString = !inString;
					}
				}
				if (!inString) {
					if (*it == '{' || *it == '['){
						LeftBacketStack.push(*it);
						LeftIndex.push(i);
					}
					else if (LeftBacketStack.size()>0 && LeftBacketStack.top() == '{' && *it == '}' || LeftBacketStack.size() > 0 && LeftBacketStack.top() == '[' && *it == ']') {
						qint32 leftIndex = LeftIndex.pop();
						LeftBacketStack.pop();
						RightIndex[leftIndex] = i;
					}
				}
				i++;
			}
		}
		qint32 getRightIndex(qint32 startDelta, qint32 leftIndex) {
			if (!RightIndex.contains(startDelta + leftIndex) ){
				vgErrorF << "JsonMatchingBracketCache: No matching right bracket found for left bracket at index" << (startDelta + leftIndex);
				VI_Throw_ST(Visindigo::General::Exception::NotFound, "");
			}
			return RightIndex[startDelta + leftIndex] - startDelta;
		}
	};

	static bool _parseFromJson(JsonMatchingBracketCache* bCache, GeneralConfig* self, QStringList* nodeStack, QString* content, QString::iterator left, QString::iterator right) {
		if (self->getNodeType() == GeneralConfig::Map) {
			bool inString = false;
			bool keyGetted = false;
			bool colonGetted = false;
			qint32 contentStartIndex = 0;
			QString keyCache;
			for (auto ch = left; ch != right; ch++) {
				if (*ch == QChar('\"')) {
					auto rch = ch == left ? left : (ch - 1);
					if (*rch != QChar('\\')) {
						inString = !inString;
						if (inString) {
							contentStartIndex = ch - content->begin() + 1;
							continue; // skip the beginning quote
						}
						if (!inString) {
							if (!keyGetted) {
								keyCache = content->sliced(contentStartIndex, ch - content->begin() - contentStartIndex);
								contentStartIndex = ch - content->begin() + 1;
								keyGetted = true;
							}
							else {
								if (colonGetted) {
									QString contentCache = content->sliced(contentStartIndex, ch - content->begin() - contentStartIndex);
									self->setString(contentCache, keyCache);
									keyGetted = false;
									keyCache.clear();
									contentStartIndex = ch - content->begin() + 1;
								}
							}
						}
					}
				}
				else {
					if (!inString) {
						if (*ch == QChar(':')) {
							colonGetted = true;
							contentStartIndex = ch - content->begin() + 1;
						}
						else if (*ch == QChar(',')) {
							QString contentCache = content->sliced(contentStartIndex, ch - content->begin() - contentStartIndex);
							QString trimmedCache = contentCache.trimmed();
							if (trimmedCache.size() > 0) {
								if (trimmedCache == QString("true")) {
									self->setBool(true, keyCache);
								}
								else if (trimmedCache == QString("false")) {
									self->setBool(false, keyCache);
								}
								else if (trimmedCache == QString("null")) {
									// do nothing, keep it as None
								}
								else {
									bool ok = false;
									qint64 intValue = trimmedCache.toLongLong(&ok);
									if (ok) {
										self->setInt(intValue, keyCache);
									}
									else {
										qreal realValue = trimmedCache.toDouble(&ok);
										if (ok) {
											self->setDouble(realValue, keyCache);
										}
										else {
											vgWarningF << "GeneralConfigParser::_parseFromJson: Cannot parse value for key" << keyCache << ":" << trimmedCache;
										}
									}
								}
							}
							colonGetted = false;
							keyGetted = false;
						}
						else if (*ch == QChar('{')) {
							if (keyGetted) {
								qint32 endIndex = bCache->getRightIndex(0, ch - content->begin());
								if (endIndex != -1) {
									self->setMap(GeneralConfigMap(), keyCache);
									_parseFromJson(bCache, self->getChildNode(keyCache), nodeStack, content, ch + 1, content->begin() + endIndex - 1);
									ch = content->begin() + endIndex;
								}
								contentStartIndex = endIndex + 1;
							}
							else {
								vgWarningF << "GeneralConfigParser::_parseFromJson: Unexpected '{' without a key.";
							}
						}
						else if (*ch == QChar('[')) {
							if (keyGetted) {
								qint32 endIndex = bCache->getRightIndex(0, ch - content->begin());
								if (endIndex != -1) {
									self->setList(GeneralConfigList(), keyCache);
									_parseFromJson(bCache, self->getChildNode(keyCache), nodeStack, content, ch + 1, content->begin() + endIndex - 1);
									ch = content->begin() + endIndex;
								}
								contentStartIndex = endIndex + 1;
							}
							else {
								vgWarningF << "GeneralConfigParser::_parseFromJson: Unexpected '[' without a key.";
							}
						}
					}
				}
			}
		}
		else if (self->getNodeType() == GeneralConfig::List) {
			qint32 index = 0;
			bool inString = false;
			qint32 contentStartIndex = 0;
			for (auto ch = left; ch != right; ch++) {
				if (!inString) {
					if (*ch == QChar('{')) {
						qint32 endIndex = bCache->getRightIndex(0, ch - content->begin());
						if (endIndex != -1) {
							self->setMap(GeneralConfigMap(), QString::number(index));
							_parseFromJson(bCache, self->getChildNode(QString::number(self->getList().size() - 1)), nodeStack, content, ch + 1, content->begin() + endIndex - 1);
							ch = content->begin() + endIndex;
							contentStartIndex = endIndex + 1;
						}
					}
					else if (*ch == QChar('[')) {
						qint32 endIndex = bCache->getRightIndex(0, ch - content->begin());
						if (endIndex != -1) {
							self->setList(GeneralConfigList(), QString::number(index));
							_parseFromJson(bCache, self->getChildNode(QString::number(self->getList().size() - 1)), nodeStack, content, ch + 1, content->begin() + endIndex - 1);
							ch = content->begin() + endIndex;
							contentStartIndex = endIndex + 1;
						}
					}
					else if (*ch == QChar(',')) {
						index++;
						QString contentCache = content->sliced(contentStartIndex, ch - content->begin() - contentStartIndex);
						QString trimmedCache = contentCache.trimmed();
						if (trimmedCache.size() > 0) {
							if (trimmedCache == QString("true")) {
								self->setBool(true, QString::number(index - 1));
							}
							else if (trimmedCache == QString("false")) {
								self->setBool(false, QString::number(index - 1));
							}
							else if (trimmedCache == QString("null")) {
								// do nothing, keep it as None
							}
							else {
								bool ok = false;
								qint64 intValue = trimmedCache.toLongLong(&ok);
								if (ok) {
									self->setInt(intValue, QString::number(index - 1));
								}
								else {
									qreal realValue = trimmedCache.toDouble(&ok);
									if (ok) {
										self->setDouble(realValue, QString::number(index - 1));
									}
									else {
										vgWarningF << "GeneralConfigParser::_parseFromJson: Cannot parse value in list:" << trimmedCache;
									}
								}
							}
						}
					}
					else if (*ch == QChar('\"')) {
						auto rch = ch == left ? left : (ch - 1);
						if (*rch != QChar('\\')) {
							inString = true;
							contentStartIndex = ch - content->begin() + 1;
							continue; // skip the beginning quote
						}
					}
				}
				else {
					if (*ch == QChar('\"')) {
						auto rch = ch == left ? left : (ch - 1);
						if (*rch != QChar('\\')) {
							inString = false;
							QString contentCache = content->sliced(contentStartIndex, ch - content->begin() - contentStartIndex);
							self->setString(contentCache, QString::number(index));
							contentStartIndex = ch - content->begin() + 1;
						}
					}
				}
			}
		}
		return false;
	}

	/*!
		\since Visindigo 0.13.0
		\a jsonString 要解析的字符串
		\a ok 成功状态值
		从Json字符串解析到GeneralConfig
	*/
	GeneralConfig* GeneralConfigParser::parseFromJson(const QString& jsonString, bool* ok) {
		GeneralConfig* config = new GeneralConfig();
		QString content = jsonString.trimmed();
		QStringList nodeStack;
		JsonMatchingBracketCache bCache = JsonMatchingBracketCache(content);
		if (content.startsWith('{') && content.endsWith('}')) {
			config->setMap(GeneralConfigMap());
			auto it = content.begin();
			auto ed = content.end();
			it++;
			ed--;
			_parseFromJson(&bCache, config, &nodeStack, &content, it, ed);
		}
		else if (content.startsWith('[') && content.endsWith(']')) {
			config->setList(GeneralConfigList());
			auto it = content.begin();
			auto ed = content.end();
			it++;
			ed--;
			_parseFromJson(&bCache, config, &nodeStack, &content, it, ed);
		}
		else {
			vgDebugF << "GeneralConfigParser::parseFromJson: Invalid JSON string format.";
			if (ok) {
				*ok = false;
			}
			return nullptr;
		}
		if (ok) {
			*ok = true;
		}
		return config;
	}

	static QString _serializeToJson(GeneralConfig* self, GeneralConfig::StringFormat format, qint32 formatIndent, qint32 depth) {
		QString start, end;
		if (self->getNodeType() == GeneralConfig::Map) {
			start = "{";
			end = "}";
			QStringList keyValues;
			auto mapPtr = self->getMap();
			for (auto key : mapPtr.keys()) {
				QString valueStr = _serializeToJson(self->getChildNode(key), format, formatIndent, depth + 1);
				if (format == GeneralConfig::StringFormat::Formatted) {
					keyValues.append(QString("\n") % QString(" ").repeated((depth + 1) * formatIndent) % "\"" % key % "\": " % valueStr);
				}
				else {
					keyValues.append("\"" % key % "\":" % valueStr);
				}
			}
			QString joinedValues = keyValues.join(format == GeneralConfig::StringFormat::Formatted ? "," : ",");
			if (format == GeneralConfig::StringFormat::Formatted && !keyValues.isEmpty()) {
				joinedValues += QString("\n") % QString(" ").repeated(depth * formatIndent);
			}
			return start % joinedValues % end;
		}
		else if (self->getNodeType() == GeneralConfig::List) {
			start = "[";
			end = "]";
			QStringList values;
			auto listPtr = self->getList();
			for (auto item : listPtr) {
				QString valueStr = _serializeToJson(item, format, formatIndent, depth + 1);
				if (format == GeneralConfig::StringFormat::Formatted) {
					values.append(QString("\n") % QString(" ").repeated((depth + 1) * formatIndent) % valueStr);
				}
				else {
					values.append(valueStr);
				}
			}
			QString joinedValues = values.join(format == GeneralConfig::StringFormat::Formatted ? "," : ",");
			if (format == GeneralConfig::StringFormat::Formatted && !values.isEmpty()) {
				joinedValues += QString("\n") % QString(" ").repeated(depth * formatIndent);
			}
			return start % joinedValues % end;
		}
		else if (self->getNodeType() == GeneralConfig::String) {
			return QString("\"") % self->getString().replace("\"", "\\\"") % "\"";
		}
		else if (self->getNodeType() == GeneralConfig::Integer) {
			return QString::number(self->getInt());
		}
		else if (self->getNodeType() == GeneralConfig::Real) {
			return QString::number(self->getReal());
		}
		else if (self->getNodeType() == GeneralConfig::Bool) {
			return self->getBool() ? "true" : "false";
		}
		else if (self->getNodeType() == GeneralConfig::None) {
			return "null";
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a config 被解析的对象
		\a format 解析格式
		\a formatIndent 带格式解析时的缩进等级

		将GeneralConfig解析到Json字符串。
	*/
	QString GeneralConfigParser::serializeToJson(GeneralConfig* config, GeneralConfig::StringFormat format, qint32 formatIndent) {
		return _serializeToJson(config, format, formatIndent, 0);
	}

	struct JsonParseData {
		GeneralConfig* currentNode = nullptr;
		const char* startPtr = nullptr;
		qint32 length = 0;
	};
	enum JsonTokenType {
		LeftBrace = '{',
		RightBrace = '}',
		LeftBracket = '[',
		RightBracket = ']',
		Colon = ':',
		Comma = ',',
		Quote = '\"',
	};
	enum JsonValueType {
		Unknown, String, Integer, Real, Bool, Null, Map, List
	};
	GeneralConfig* GeneralConfigParser::parseFromJson_2(const QString& json, bool* ok) {
		QByteArray content = json.toUtf8();
		const char* dataPtr = content.constData();
		qint32 length = content.size();
		QStack<GeneralConfig*> parseStack;
		QString keyCache = "";
		const char* colonPtr = nullptr;
		bool getKey = false;
		JsonValueType type = JsonValueType::Unknown;
		while (*dataPtr) {
			switch (*dataPtr) {
			case JsonTokenType::Quote:
			{
				auto front = dataPtr == content.constData() ? content.constData() : dataPtr - 1;
				if (*front != '\\') {
					// a quote not escaped
					auto strStart = ++dataPtr;
					while (*dataPtr) {
						if (*dataPtr == JsonTokenType::Quote) {
							auto rch = dataPtr == content.constData() ? content.constData() : dataPtr - 1;
							if (*rch != '\\') {
								// end quote found
								break;
							}
						}
						dataPtr++;
					}
					if (!getKey) {
						keyCache = QString::fromUtf8(strStart, dataPtr - strStart);
					}
					else {
						parseStack.top()->setString(QString::fromUtf8(strStart, dataPtr - strStart), keyCache);
						type = JsonValueType::String;
					}
				}
			}
			break;
			case JsonTokenType::Colon:
				getKey = true;
				colonPtr = dataPtr + 1;
			break;
			case JsonTokenType::Comma:
			{
				if (type != JsonValueType::String && type != JsonValueType::Map && type != JsonValueType::List) {
					// value is bool, null, int, real
					auto front = colonPtr;
					while (*front) {
						if (*front != ' ' && *front != '\n' && *front != '\r' && *front != '\t') {
							break;
						}
						front++;
					}
					auto end = dataPtr - 1;
					while (*end) {
						if (*end != ' ' && *end != '\n' && *end != '\r' && *end != '\t') {
							break;
						}
						end--;
					}
					int contentLength = end - front + 1;
					vgDebug << "GeneralConfigParser::parseFromJson_2: Parsing value for key" << keyCache << ":" << QString::fromUtf8(front, contentLength);
					switch (*front) {
					case 't':
						if (contentLength >= 4 && 
							*(front + 1) == 'r' && 
							*(front + 2) == 'u' && 
							*(front + 3) == 'e') {
							parseStack.top()->setBool(true, keyCache);
							type = JsonValueType::Bool;
						}
						break;
					case 'f':
						if (contentLength >= 5 && 
							*(front + 1) == 'a' && 
							*(front + 2) == 'l' && 
							*(front + 3) == 's' && 
							*(front + 4) == 'e') {
							parseStack.top()->setBool(false, keyCache);
							type = JsonValueType::Bool;
						}
						break;
					case 'n':
						if (contentLength >= 4 && 
							*(front + 1) == 'u' && 
							*(front + 2) == 'l' && 
							*(front + 3) == 'l') {
							// do nothing, keep it as None
							type = JsonValueType::Null;
						}
						break;
					default:
					{
						for (auto it = front; it != end; it++) {
							if ((*it < '0' || *it > '9') && *it != '-' && *it != '+' && *it != '.' && *it != 'e' && *it != 'E') {
								// not a number
								vgWarningF << "GeneralConfigParser::_parseFromJson_2: Cannot parse value for key" << keyCache << ":" << QString::fromUtf8(front, contentLength);
								type = JsonValueType::Unknown;
								if (ok) {
									*ok = false;
								}
								break;
							}
							else if (*it == '.' || *it == 'e' || *it == 'E') {
								// real number
								qreal realValue = QString::fromUtf8(front, contentLength).toDouble();
								parseStack.top()->setDouble(realValue, keyCache);
								type = JsonValueType::Real;
								break;
							}
							else {
								qint64 intValue = QString::fromUtf8(front, contentLength).toLongLong();
								parseStack.top()->setInt(intValue, keyCache);
								type = JsonValueType::Integer;
								break;
							}
						}
					}
					break;
					}
				}
				if (parseStack.top()->getNodeType() == GeneralConfig::List) {
					getKey = true;
					keyCache = QString::number(parseStack.top()->getList().size());
				}
				else {
					getKey = false;
				}
				type = JsonValueType::Unknown;
			}
			break;
			case JsonTokenType::LeftBrace:
			{
				if (parseStack.isEmpty()) {
					parseStack.push(new GeneralConfig(GeneralConfigMap()));
				}
				else {
					parseStack.top()->setMap(GeneralConfigMap(), keyCache);
					parseStack.push(parseStack.top()->getChildNode(keyCache));
				}
				keyCache = "";
				getKey = false;
				type = JsonValueType::Unknown;
			}
			break;
			case JsonTokenType::RightBrace:
			{
				if (parseStack.size() > 1) {
					parseStack.pop();
					type = JsonValueType::Map;
				}
				else {
					goto endParse;
				}
			}
			break;
			case JsonTokenType::LeftBracket:
			{
				if (parseStack.isEmpty()) {
					parseStack.push(new GeneralConfig(GeneralConfigList()));
				}
				else {
					parseStack.top()->setList(GeneralConfigList(), keyCache);
					parseStack.push(parseStack.top()->getChildNode(keyCache));
				}
				keyCache = "0";
				getKey = true;
				type = JsonValueType::Unknown;
			}
			break;
			case JsonTokenType::RightBracket:
			{
				if (parseStack.size() > 1) {
					parseStack.pop();
					type = JsonValueType::List;
				}
				else {
					goto endParse;
				}
			}
			default:
				break;
				//sth
			}
			dataPtr++;
		}
endParse:
		if (parseStack.isEmpty()) {
			vgDebugF << "GeneralConfigParser::parseFromJson_2: Invalid JSON string format.";
			if (ok) {
				*ok = false;
			}
			return nullptr;
		}else if (parseStack.size() > 1) {
			vgDebugF << "GeneralConfigParser::parseFromJson_2: Incomplete JSON string format.";
			if (ok) {
				*ok = false;
			}
			return nullptr;
		}
		else {
			if (ok) {
				*ok = true;
			}
			return parseStack.first();
		}
	}
}