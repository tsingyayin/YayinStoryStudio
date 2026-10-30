#ifndef YSSCore_Editor_private_CodeDiff_p_h
#define YSSCore_Editor_private_CodeDiff_p_h
#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>
#include <QtCore/qstringlist.h>
#include <QtGui/qfont.h>
#include <QtWidgets/qtextedit.h>
#include <tuple>

class QLabel;
class QPushButton;
class QSplitter;
class QVBoxLayout;
class QWidget;

namespace YSSCore::General {
	class YSSURI;
}

namespace YSSCore::Editor {
	class CodeDiff;

	// (oldStart, oldSize, newStart, newSize)，下标均从 0 开始。
	using CodeDiffRange = std::tuple<qint32, qint32, qint32, qint32>;

	/*!
		\class YSSCore::Editor::CodeDiffProvider
		\since YSS 0.18.0
		\brief 差异分析提供者。

		给定两侧的文本行，返回按 oldStart 升序排列的相异范围。
		每一项依次为：旧侧起始行、旧侧行数、新侧起始行、新侧行数。
		行数为 0 表示该侧在这一点上没有内容（纯插入或纯删除）。
	*/
	class CodeDiffProvider {
	public:
		virtual ~CodeDiffProvider();
		virtual QList<CodeDiffRange> compute(const QStringList& oldLines, const QStringList& newLines) = 0;
	};

	/*!
		\class YSSCore::Editor::BuiltinCodeDiffProvider
		\since YSS 0.18.0
		\brief 默认的差异分析器，使用 Myers 最短编辑脚本算法。

		差异数过大（超过内部上限）时退化为"整篇替换"，以避免内存与耗时失控。
	*/
	class BuiltinCodeDiffProvider final : public CodeDiffProvider {
	public:
		QList<CodeDiffRange> compute(const QStringList& oldLines, const QStringList& newLines) override;
	};

	// 将来提供基于 Git 的分析器时，实现同一个 CodeDiffProvider 接口即可：
	// class GitCodeDiffProvider final : public CodeDiffProvider { ... };

	/*!
		\class YSSCore::Editor::DiffRow
		\brief 比较器实际显示的一行：左右两侧各一个格子。

		OldNumber/NewNumber 为 -1 表示该侧在这一行没有内容（显示为空行占位），
		这样两侧的块数严格相等，滚动位置天然一一对应。
	*/
	struct DiffRow {
		qint32 OldNumber = -1;
		qint32 NewNumber = -1;
		QString OldText;
		QString NewText;
		// 属于第几处差异（对应 getDiffRanges 的下标），-1 表示上下文行。
		qint32 RangeIndex = -1;
	};

	/*!
		\class YSSCore::Editor::DiffPane
		\brief 比较器的单侧文本面板：只读、不折行，并在朝向中缝的一侧显示行号。
	*/
	class DiffPane;

	// 行号栏：放在面板朝向中缝的留白里，独立接收绘制事件。
	// QPlainTextEdit 的 paintEvent 实际处理的是 viewport 的绘制，在父控件留白上绘制会被裁掉。
	class LineNumberBar : public QWidget {
	public:
		explicit LineNumberBar(DiffPane* owner);
		void setSeparatorOnRight(bool onRight);
	protected:
		void paintEvent(QPaintEvent* event) override;
	private:
		DiffPane* Owner = nullptr;
		bool SeparatorOnRight = false;
	};

	class DiffPane : public QTextEdit {
	public:
		enum class LineNumberSide { Left, Right };
		struct VisibleLine {
			qint32 Number = -1;
			qint32 Top = 0;
			qint32 Height = 0;
		};
	public:
		explicit DiffPane(QWidget* parent = nullptr);
		void setLineNumbers(const QList<qint32>& numbers);
		void setLineNumberSide(LineNumberSide side);
		LineNumberSide getLineNumberSide() const;
		void setCodeFont(const QFont& font);
		QFont getCodeFont() const;
		const QFont& getNumberFont() const;
		QList<VisibleLine> getVisibleLines();
	protected:
		void paintEvent(QPaintEvent* event) override;
		void resizeEvent(QResizeEvent* event) override;
	private:
		void updateMargin();
		void updateBarGeometry();
		qint32 numberColumnWidth() const;
	protected:
		QList<qint32> LineNumbers;
		LineNumberSide Side = LineNumberSide::Left;
		QFont CodeFont;
		QFont NumberFont;
		qint32 MarginWidth = 0;
		LineNumberBar* Bar = nullptr;
	};

	class CodeDiffPrivate {
		friend class CodeDiff;
	public:
		explicit CodeDiffPrivate(CodeDiff* q);
		~CodeDiffPrivate();
	protected:
		void buildUi();
		void startLoad(const YSSCore::General::YSSURI& oldUri, const YSSCore::General::YSSURI& newUri);
		// 读取一侧的文本；两侧读完后由 finishLoad 接手。
		void loadOne(const YSSCore::General::YSSURI& uri, bool isOldSide);
		void finishLoad();
		void setSource(const QStringList& oldLines, const QStringList& newLines);
		void rebuildRows();
		void applyRowsToPanes();
		void updateHighlights();
		void updateConflictBar();
		void focusCurrentRange();
		void choose(bool takeNew);
		void resetConflictState();
		void setCodeFont(const QFont& font);
		QStringList mergedLines() const;
		qint32 firstRowOfRange(qint32 rangeIndex) const;
	protected:
		CodeDiff* q;
		QVBoxLayout* Layout = nullptr;
		QSplitter* Splitter = nullptr;
		DiffPane* OldView = nullptr;
		DiffPane* NewView = nullptr;
		QWidget* ConflictBar = nullptr;
		QLabel* ConflictLabel = nullptr;
		QPushButton* TakeOldButton = nullptr;
		QPushButton* TakeNewButton = nullptr;

		CodeDiffProvider* Provider = nullptr;
		QFont CodeFont;
		QStringList OldLines;
		QStringList NewLines;
		QList<CodeDiffRange> Ranges;
		QList<DiffRow> Rows;
		QList<bool> Selections;   // true 取右侧，false 取左侧
		QList<bool> Resolved;     // 该处差异是否已经被询问过
		bool ConflictMode = false;
		bool SyncingScroll = false;
		qint32 CurrentIndex = -1;

		// 虚拟文件异步读取状态
		qint32 PendingReads = 0;
		bool ReadFailed = false;
		QString ReadError;
		QByteArray OldData;
		QByteArray NewData;
	};
}
#endif // YSSCore_Editor_private_CodeDiff_p_h
