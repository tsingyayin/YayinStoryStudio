#include "../FileEditWidget.h"

namespace YSSCore::Editor {
	/*! 
		\class YSSCore::Editor::FileEditWidget
		\brief 此类为Yayin Story Studio 提供文件编辑的基类。
		\since YSSCore 0.13.0
		\inmodule YSSCore

		FileEditWidget是所有文件编辑器的基类，所有文件编辑器都需要继承此类。

		FileEditWidget除了是继承自QFrame之外，几乎相当于纯虚接口类。此类所有成员函数均为纯虚函数。
	*/
	FileEditWidget::FileEditWidget(QWidget* parent) : QFrame(parent) {
		// Constructor implementation
	}
}

/*! 
	\fn QString YSSCore::Editor::FileEditWidget::getFilePath() const
	获取当前文件编辑器的文件路径。
*/

/*! 
	\fn QString YSSCore::Editor::FileEditWidget::getFileName() const
	获取当前文件编辑器的文件名。
*/

/*! 
	\fn bool YSSCore::Editor::FileEditWidget::openFile(const QString& path)
	打开指定路径的文件，并加载其内容。
*/

/*! 
	\fn bool YSSCore::Editor::FileEditWidget::onClose()
	处理文件编辑器关闭时的逻辑。返回true表示可以关闭，返回false表示不能关闭。
*/

/*! 
	\fn bool YSSCore::Editor::FileEditWidget::onSave(const QString& path)
	保存当前文件编辑器的内容到指定路径。如果路径为空，则使用当前文件路径。
*/

/*! 
	\fn bool YSSCore::Editor::FileEditWidget::onSaveAs(const QString& path)
	将当前文件编辑器的内容另存为到指定路径。
*/

/*! 
	\fn bool YSSCore::Editor::FileEditWidget::onReload()
	重新加载当前文件编辑器的内容。如果内容已修改，可能会提示用户保存更改。
*/