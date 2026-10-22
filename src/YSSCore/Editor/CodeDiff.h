#ifndef YSSCore_Editor_CodeDiff_h
#define YSSCore_Editor_CodeDiff_h
#include "Editor/FileEditWidget.h"
#include <QtCore/qlist.h>
#include <QtCore/qstringlist.h>
#include <tuple>

// 比较器虚拟文件的扩展名，同时也是虚拟文件服务器的 ID。
#define YSSVirtualFile_CodeDiff "YSS.MainEditor.CodeDiff"

namespace YSSCore::Editor {
	class CodeDiffPrivate;

	/*!
		\class YSSCore::Editor::CodeDiff
		\since YSS 0.18.0
		\inmodule YSSCore
		\ingroup FileService
		\brief 以只读方式展示两份文本行的差异，并可逐个差异询问取舍。

		\section1 两种工作状态

		\list
		\li 普通状态：左右并列显示两份文本，并把每一处差异以行背景标出。
		\li 取舍状态（\l{setConflictModeEnable(true)}）：在普通状态之上显示取舍栏，
			按 \l{getDiffRanges()} 的顺序逐处询问"取左侧还是取右侧"，
			选择结果由 \l{getConflictSelection()} 取回，合并结果由 \l{getConflictMerged()} 取回。
		\endlist

		\section1 虚拟文件

		调用 \l{FileEditWidget::openFile} 打开形如
		\c{@YSS.MainEditor.CodeDiff!文件名?{"old_uri":"...","new_uri":"..."}}
		的虚拟文件时，会自动异步读取两侧文件并立即开始比较。
	*/
	class YSSCoreAPI CodeDiff : public FileEditWidget {
		Q_OBJECT;
	public:
		// (oldStart, oldSize, newStart, newSize)，下标均从 0 开始。
		using DiffRange = std::tuple<qint32, qint32, qint32, qint32>;
	public:
		explicit CodeDiff(QWidget* parent = nullptr);
		virtual ~CodeDiff();
	public:
		void setDiffSource(const QStringList& oldLines, const QStringList& newLines);
		QList<DiffRange> getDiffRanges() const;
		void setConflictModeEnable(bool enable);
		bool isConflictModeEnabled() const;
		QList<bool> getConflictSelection() const;
		QStringList getConflictMerged() const;
	public:
		void setTextFont(const QFont& font);
		QFont getTextFont() const;
		void setFontSize(qint32 size);
		qint32 getFontSize() const;
	public:
		virtual bool onVirtualOpen(const QString& ext, const QString& fileName, const QString& param) override;
		virtual bool onVirtualClose() override;
	private:
		CodeDiffPrivate* d;
	};
}
#endif // YSSCore_Editor_CodeDiff_h