#include <QtGui/qevent.h>
#include <QtCore/qfileinfo.h>
#include "../FileEditWidget.h"
#include <General/Log.h>
#include "General/YSSLogger.h"
#include <Utility/FileUtility.h>
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
		\inmodule YSSCore

		FileEditWidget是所有文件编辑器的基类，所有文件编辑器都需要继承此类。

		FileEditWidget除了是继承自QFrame之外，几乎相当于纯虚接口类。此类的虚函数均为纯虚函数，派生类必须全部实现这些纯虚函数才能实例化对象。
	*/

	/*!
		\fn YSSCore::Editor::FileEditWidget::fileChanged(const QString& filePath)
		\since YSS 0.13.0
		当文件内容被修改时，应触发此信号。
	*/

	/*!
		\fn YSSCore::Editor::FileEditWidget::fileChangeCanceled(const QString& filePath)
		\since YSS 0.14.0
		当文件内容修改被取消时，应触发此信号。
	*/

	/*!
		\fn YSSCore::Editor::FileEditWidget::fileSaved(const QString& filePath)
		\since YSS 0.13.0
		当文件被成功保存时，自动触发此信号。
	*/

	/*!
		\fn YSSCore::Editor::FileEditWidget::fileRenamed(const QString& rawPath, const QString& changedPath)
		\since YSS 0.15.0
		当文件路径被修改时，自动触发此信号。
	*/

	/*!
		\fn YSSCore::Editor::FileEditWidget::closed(const QString& filePath)
		\since YSS 0.15.0
		当文件最终被关闭时，自动触发此信号。
		此信号在onClose返回true后，即允许关闭的情况下才会触发
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
		\since Visindigo 0.13.0
		获取当前文件编辑器的文件路径。
		return 返回当前文件编辑器的文件路径。如果当前没有打开任何文件，则返回空字符串。
	*/
	QString FileEditWidget::getFilePath() const {
		return d->filePath;
	}

	/*!
		\since Visindigo 0.13.0
		获取当前文件编辑器的文件名。
		return 返回当前文件编辑器的文件名。如果当前没有打开任何文件，则返回空字符串。
	*/
	QString FileEditWidget::getFileName() const {
		return QFileInfo(d->filePath).fileName();
	}

	/*!
		\since Visindigo 0.13.0
		判断当前文件内容是否被修改。
		return 如果文件内容被修改，返回true；否则返回false。
	*/
	bool FileEditWidget::isFileChanged() const {
		return d->fileChanged;
	}

	/*!
		\since Visindigo 0.13.0
		将当前文件内容标记为已修改状态。
		此函数通常在派生类中被调用，当用户修改了文件内容时，应调用此函数以更新文件状态。
		之后，它会自动触发fileChanged信号，通知外部文件内容已被修改，并传递当前文件路径作为参数。
	*/
	void FileEditWidget::setFileChanged() {
		d->fileChanged = true;
		emit fileChanged(d->filePath);
	}

	/*!
		\since Visindigo 0.13.0
		将当前文件内容标记为未修改状态。
		此函数通常在派生类中被调用，当文件内容被保存或重新加载后，应调用此函数以更新文件状态。
		之后，它会自动触发fileChangeCanceled信号，通知外部文件内容修改已被取消，并传递当前文件路径作为参数。
	*/
	void FileEditWidget::cancelFileChanged() {
		d->fileChanged = false;
		emit fileChangeCanceled(d->filePath);
	}

	/*!
		\since Visindigo 0.13.0
		打开指定路径的文件，并加载其内容。
		\a path 要打开的文件路径。
		return 如果文件成功打开并加载，返回true；否则返回false。

		此类在判定文件路径是否为空后，调用派生类实现的onOpen()函数以实际打开文件。
	*/
	bool FileEditWidget::openFile(const QString& path) {
		if (path.isEmpty()) {
			yWarningF << "File path is empty.";
			return false;
		}
		d->filePath = path;
		if (onOpen(path)) {
			return true;
		}
		return false;
	}

	/*!
		\since YSS 0.15.0
		保存当前文件编辑器的内容到指定路径。
		\a path 要保存的文件路径。如果为空字符串，则使用当前文件路径。非空字符串在保存成功后会更新当前文件路径。
		\a deleteWhenSaveAs 当执行另存为操作时，是否删除原文件。默认为false，即不删除原文件。如果设置为true，
		可以视作重命名文件，保存成功后原文件将被删除。请谨慎使用此选项，以免误删重要文件。
		return 如果文件成功保存，返回true；否则返回false。
		此类在判定文件路径是否为空后，调用派生类实现的onSave()函数以实际保存文件。
		一旦文件成功保存，fileChanged状态会被重置为false，并触发fileSaved信号。

		\note 这个函数最初从0.13.0引入，在0.15.0中增加了deleteWhenSaveAs参数以支持在另存为操作时删除原文件的功能。
		此外，从0.15.0开始，这个函数会在调用虚函数onSave之后，额外检查文件是否确实存在，以确保保存操作的成功性。
		这是因为某些派生类的onSave实现可能会在保存失败时没有正确返回false，因此增加了文件存在性的检查以提高函数的可靠性。
	*/
	bool FileEditWidget::saveFile(const QString& path, bool deleteWhenSaveAs) {
		QString pathCopy = path;
		if (path.isEmpty()) {
			if (d->filePath.isEmpty()) {
				return false;
			}
			pathCopy = d->filePath;
		}
		bool ok = onSave(pathCopy);
		if (not Visindigo::Utility::FileUtility::isFileExist(pathCopy)) {
			return false;
		}
		if (ok) {
			cancelFileChanged();
			if (not path.isEmpty() && path != d->filePath) {
				if (deleteWhenSaveAs) {
					QFile::remove(d->filePath);
				}
				QString oldPath = d->filePath;
				d->filePath = pathCopy;
				emit fileSaved(d->filePath);
				emit fileRenamed(oldPath, pathCopy);
			}
			else {
				d->filePath = pathCopy;
				emit fileSaved(d->filePath);
			}
		}
		return ok;
	}

	/*!
		\since Visindigo 0.13.0
		重新加载当前文件编辑器的内容。
		return 如果文件成功重新加载，返回true；否则返回false。
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
		\since Visindigo 0.13.0
		复制当前选中的内容到剪贴板。
		return 如果复制操作成功，返回true；否则返回false。
		此类调用派生类实现的onCopy()函数以实际处理复制逻辑。
	*/
	bool FileEditWidget::copy() {
		return onCopy();
	}

	/*!
		\since Visindigo 0.13.0
		剪切当前选中的内容到剪贴板。
		return 如果剪切操作成功，返回true；否则返回false。
		此类调用派生类实现的onCut()函数以实际处理剪切逻辑。
	*/
	bool FileEditWidget::cut() {
		return onCut();
	}

	/*!
		\since Visindigo 0.13.0
		从剪贴板粘贴内容到当前光标位置。
		return 如果粘贴操作成功，返回true；否则返回false。
		此类调用派生类实现的onPaste()函数以实际处理粘贴逻辑。
	*/
	bool FileEditWidget::paste() {
		return onPaste();
	}

	/*!
		\since Visindigo 0.13.0
		撤销上一次编辑操作。
		return 如果撤销操作成功，返回true；否则返回false。
		此类调用派生类实现的onUndo()函数以实际处理撤销逻辑。
	*/
	bool FileEditWidget::undo() {
		return onUndo();
	}

	/*!
		\since Visindigo 0.13.0
		重做上一次被撤销的编辑操作。
		return 如果重做操作成功，返回true；否则返回false。
		此类调用派生类实现的onRedo()函数以实际处理重做逻辑。
	*/
	bool FileEditWidget::redo() {
		return onRedo();
	}

	/*!
		\since Visindigo 0.13.0
		选中当前文件编辑器中的所有内容。
		return 如果全选操作成功，返回true；否则返回false。
		此类调用派生类实现的onSelectAll()函数以实际处理全选逻辑。
	*/
	bool FileEditWidget::selectAll() {
		return onSelectAll();
	}

	/*!
		\since YSSCore 0.13.0
		\a lineNumber 行号，从1开始。
		\a column 列号，从1开始。
		将光标移动到指定的行号和列号位置。
	*/
	bool FileEditWidget::cursorToPosition(qint32 lineNumber, qint32 column) {
		return onCursorToPosition(lineNumber, column);
	}

	/*!
		\since YSSCore 0.13.0
		\a lineNumber 行号，从1开始。
		\a column 列号，从1开始。
		由派生类实现的函数，用于将光标移动到指定的行号和列号位置。
		考虑到FileEditWidget并不总一定是代码编辑器，因此可按需实现它。
	*/
	bool FileEditWidget::onCursorToPosition(qint32 lineNumber, qint32 column) {
		return false;
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
			emit fileClosed(d->filePath);
		}
		else {
			event->ignore();
		}
	}

	/*!
		\fn YSSCore::Editor::FileEditWidget::onOpen(const QString& path) = 0
		\since YSS 0.13.0
		派生类必须实现此纯虚函数以处理文件打开逻辑。
		\a path 要打开的文件路径。

		return 如果文件成功打开并加载，返回true；否则返回false。

		\warning 从0.15开始，这个函数可能因为onReload而重复调用，不保证
		在整个生命周期内只调用一次。因此如果你在这个函数内尝试懒加载你的
		FileEditWidget，应当首先检查资源是否已经初始化，避免出现内存泄漏。
	*/

	/*!
		\fn YSSCore::Editor::FileEditWidget::onClose() = 0
		\since YSS 0.13.0

		派生类必须实现此纯虚函数以处理文件关闭逻辑。
		return 如果文件编辑器可以关闭，返回true；否则返回false。

		\warning 不要在这个函数里调用close或deleteLater等任何实质上真的
		会关闭这个Widget的函数，会造成递归爆栈。这个函数的唯一作用是通过
		返回值告知YSS是否真的应该关闭它。因此你可以在里面做一些保存、警告等操作
	*/

	/*!
		\fn YSSCore::Editor::FileEditWidget::onSave(const QString& path = "") = 0
		\since YSS 0.13.0

		派生类必须实现此纯虚函数以处理文件保存逻辑。
		\a path 要保存的文件路径。如果为空字符串，则使用当前文件路径。非空字符串在保存成功后会更新当前文件路径。
		return 如果文件成功保存，返回true；否则返回false。
	*/

	/*!
		\since YSS 0.13.0
		派生类实现此纯虚函数以处理文件重新加载逻辑。
		return 如果文件成功重新加载，返回true；否则返回false。

		在0.15之前的版本，这个函数默认返回false
		从0.15开始，这个函数默认使用现在的文件路径重新调用onOpen()
	*/
	bool FileEditWidget::onReload() {
		if (d->filePath.isEmpty()) {
			return false;
		}
		return onOpen(d->filePath);
	}

	/*!
		\since YSS 0.13.0
		派生类实现此纯虚函数以处理复制逻辑。
		return 如果复制操作成功，返回true；否则返回false。
	*/
	bool FileEditWidget::onCopy() {
		return false;
	}
	/*!
		\since YSS 0.13.0
		派生类实现此纯虚函数以处理剪切逻辑。
		return 如果剪切操作成功，返回true；否则返回false。
	*/
	bool FileEditWidget::onCut() {
		return false;
	}

	/*!
		\since YSS 0.13.0
		派生类实现此纯虚函数以处理粘贴逻辑。
		return 如果粘贴操作成功，返回true；否则返回false。
	*/
	bool FileEditWidget::onPaste() {
		return false;
	}

	/*!
		\since YSS 0.13.0
		派生类实现此纯虚函数以处理撤销逻辑。
		return 如果撤销操作成功，返回true；否则返回false。
	*/
	bool FileEditWidget::onUndo() {
		return false;
	}

	/*!
		\since YSS 0.13.0
		派生类实现此纯虚函数以处理重做逻辑。
		return 如果重做操作成功，返回true；否则返回false。
	*/
	bool FileEditWidget::onRedo() {
		return false;
	}

	/*!
		\since YSS 0.13.0
		派生类实现此纯虚函数以处理全选逻辑。
		return 如果全选操作成功，返回true；否则返回false。
	*/
	bool FileEditWidget::onSelectAll() {
		return false;
	}
}