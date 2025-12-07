#include <QtGui/qevent.h>
#include <QtCore/qfileinfo.h>
#include "../FileEditWidget.h"
#include <General/Log.h>
namespace YSSCore::Editor {
	VImplClass(FileEditWidget) {
		VIAPI(FileEditWidget);
protected:
	bool fileChanged = false;
	QString filePath;
	};
	/*!
		\class YSSCore::Editor::FileEditWidget
		\brief 此类为Yayin Story Studio 提供文件编辑的基类。
		\since Visindigo 0.13.0
		\inmodule Visindigo

		FileEditWidget是所有文件编辑器的基类，所有文件编辑器都需要继承此类。

		FileEditWidget除了是继承自QFrame之外，几乎相当于纯虚接口类。此类的虚函数均为纯虚函数，派生类必须全部实现这些纯虚函数才能实例化对象。
	*/

	/*!
		\fn YSSCore::Editor::FileEditWidget::fileChanged(const QString& filePath)
		当文件内容被修改时，此信号会被触发。
	*/

	/*!
		\fn YSSCore::Editor::FileEditWidget::fileSaved(const QString& filePath)
		当文件被成功保存时，此信号会被触发。
	*/

	/*!
		\since Visindigo 0.13.0
		\a parent 为父对象。
		构造FileEditWidget对象。
	*/
	FileEditWidget::FileEditWidget(QWidget* parent) : QFrame(parent) {
		d = new FileEditWidgetPrivate;
	}

	/*!
		\since Visindigo 0.13.0
		析构FileEditWidget对象。
	*/
	FileEditWidget::~FileEditWidget() {
		delete d;
	}

	/*!
		获取当前文件编辑器的文件路径。
		\return 返回当前文件编辑器的文件路径。如果当前没有打开任何文件，则返回空字符串。
	*/
	QString FileEditWidget::getFilePath() const {
		return d->filePath;
	}

	/*!
		获取当前文件编辑器的文件名。
		\return 返回当前文件编辑器的文件名。如果当前没有打开任何文件，则返回空字符串。
	*/
	QString FileEditWidget::getFileName() const {
		return QFileInfo(d->filePath).fileName();
	}

	/*!
		判断当前文件内容是否被修改。
		\return 如果文件内容被修改，返回true；否则返回false。
	*/
	bool FileEditWidget::isFileChanged() const {
		return d->fileChanged;
	}

	/*!
		将当前文件内容标记为已修改状态，并触发fileChanged信号。
		此函数通常在派生类中被调用，当用户修改了文件内容时，应调用此函数以更新文件状态。
	*/
	void FileEditWidget::setFileChanged() {
		d->fileChanged = true;
		emit fileChanged(d->filePath);
	}

	/*!
		将当前文件内容标记为未修改状态。
		此函数通常在派生类中被调用，当文件内容被保存或重新加载后，应调用此函数以更新文件状态。
	*/
	void FileEditWidget::cancelFileChanged() {
		d->fileChanged = false;
	}

	/*!
		打开指定路径的文件，并加载其内容。
		\a path 要打开的文件路径。
		\return 如果文件成功打开并加载，返回true；否则返回false。

		此类在判定文件路径是否为空后，调用派生类实现的onOpen()函数以实际打开文件。
	*/
	bool FileEditWidget::openFile(const QString& path) {
		if (path.isEmpty()) {
			yWarningF << "File path is empty.";
			return false;
		}
		if (onOpen(path)) {
			d->filePath = path;
			return true;
		}
		return false;
	}

	/*!
		保存当前文件编辑器的内容到指定路径。
		\a path 要保存的文件路径。如果为空字符串，则使用当前文件路径。非空字符串在保存成功后会更新当前文件路径。
		\return 如果文件成功保存，返回true；否则返回false。
		此类在判定文件路径是否为空后，调用派生类实现的onSave()函数以实际保存文件。
		一旦文件成功保存，fileChanged状态会被重置为false，并触发fileSaved信号。
	*/
	bool FileEditWidget::saveFile(const QString& path) {
		QString pathCopy = path;
		if (path.isEmpty()) {
			if (d->filePath.isEmpty()) {
				return false;
			}
			pathCopy = d->filePath;
		}
		bool ok = onSave(pathCopy);
		if (ok) {
			d->filePath = pathCopy;
			d->fileChanged = false;
			emit fileSaved(d->filePath);
		}
		return ok;
	}

	/*!
		重新加载当前文件编辑器的内容。
		\return 如果文件成功重新加载，返回true；否则返回false。
		此类在判定当前文件路径是否为空后，调用派生类实现的onReload()函数以实际重新加载文件。
	*/
	bool FileEditWidget::reloadFile() {
		if (d->filePath.isEmpty()) {
			yWarningF << "File path is empty.";
			return false;
		}
		return onReload();
	}

