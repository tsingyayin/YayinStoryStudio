#include <QtCore/qlist.h>
#include <QtCore/qset>
#include "Utility/StringUtility.h"

namespace Visindigo::Utility {
	/*!
		\class Visindigo::Utility::StringUtility
		\brief 提供一些字符串相关的实用函数.
		\since Visindigo 0.14.0
		\inmodule VisindigoUtility

		这个类提供一些和字符串相关的使用函数，它们都是静态函数。
	*/

	/*!
		\since Visindigo 0.14.0
		\a s1 目标字符串
		\a s2 候选字符串

		return 两个字符串的编辑距离。编辑距离是指将一个字符串转换成另一个字符串所需要的最少编辑操作次数，
		编辑操作包括插入一个字符、删除一个字符和替换一个字符。

		一般而言，如果编辑距离不小于最短字符串的长度，可以认为这两个字符串完全不同。
		如果将此函数用于纠错，可以考虑将编辑距离为1或2的字符串视为可能的纠错候选项。
	*/
	qint32 StringUtility::getLevensteinDistance(const QString& s1, const QString& s2) {
		int len1 = s1.length();
		int len2 = s2.length();
		QList<QList<int>> dp(len1 + 1, QList<int>(len2 + 1));
		for (int i = 0; i <= len1; i++) {
			dp[i][0] = i;
		}
		for (int j = 0; j <= len2; j++) {
			dp[0][j] = j;
		}
		for (int i = 1; i <= len1; i++) {
			for (int j = 1; j <= len2; j++) {
				if (s1[i - 1] == s2[j - 1]) {
					dp[i][j] = dp[i - 1][j - 1];
				}
				else {
					dp[i][j] = qMin(qMin(dp[i - 1][j] + 1, dp[i][j - 1] + 1), dp[i - 1][j - 1] + 1);
				}
			}
		}
		return dp[len1][len2];
	}

	/*!
		\since Visindigo 0.14.0
		\a target 目标字符串
		\a candidates 候选字符串列表
		\a maxDistance 最大编辑距离

		return 一个字符串列表，包含所有与目标字符串的编辑距离不大于maxDistance的候选字符串。
	*/
	QStringList StringUtility::getSimilarStrings(const QString& target, const QStringList& candidates, int maxDistance) {
		QStringList result;
		for (const QString& candidate : candidates) {
			if (getLevensteinDistance(target, candidate) <= maxDistance) {
				result.append(candidate);
			}
		}
		return result;
	}

	/*!
		\since Visindigo 0.14.0
		\a str 输入字符串
		\a candidates 候选字符串列表
		\a caseSensitive 是否区分大小写

		return 一个字符串列表，包含所有以指定字符串开头的候选字符串。可以选择是否区分大小写。
	*/
	QStringList StringUtility::getStartWith(const QString& str, const QStringList& candidates, bool caseSensitive) {
		QStringList result;
		for (const QString& candidate : candidates) {
			if (candidate.startsWith(str, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive)) {
				result.append(candidate);
			}
		}
		return result;
	}

	/*!
		\since Visindigo 0.14.0
		\a list 输入字符串列表

		return 一个字符串列表，包含输入列表中所有不重复的字符串。重复的字符串只保留第一次出现的。
	*/
	QStringList StringUtility::deduplicate(const QStringList& list) {
		if (list.isEmpty()) {
			return list;
		}
		QStringList result;
		QSet<QString> seen;
		for (const QString& str : list) {
			if (!seen.contains(str)) {
				result.append(str);
				seen.insert(str);
			}
		}
		return result;
	}

	/*!
		\since Visindigo 0.15.0
		\a str 输入字符串

		return 一个字符串是否全部由空白字符组成。空白字符包括空格、制表符、换行符等。
	*/
	bool StringUtility::isAllBlank(const QString& str) {
		for (QChar c : str) {
			if (!c.isSpace()) {
				return false;
			}
		}
		return true;
	}

