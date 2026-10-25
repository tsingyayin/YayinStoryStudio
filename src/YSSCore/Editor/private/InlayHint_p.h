#ifndef YSSCore_Editor_private_InlayHint_p_h
#define YSSCore_Editor_private_InlayHint_p_h
#include <QtGui/qfont.h>
#include "Editor/CustomDocumentData.h"
#include "Editor/InlayHint.h"
namespace YSSCore::__Private__ {
	/*!
		挂在文本块上的内联提示数据。与 YSSCore::__Private__::DocumentMessageList 同理：
		由框架懒创建并交给文本块持有，块被删除、合并时随块一起释放，行号变化时自动跟随。
	*/
	class InlayHintList :public YSSCore::Editor::ICustomDocumentData {
	public:
		QList<YSSCore::Editor::InlayHint> Hints;
	};

	/*!
		\a baseFont 正文使用的字体。
		\a fontScale 提示字号相对正文字号的缩放比例。

		\return 内联提示绘制时使用的字体。
	*/
	QFont inlayHintFont(const QFont& baseFont, qreal fontScale);

	/*!
		\a baseFont 正文使用的字体。

		\return 图片提示占位正方形的边长。图片提示的宽高都取这个值。
	*/
	qreal inlayHintImageSide(const QFont& baseFont);

	/*!
		\a hint 内联提示。
		\a providerID 可选输出参数，用于取得 Multi 中指定的 Provider ID。

		\return Multi 是否符合 VirtualFilePath 格式，且对应的 Provider 已在
		InlayHintManager 中注册。
	*/
	bool inlayHintUsesProvider(const YSSCore::Editor::InlayHint& hint, QString* providerID = nullptr);

	/*!
		\a hint 内联提示。
		\a baseFont 正文使用的字体。
		\a hintFont 内联提示绘制时使用的字体，用 inlayHintFont 取得。

		\return 这条提示需要在文本流中占用的空隙宽度。图片提示为行高正方形，
		文字提示为提示文字宽度与左右内边距之和。

		注入字距与绘制提示必须使用同一个值，否则提示会和正文重叠。
	*/
	qreal inlayHintGapWidth(const YSSCore::Editor::InlayHint& hint, const QFont& baseFont, const QFont& hintFont);
}
#endif // YSSCore_Editor_private_InlayHint_p_h