	/*!
		关闭当前文件编辑器。
		\return 如果文件编辑器可以关闭，返回true；否则返回false。
		此类调用派生类实现的onClose()函数以实际处理关闭逻辑。
	*/
	bool FileEditWidget::closeFile() {
		return onClose();
	}

	/*!
		复制当前选中的内容到剪贴板。
		\return 如果复制操作成功，返回true；否则返回false。
		此类调用派生类实现的onCopy()函数以实际处理复制逻辑。
	*/
	bool FileEditWidget::copy() {
		return onCopy();
	}

	/*!
		剪切当前选中的内容到剪贴板。
		\return 如果剪切操作成功，返回true；否则返回false。
		此类调用派生类实现的onCut()函数以实际处理剪切逻辑。
	*/
	bool FileEditWidget::cut() {
		return onCut();
	}

	/*!
		从剪贴板粘贴内容到当前光标位置。
		\return 如果粘贴操作成功，返回true；否则返回false。
		此类调用派生类实现的onPaste()函数以实际处理粘贴逻辑。
	*/
	bool FileEditWidget::paste() {
		return onPaste();
	}

	/*!
		撤销上一次编辑操作。
		\return 如果撤销操作成功，返回true；否则返回false。
		此类调用派生类实现的onUndo()函数以实际处理撤销逻辑。
	*/
	bool FileEditWidget::undo() {
		return onUndo();
	}

	/*!
		重做上一次被撤销的编辑操作。
		\return 如果重做操作成功，返回true；否则返回false。
		此类调用派生类实现的onRedo()函数以实际处理重做逻辑。
	*/
	bool FileEditWidget::redo() {
		return onRedo();
	}

	/*!
		选中当前文件编辑器中的所有内容。
		\return 如果全选操作成功，返回true；否则返回false。
		此类调用派生类实现的onSelectAll()函数以实际处理全选逻辑。
	*/
	bool FileEditWidget::selectAll() {
		return onSelectAll();
	}

	/*!
		处理文件编辑器关闭事件。
		\a event 关闭事件对象。
		此函数在文件编辑器尝试关闭时被调用，调用派生类实现的onClose()函数以决定是否允许关闭。
		如果onClose()返回true，则接受关闭事件；否则忽略关闭事件，文件编辑器保持打开状态。
		派生类若重载了closeEvent()，应确保调用基类的实现以保持正确的关闭行为。
	*/
	void FileEditWidget::closeEvent(QCloseEvent* event) {
		if (onClose()) {
			event->accept();
		}
		else {
			event->ignore();
		}
	}
}

/*!
	\fn YSSCore::Editor::FileEditWidget::onOpen(const QString& path) = 0
	派生类必须实现此纯虚函数以处理文件打开逻辑。
	\a path 要打开的文件路径。

	\return 如果文件成功打开并加载，返回true；否则返回false。
*/

/*!
	\fn YSSCore::Editor::FileEditWidget::onClose() = 0
	派生类必须实现此纯虚函数以处理文件关闭逻辑。
	\return 如果文件编辑器可以关闭，返回true；否则返回false。
*/

/*!
	\fn YSSCore::Editor::FileEditWidget::onSave(const QString& path = "") = 0
	派生类必须实现此纯虚函数以处理文件保存逻辑。
	\a path 要保存的文件路径。如果为空字符串，则使用当前文件路径。非空字符串在保存成功后会更新当前文件路径。
	\return 如果文件成功保存，返回true；否则返回false。
*/

/*!
	\fn YSSCore::Editor::FileEditWidget::onReload() = 0
	派生类必须实现此纯虚函数以处理文件重新加载逻辑。
	\return 如果文件成功重新加载，返回true；否则返回false。
*/

/*!
	\fn YSSCore::Editor::FileEditWidget::onCopy() = 0
	派生类必须实现此纯虚函数以处理复制逻辑。
	\return 如果复制操作成功，返回true；否则返回false。
*/

/*!
	\fn YSSCore::Editor::FileEditWidget::onCut() = 0
	派生类必须实现此纯虚函数以处理剪切逻辑。
	\return 如果剪切操作成功，返回true；否则返回false。
*/

/*!
	\fn YSSCore::Editor::FileEditWidget::onPaste() = 0
	派生类必须实现此纯虚函数以处理粘贴逻辑。
	\return 如果粘贴操作成功，返回true；否则返回false。
*/

/*!
	\fn YSSCore::Editor::FileEditWidget::onUndo() = 0
	派生类必须实现此纯虚函数以处理撤销逻辑。
	\return 如果撤销操作成功，返回true；否则返回false。
*/

/*!
	\fn YSSCore::Editor::FileEditWidget::onRedo() = 0
	派生类必须实现此纯虚函数以处理重做逻辑。
	\return 如果重做操作成功，返回true；否则返回false。
*/

/*!
	\fn YSSCore::Editor::FileEditWidget::onSelectAll() = 0
	派生类必须实现此纯虚函数以处理全选逻辑。
	\return 如果全选操作成功，返回true；否则返回false。
*/