	/*!
		\since Visindigo 0.16.0
		\a str 输入字符串
		\a maxLength 最大长度
		\a elideStr 省略符字符串，默认为"..."。

		return 如果输入字符串的长度超过最大长度，则返回一个被截断并添加省略符的字符串；否则返回原字符串。
	*/
	QString StringUtility::autoElide(const QString& str, qint32 maxLength, const QString& elideStr) {
		if (str.length() <= maxLength){
			return str;
		}
		qint32 halfLength = (maxLength - elideStr.length()) / 2;
		return str.left(halfLength) + elideStr + str.right(halfLength);
	}

	
	/*!
		\since Visindigo 0.16.0
		\a codePoint 一个 Unicode 码点

		return 该码点是否属于 CJKV （中日韩越统一表意文字）相关字符，包括 CJK 统一表意文字及其扩展区、
		平假名、片假名、谚文、注音、CJK 符号与标点、全角/半角形式等。
	*/
	bool StringUtility::isCJKV(uint codePoint) {
		if (codePoint < 0x2E80) {
			return false;
		}
		if (codePoint <= 0x303F) {                 // CJK 部首 / 康熙部首 / 表意描述 / CJK 符号和标点
			return true;
		}
		if (codePoint >= 0x3040 && codePoint <= 0x30FF) {   // 平假名、片假名
			return true;
		}
		if (codePoint >= 0x3100 && codePoint <= 0x33FF) {   // 注音、谚文兼容 Jamo、CJK 笔顺、CJK 兼容等
			return true;
		}
		if (codePoint >= 0x3400 && codePoint <= 0x4DBF) {   // CJK 统一表意文字扩展 A
			return true;
		}
		if (codePoint >= 0x4E00 && codePoint <= 0x9FFF) {   // CJK 统一表意文字
			return true;
		}
		if (codePoint >= 0xAC00 && codePoint <= 0xD7AF) {   // 谚文音节
			return true;
		}
		if (codePoint >= 0xF900 && codePoint <= 0xFAFF) {   // CJK 兼容表意文字
			return true;
		}
		if (codePoint >= 0xFF00 && codePoint <= 0xFFEF) {   // 全角 / 半角形式（含全角假名与全角拉丁字母）
			return true;
		}
		if (codePoint >= 0x20000 && codePoint <= 0x2FA1F) { // CJK 统一表意文字扩展 B~F 及兼容补充
			return true;
		}
		if (codePoint >= 0x30000 && codePoint <= 0x323AF) { // CJK 统一表意文字扩展 G / H
			return true;
		}
		return false;
	}

	/*!
		\since Visindigo 0.16.0
		\a str 输入字符串

		return 一个包含字数、字符数、段落数等信息的统计结果。

		统计规则：
		- WordCount：每个 CJKV 字符计为一个字，连续的由非 CJKV、非空白、非标点字符组成的片段计为一个词，
		  非 CJKV 标点也会作为词边界；
		- CharCountIncludeWhitespace：按 Unicode 码点计数的总字符数（包含空白）；
		- CharCountExcludeWhitespace：不含空白字符的字符数；
		- CJKVCharCount / NonCJKVCharCount：中、日、韩、越（CJKV）相关字符的个数；
		- ParagraphCount：以换行符分隔的非空段落个数。
	*/
	StringUtility::Statistic StringUtility::getStatistic(const QString& str) {
		Statistic result;

		bool inWord = false;          // 是否正处在一段连续的"非 CJKV 词"中
		bool lineHasContent = false;  // 当前段落是否含有非空白字符

		for (int i = 0; i < str.size(); ) {
			const ushort first = str[i].unicode();
			uint codePoint = first;
			if (QChar::isHighSurrogate(first)
				&& i + 1 < str.size()
				&& QChar::isLowSurrogate(str[i + 1].unicode())) {
				codePoint = QChar::surrogateToUcs4(str[i], str[i + 1]);
				i += 2;
			}
			else {
				i += 1;
			}

			result.CharCountIncludeWhitespace++;

			if (codePoint == '\n') {
				if (lineHasContent) {
					result.ParagraphCount++;
					lineHasContent = false;
				}
				inWord = false;
				continue;
			}

			const bool isSpace = QChar(codePoint).isSpace();
			const bool isPunct = QChar(codePoint).isPunct();
			if (!isSpace) {
				result.CharCountExcludeWhitespace++;
				lineHasContent = true;
			}

			if (isCJKV(codePoint)) {
				result.CJKVCharCount++;
				result.WordCount++;       // 每个 CJKV 字符计为一个字
				inWord = false;
			}
			else if (!isSpace) {
				result.NonCJKVCharCount++;
				if (isPunct) {
					inWord = false;       // 非 CJKV 标点作为词边界，不单独成词
				}
				else if (!inWord) {
					result.WordCount++;   // 一段连续的西方文字计为一个词
					inWord = true;
				}
			}
			else {
				inWord = false;           // 空白字符作为词边界
			}
		}

		if (lineHasContent) {
			result.ParagraphCount++;
		}

		return result;
	}
}