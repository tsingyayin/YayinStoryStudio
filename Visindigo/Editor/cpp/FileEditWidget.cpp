#include <QtGui/qevent.h>
#include <QtCore/qfileinfo.h>
#include "../FileEditWidget.h"

namespace Visindigo::Editor {
	VImplClass(FileEditWidget) {
		VIAPI(FileEditWidget);
protected:
	bool fileChanged = false;
	QString filePath;
	};
	/*!
		\class Visindigo::Editor::FileEditWidget
		\brief 此类为Yayin Story Studio 提供文件编辑的基类。
		\since Visindigo 0.13.0
		\inmodule Visindigo

		FileEditWidget是所有文件编辑器的基类，所有文件编辑器都需要继承此类。

		FileEditWidget除了是继承自QFrame之外，几乎相当于纯虚接口类。此类所有成员函数均为纯虚函数。
	*/
	FileEditWidget::FileEditWidget(QWidget* parent) : QFrame(parent) {
		d = new FileEditWidgetPrivate;
	}
	FileEditWidget::~FileEditWidget() {
		delete d;
	}
	QString FileEditWidget::getFilePath() const {
		return d->filePath;
	}
	QString FileEditWidget::getFileName() const {
		return QFileInfo(d->filePath).fileName();
	}
	bool FileEditWidget::isFileChanged() const {
		return d->fileChanged;
	}
	void FileEditWidget::setFileChanged() {
		d->fileChanged = true;
		emit fileChanged(d->filePath);
	}
	void FileEditWidget::cancelFileChanged() {
		d->fileChanged = false;
	}
	bool FileEditWidget::openFile(const QString& path) {
		if (path.isEmpty()) {
			return false;
		}
		d->filePath = path;
		return onOpen(path);
	}
	bool FileEditWidget::saveFile(const QString& path) {
		if (path.isEmpty()) {
			if (d->filePath.isEmpty()) {
				return false;
			}
		}
		else {
			d->filePath = path;
		}
		bool ok = onSave(d->filePath);
		if (ok) {
			d->fileChanged = false;
			emit fileSaved(d->filePath);
		}
		return ok;
	}
	bool FileEditWidget::closeFile() {
		return onClose();
	}
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
	\fn QString Visindigo::Editor::FileEditWidget::getFilePath() const
	获取当前文件编辑器的文件路径。
*/

/*!
	\fn QString Visindigo::Editor::FileEditWidget::getFileName() const
	获取当前文件编辑器的文件名。
*/

/*!
	\fn bool Visindigo::Editor::FileEditWidget::openFile(const QString& path)
	打开指定路径的文件，并加载其内容。
*/

/*!
	\fn bool Visindigo::Editor::FileEditWidget::onClose()
	处理文件编辑器关闭时的逻辑。返回true表示可以关闭，返回false表示不能关闭。
*/

/*!
	\fn bool Visindigo::Editor::FileEditWidget::onSave(const QString& path)
	保存当前文件编辑器的内容到指定路径。如果路径为空，则使用当前文件路径。
*/

/*!
	\fn bool Visindigo::Editor::FileEditWidget::onSaveAs(const QString& path)
	将当前文件编辑器的内容另存为到指定路径。
*/

/*!
	\fn bool Visindigo::Editor::FileEditWidget::onReload()
	重新加载当前文件编辑器的内容。如果内容已修改，可能会提示用户保存更改。
*/