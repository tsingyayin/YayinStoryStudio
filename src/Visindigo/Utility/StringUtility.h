#ifndef Visindigo_Utility_StringUtility_h
#define Visindigo_Utility_StringUtility_h
#include <QtCore/qstring.h>
#include <QtCore/qnamespace.h>
#include "VICompileMacro.h"
namespace Visindigo::Utility {
	class VisindigoAPI StringUtility {
	public:
		struct Statistic {
			qint64 WordCount = 0;
			qint64 CharCountExcludeWhitespace = 0;
			qint64 CharCountIncludeWhitespace = 0;
			qint64 ParagraphCount = 0;
			qint64 NonCJKVCharCount = 0;
			qint64 CJKVCharCount = 0;
		};
	public:
		static qint32 getLevensteinDistance(const QString& s1, const QString& s2);
		static QStringList getSimilarStrings(const QString& target, const QStringList& candidates, int maxDistance = 3);
		static QStringList getStartWith(const QString& str, const QStringList& candidates, bool caseSensitive = true);
		static QStringList deduplicate(const QStringList& list);
		static bool isAllBlank(const QString& str);
		static QString autoElide(const QString& str, qint32 maxLength, const QString& elideStr = "...");
		static Statistic getStatistic(const QString& str);
		static bool isCJKV(uint codePoint);
	};
}
#endif // Visindigo_Utility_StringUtility_h	