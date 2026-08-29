#include "Editor/MainEditor/FileEditWidgetArea.h"
#include "Editor/MainEditor/private/StackComponents_p.h"
#include "Editor/MainEditor/MainWin.h"
#include <General/YSSProject.h>
#include <Editor/TextEdit.h>
#include <General/Log.h>
#include <Editor/FileServerManager.h>
#include <QtCore/qfileinfo.h>
#include <General/TranslationHost.h>
#include <Editor/VirtualFilePath.h>
#include "Editor/MainEditor/TextEditConfigOperator.h"
#include <QtCore/qmimedata.h>
#include <QtGui/qdrag.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qapplication.h>
#include <Utility/QtSSHelper.h>
namespace YSS::Editor {
	class FileEditWidgetAreaPrivate {
		friend class FileEditWidgetArea;
	protected:
		StackTagWidget* TagArea;
		QFrame* ContentArea = nullptr;
		QVBoxLayout* Layout;
		//MessageViewer* MsgViewer;
		DefaultStackWidgetCentralArea* CentralArea;
		QString areaID;
		qint32 OpenFileCount = 0;
		QLabel* DragInMsgLabel;
		bool focusIn = false;
	protected:
		static FileEditWidgetArea* mainArea;
		static QSet<QString> usedAreaIDs;
		static QMap<QString, FileEditWidgetArea*> areaIDMap;
		static QList<FileEditWidgetArea*> getAllAreas() {
			return areaIDMap.values();
		}
	};

	FileEditWidgetArea* FileEditWidgetAreaPrivate::mainArea = nullptr;
	QSet<QString> FileEditWidgetAreaPrivate::usedAreaIDs = QSet<QString>();
	QMap<QString, FileEditWidgetArea*> FileEditWidgetAreaPrivate::areaIDMap = QMap<QString, FileEditWidgetArea*>();

	FileEditWidgetArea::FileEditWidgetArea(QWidget* parent) :Visindigo::Widgets::BorderFrame(parent) {
		// NOTE: acceptDrops() is only a getter (returns bool); must use
		// setAcceptDrops(true) to actually enable accepting drops.
		this->setAcceptDrops(true);
		this->setFocusPolicy(Qt::StrongFocus);

		d = new FileEditWidgetAreaPrivate;
		if (FileEditWidgetAreaPrivate::mainArea == nullptr) {
			FileEditWidgetAreaPrivate::mainArea = this;
		}
		qint32 size = FileEditWidgetAreaPrivate::usedAreaIDs.size();
		for (int i = 0; i < size + 1; i++) {
			if (not FileEditWidgetAreaPrivate::usedAreaIDs.contains(QString::number(i))) {
				setAreaID(QString::number(i));
				break;
			}
		}
		d->Layout = new QVBoxLayout(this);
		d->Layout->setSpacing(0);
		d->Layout->setContentsMargins(0, 0, 0, 0);
		d->TagArea = new StackTagWidget(this);
		d->TagArea->setArea(this);
		d->TagArea->setFixedHeight(32);

		d->CentralArea = new DefaultStackWidgetCentralArea(this);
		d->CentralArea->setText(VITR("YSS::editor.stackWidgetArea.noFileOpened"));
		d->ContentArea = d->CentralArea;

		d->Layout->addWidget(d->TagArea);
		d->Layout->addWidget(d->ContentArea);

		connect(d->TagArea, &StackTagWidget::switchToFile, this, [this](const QString& filePath) {
			setCurrentWidget(filePath);
			});
		connect(d->TagArea, &StackTagWidget::closeFile, this, [this](const QString& filePath) {
			closeWidget(filePath);
			});
		connect(d->TagArea, &StackTagWidget::renameRequested, this, [this](const QString& oldPath) {
			emit renameRequested(oldPath);
			});
		connect(d->TagArea, &StackTagWidget::saveRequested, this, [this](const QString& filePath) {
			YSSCore::Editor::FileEditWidget* widget = YSSFSM->getFileEditWidget(filePath);
			if (widget) {
				widget->saveFile();
			}
			});
		connect(d->TagArea, &StackTagWidget::saveAsRequested, this, [this](const QString& filePath) {
			emit saveAsRequested(filePath);
			});
		connect(d->TagArea, &StackTagWidget::closeAllRequested, this, [this]() {
			closeAll();
			});
		connect(d->TagArea, &StackTagWidget::closeSavedRequested, this, [this]() {
			closeSaved();
			});

		
		connect(qApp, &QApplication::focusChanged, this, [this](QWidget* old, QWidget* now) {
			if (now and Visindigo::Utility::QtSSHelper::isDescendantsOf(now, this) and not d->focusIn) {
				d->focusIn = true;
				emit areaFocusd(d->areaID);
			}
			else {
				d->focusIn = false;
			}
			});

		d->DragInMsgLabel = new QLabel(this);
		d->DragInMsgLabel->setText(VITR("YSS::editor.stackWidgetArea.dragInFile"));
		d->DragInMsgLabel->setAlignment(Qt::AlignCenter);
		d->DragInMsgLabel->setStyleSheet("background-color: rgba(0, 0, 0, 0.5); color: white; font-size: 16px;");
		d->DragInMsgLabel->hide();

		MainWin::getInstance()->onFileEditWidgetAreaCreated(this);
	}

