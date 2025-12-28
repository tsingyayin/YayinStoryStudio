#include "Utility/GeneralConfigParser.h"
#include "General/Log.h"
#include <QtCore/qstack.h>
#include <QtCore/qstring.h>
#include <QtCore/qhash.h>
#include "General/Exception.h"
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

	static bool _parseFromJson(JsonMatchingBracketCache* bCache, GeneralConfig* self, QStringList* nodeStack, QString content, qint32 startDelta) {
		if (self->getNodeType() == GeneralConfig::Map) {
			bool inString = false;
			bool keyGetted = false;
			bool colonGetted = false;
			qint32 contentStartIndex = 0;
			QString keyCache;
			for (auto ch = content.begin(); ch != content.end(); ch++) {
				if (*ch == QChar('\"')) {
					auto rch = ch == content.begin() ? QChar() : *(ch - 1);
					if (rch != QChar('\\')) {
						inString = !inString;
						if (inString) {
							contentStartIndex = ch - content.begin() + 1;
							continue; // skip the beginning quote
						}
						if (!inString) {
							if (!keyGetted) {
								keyCache = content.mid(contentStartIndex, ch - content.begin() - contentStartIndex);
								contentStartIndex = ch - content.begin() + 1;
								keyGetted = true;
							}
							else {
								if (colonGetted) {
									QString contentCache = content.mid(contentStartIndex, ch - content.begin() - contentStartIndex);
									self->setString(contentCache, keyCache);
									keyGetted = false;
									keyCache.clear();
									contentStartIndex = ch - content.begin() + 1;
								}
							}
						}
					}
				}
				else {
					if (!inString) {
						if (*ch == QChar(':')) {
							colonGetted = true;
							contentStartIndex = ch - content.begin() + 1;
						}
						else if (*ch == QChar(',')) {
							QString contentCache = content.mid(contentStartIndex, ch - content.begin() - contentStartIndex);
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
								qint32 endIndex = bCache->getRightIndex(startDelta, ch - content.begin());
								if (endIndex != -1) {
									self->setMap(GeneralConfigMap(), keyCache);
									QString subContent = content.mid(ch - content.begin()+1, endIndex - (ch - content.begin() + 1));
									_parseFromJson(bCache, self->getChildNode(keyCache), nodeStack, subContent, startDelta + (ch - content.begin() + 1));
									ch += endIndex - (ch - content.begin());
								}
								contentStartIndex = endIndex + 1;
							}
							else {
								vgWarningF << "GeneralConfigParser::_parseFromJson: Unexpected '{' without a key.";
							}
						}
						else if (*ch == QChar('[')) {
							if (keyGetted) {
								qint32 endIndex = bCache->getRightIndex(startDelta, ch - content.begin());
								if (endIndex != -1) {
									self->setList(GeneralConfigList(), keyCache);
									QString subContent = content.mid(ch - content.begin()+1, endIndex - (ch - content.begin() + 1));
									_parseFromJson(bCache, self->getChildNode(keyCache), nodeStack, subContent, startDelta + (ch - content.begin() + 1));
									ch += endIndex - (ch - content.begin());
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
			for (auto ch = content.begin(); ch != content.end(); ch++) {
				if (!inString) {
					if (*ch == QChar('{')) {
						qint32 endIndex = bCache->getRightIndex(startDelta, ch - content.begin());
						if (endIndex != -1) {
							self->setMap(GeneralConfigMap(), QString::number(index));
							QString subContent = content.mid(ch - content.begin() + 1, endIndex - (ch - content.begin()+1));
							_parseFromJson(bCache, self->getChildNode(QString::number(self->getList().size() - 1)), nodeStack, subContent, startDelta + (ch - content.begin() + 1));
							ch += endIndex - (ch - content.begin());
							contentStartIndex = endIndex + 1;
						}
					}
					else if (*ch == QChar('[')) {
						qint32 endIndex = bCache->getRightIndex(startDelta, ch - content.begin());
						if (endIndex != -1) {
							self->setList(GeneralConfigList(), QString::number(index));
							QString subContent = content.mid(ch - content.begin() + 1, endIndex - (ch - content.begin() +1));
							_parseFromJson(bCache, self->getChildNode(QString::number(self->getList().size() - 1)), nodeStack, subContent, startDelta + (ch - content.begin() + 1));
							ch += endIndex - (ch - content.begin());
							contentStartIndex = endIndex + 1;
						}
					}
					else if (*ch == QChar(',')) {
						index++;
						QString contentCache = content.mid(contentStartIndex, ch - content.begin() - contentStartIndex);
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
						auto rch = ch == content.begin() ? QChar() : *(ch - 1);
						if (rch != QChar('\\')) {
							inString = true;
							contentStartIndex = ch - content.begin() + 1;
							continue; // skip the beginning quote
						}
					}
				}
				else {
					if (*ch == QChar('\"')) {
						auto rch = ch == content.begin() ? QChar() : *(ch - 1);
						if (rch != QChar('\\')) {
							inString = false;
							QString contentCache = content.mid(contentStartIndex, ch - content.begin() - contentStartIndex);
							self->setString(contentCache, QString::number(index));
							contentStartIndex = ch - content.begin() + 1;
						}
					}
				}
			}
		}
		return false;
	}

	GeneralConfig* GeneralConfigParser::parseFromJson(const QString& jsonString, bool* ok) {
		GeneralConfig* config = new GeneralConfig();
		QString content = jsonString.trimmed();
		QStringList nodeStack;
		JsonMatchingBracketCache bCache = JsonMatchingBracketCache(content);
		if (content.startsWith('{') && content.endsWith('}')) {
			config->setMap(GeneralConfigMap());
			_parseFromJson(&bCache, config, &nodeStack, content.mid(1, content.size() - 2), 1);
		}
		else if (content.startsWith('[') && content.endsWith(']')) {
			config->setList(GeneralConfigList());
			_parseFromJson(&bCache, config, &nodeStack, content.mid(1, content.size() - 2), 1);
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

	QString GeneralConfigParser::serializeToJson(GeneralConfig* config, GeneralConfig::StringFormat format, qint32 formatIndent) {
		return _serializeToJson(config, format, formatIndent, 0);
	}

	GeneralConfig GeneralConfigParser::parseFromYaml(const QString& yamlString, bool* ok) {
		if (ok) {
			*ok = false;
		}
		return GeneralConfig();
	}

	QString GeneralConfigParser::serializeToYaml(const GeneralConfig& config, GeneralConfig::StringFormat format, qint32 formatIndent) {
		return QString();
	}
}