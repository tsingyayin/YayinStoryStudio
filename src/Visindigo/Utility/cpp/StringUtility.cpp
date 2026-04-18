#include "../StringUtility.h"
#include <QtCore/qlist.h>
namespace Visindigo::Utility {
	/*!
		\class Visindigo::Utility::StringUtility
		\brief 提供一些字符串相关的实用函数。
		\since Visindigo 0.14.0
		\inmodule VisindigoUtility

		这个类提供一些和字符串相关的使用函数，它们都是静态函数。
	*/

	/*!
		\since Visindigo 0.14.0
		返回两个字符串的编辑距离。编辑距离是指将一个字符串转换成另一个字符串所需要的最少编辑操作次数，
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
}