	FileEditWidgetArea::~FileEditWidgetArea() {
		/*
			IMPORTANT MEMORY TRAP HERE:
			See the comment in ToolWidgetArea destructor.

			只关闭属于本 Area 的编辑部件：FileEditWidget 在 addWidget() 中
			会被 setParent(this)，随后 QLayout::insertWidget 也保持其父级为本 Area。
			若这里关闭所有已打开文件，销毁树状布局中任意一个 Area 都会误关全局文件。
		*/
		for (auto widget : YSSFSM->getAllFileEditWidgets()) {
			if (widget->parent() == this) {
				widget->disconnect(this);
				widget->disconnect(d->TagArea);
				widget->setParent(nullptr);
				widget->closeFile();
			}
		}
		if (FileEditWidgetAreaPrivate::mainArea == this) {
			FileEditWidgetAreaPrivate::mainArea = nullptr;
		}
		FileEditWidgetAreaPrivate::areaIDMap.remove(d->areaID);
		FileEditWidgetAreaPrivate::usedAreaIDs.remove(d->areaID);
		emit areaClosed(d->areaID);
		vgDebug << "FileEditWidgetArea destroyed: " << d->areaID;
		delete d;
	}

	void FileEditWidgetArea::setAreaID(const QString& areaID) {
		if (FileEditWidgetAreaPrivate::usedAreaIDs.contains(areaID)) {
			return;
		}
		if (not d->areaID.isEmpty()) {
			FileEditWidgetAreaPrivate::areaIDMap.remove(d->areaID);
			FileEditWidgetAreaPrivate::usedAreaIDs.remove(d->areaID);
		}
		d->areaID = areaID;
		FileEditWidgetAreaPrivate::areaIDMap.insert(areaID, this);
		FileEditWidgetAreaPrivate::usedAreaIDs.insert(areaID);
	}

	QString FileEditWidgetArea::getAreaID() const {
		return d->areaID;
	}

	FileEditWidgetArea* FileEditWidgetArea::getAreaByID(const QString& areaID) {
		return FileEditWidgetAreaPrivate::areaIDMap.value(areaID, nullptr);
	}

	FileEditWidgetArea* FileEditWidgetArea::getMainArea() {
		return FileEditWidgetAreaPrivate::mainArea;
	}

	QList<FileEditWidgetArea*> FileEditWidgetArea::getAllAreas() {
		return FileEditWidgetAreaPrivate::getAllAreas();
	}

	void FileEditWidgetArea::addWidget(YSSCore::Editor::FileEditWidget* widget) {
		QString filePath = widget->getFilePath();
		vgDebug << filePath;
		if (d->TagArea->containsStackLabel(filePath)) {
			setCurrentWidget(filePath);
			return;
		}
		widget->setParent(this);
		auto vfp = YSSCore::Editor::VirtualFilePath(filePath);
		if (vfp.isValid()) {
			d->TagArea->addStackLabel(filePath, vfp.getFileName());
		}
		else {
			d->TagArea->addStackLabel(filePath);
		}
	
		connect(widget, &YSSCore::Editor::FileEditWidget::fileChanged, d->TagArea, &StackTagWidget::setFileChanged);
		connect(widget, &YSSCore::Editor::FileEditWidget::fileChangeCanceled, d->TagArea, &StackTagWidget::cancelFileChanged);
		connect(widget, &YSSCore::Editor::FileEditWidget::fileSaved, d->TagArea, &StackTagWidget::cancelFileChanged);
		connect(widget, &YSSCore::Editor::FileEditWidget::fileClosed, this, [this, widget]() {
			d->TagArea->removeStackLabel(widget->getFilePath()); // this function handle re-choice if the closed widget is current one
			YSSCore::General::YSSProject::getCurrentProject()->removeEditorOpenedFile(widget->getFilePath());
			d->OpenFileCount--;
			if (d->OpenFileCount <= 0) {
				d->OpenFileCount = 0;
				emit allFileClosed();
			}
			});
		connect(widget, &YSSCore::Editor::FileEditWidget::fileRenamed, this, [this](const QString& oldPath, const QString& newPath) {
			d->TagArea->changeStackLabel(oldPath, newPath);
			YSSCore::General::YSSProject::getCurrentProject()->removeEditorOpenedFile(oldPath);
			YSSCore::General::YSSProject::getCurrentProject()->addEditorOpenedFile(newPath, getAreaID());
			});
		YSSCore::Editor::TextEdit* textEdit = qobject_cast<YSSCore::Editor::TextEdit*>(widget);
		if (textEdit) {
			textEdit->setHoverArea(YSS::Editor::MainWin::getInstance());
			connect(textEdit, &YSSCore::Editor::TextEdit::cursorPositionChanged, this, [this, textEdit]() {
				emit textEditCursorPositionChanged(textEdit->getFilePath(), textEdit->getTextCursor());
				});
			YSS::Editor::TextEditConfigOperator::applyTo(textEdit);
		}
		YSSCore::General::YSSProject::getCurrentProject()->addEditorOpenedFile(filePath, d->areaID);
		d->OpenFileCount++;
		setCurrentWidget(filePath);
	}

	bool FileEditWidgetArea::containsWidget(const QString& filePath) const {
		return d->TagArea->containsStackLabel(filePath);
	}

	void FileEditWidgetArea::closeAll(bool autoGiveup) {
		for (auto widget : YSSFSM->getAllFileEditWidgets()) {
			vgDebug << "close " << widget->getFilePath();
			widget->closeFile(autoGiveup);
		}
	}

	void FileEditWidgetArea::closeSaved() {
		for (auto widget : YSSFSM->getAllFileEditWidgets()) {
			QString filePath = widget->getFilePath();
			if (not widget->isFileChanged() and not d->TagArea->isStackLabelPinned(filePath)) {
				vgDebug << "close " << filePath;
				widget->closeFile();
			}
		}
	}
	void FileEditWidgetArea::closeWidget(YSSCore::Editor::FileEditWidget* widget) {
		closeWidget(widget->getFilePath());
	}

	void FileEditWidgetArea::closeWidget(const QString& filePath) {
		YSSCore::Editor::FileEditWidget* widget = YSSFSM->getFileEditWidget(filePath);
		if (not widget) {
			return;
		}
		widget->closeFile();
	}

	void FileEditWidgetArea::setCurrentWidget(YSSCore::Editor::FileEditWidget* widget) {
		setCurrentWidget(widget->getFilePath());
	}

	void FileEditWidgetArea::setCurrentWidget(const QString& filePath) {
		if (filePath == getCurrentWidgetFilePath()) {
			return;
		}
		if (filePath.isEmpty()) {
			d->ContentArea->hide();
			d->Layout->removeWidget(d->ContentArea);
			d->ContentArea = d->CentralArea;
			d->Layout->insertWidget(1, d->ContentArea);
			d->ContentArea->show();
		}
		YSSCore::Editor::FileEditWidget* widget = YSSFSM->getFileEditWidget(filePath);
		vgDebug << "setCurrentWidget: " << filePath << " widget: " << widget;
		if (not widget) {
			return;
		}
		if (d->ContentArea != widget) {
			d->ContentArea->hide();
			d->Layout->removeWidget(d->ContentArea);
			d->ContentArea = widget;
			d->Layout->insertWidget(1, d->ContentArea);
			d->ContentArea->show();
		}
		YSSCore::General::YSSProject::getCurrentProject()->setFocusedFile(filePath);
		emit currentFileChanged(filePath);
		d->TagArea->setCurrentStackLabel(filePath);
	}

	void FileEditWidgetArea::setCurrentWidget(YSSCore::Editor::FileEditWidget* widget, qint32 lineNumber, qint32 column) {
		setCurrentWidget(widget->getFilePath(), lineNumber, column);
	}

	void FileEditWidgetArea::setCurrentWidget(const QString& filePath, qint32 lineNumber, qint32 column) {
		if (filePath.isEmpty()) {
			d->ContentArea->hide();
			d->Layout->removeWidget(d->ContentArea);
			d->ContentArea = d->CentralArea;
			d->Layout->insertWidget(1, d->ContentArea);
			d->ContentArea->show();
		}
		QFileInfo fileInfo(filePath);
		QString absPath = fileInfo.absoluteFilePath();
		if (not YSSFSM->getAllOpenedFilePaths().contains(absPath)) {
			return;
		}
		YSSCore::Editor::FileEditWidget* widget = YSSFSM->getFileEditWidget(absPath);
		if (not widget) {
			return;
		}
		setCurrentWidget(widget);
		widget->cursorToPosition(lineNumber, column);
	}

	YSSCore::Editor::FileEditWidget* FileEditWidgetArea::getCurrentWidget() const {
		if (d->ContentArea == d->CentralArea) {
			return nullptr;
		}
		return qobject_cast<YSSCore::Editor::FileEditWidget*>(d->ContentArea);
	}

	QString FileEditWidgetArea::getCurrentWidgetFilePath() const {
		auto widget = getCurrentWidget();
		if (widget) {
			return widget->getFilePath();
		}
		return QString();
	}

	void FileEditWidgetArea::moveWidgetTo(const QString& filePath, FileEditWidgetArea* otherArea) {
		QString absPath = filePath;
		if (not YSSCore::Editor::VirtualFilePath::isVirtualFilePath(filePath)) {
			absPath = QFileInfo(filePath).absoluteFilePath();
		}
		d->TagArea->removeStackLabel(absPath);
		YSSCore::Editor::FileEditWidget* widget = YSSFSM->getFileEditWidget(absPath);
		YSSCore::General::YSSProject::getCurrentProject()->removeEditorOpenedFile(absPath);
		if (widget) {
			widget->disconnect(this);
			widget->disconnect(d->TagArea);
			otherArea->addWidget(widget);
			d->OpenFileCount--;
			if (d->OpenFileCount <= 0) {
				d->OpenFileCount = 0;
				emit allFileClosed();
			}
		}
	}

	void FileEditWidgetArea::dragEnterEvent(QDragEnterEvent* event) {
		if (event->mimeData()->hasFormat(StackTag::stackTagDragMimeType)) {
			event->acceptProposedAction();
			d->DragInMsgLabel->show();
			d->DragInMsgLabel->raise();
			d->DragInMsgLabel->resize(this->size());
		}
	}
	void FileEditWidgetArea::dragMoveEvent(QDragMoveEvent* event) {
		if (event->mimeData()->hasFormat(StackTag::stackTagDragMimeType)) {
			event->acceptProposedAction();
		}
	}

	void FileEditWidgetArea::dragLeaveEvent(QDragLeaveEvent* event) {
		QFrame::dragLeaveEvent(event);
		d->DragInMsgLabel->hide();
	}

	void FileEditWidgetArea::dropEvent(QDropEvent* event) {
		if (event->mimeData()->hasFormat(StackTag::stackTagDragMimeType)) {
			QByteArray data = event->mimeData()->data(StackTag::stackTagDragMimeType);
			Visindigo::Utility::JsonConfig json(QString::fromUtf8(data));
			QString selfPtr = json.getString("self");
			StackTag* selfLabel = reinterpret_cast<StackTag*>(selfPtr.toULongLong(nullptr, 16));
			// The drag source serializes the "area" key: the StackTagWidget the dragged
			// tag stays in (i.e. the SOURCE tag bar), not "parent".
			QString sourcePtr = json.getString("area");
			StackTagWidget* sourceTagArea = reinterpret_cast<StackTagWidget*>(sourcePtr.toULongLong(nullptr, 16));
			if (selfLabel && sourceTagArea && sourceTagArea != d->TagArea) {
				FileEditWidgetArea* from = sourceTagArea->getArea();
				if (from && from != this) {
					from->moveWidgetTo(selfLabel->getFilePath(), this);
				}
			}
		}
		d->DragInMsgLabel->hide();
	}

	void FileEditWidgetArea::resizeEvent(QResizeEvent* event) {
		QFrame::resizeEvent(event);

	}
}