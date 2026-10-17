#include <algorithm>
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qmimedata.h>
#include <QtGui/qdrag.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qapplication.h>
#include <Editor/FileServerManager.h>
#include <Editor/TextEdit.h>
#include <Editor/VirtualFilePath.h>
#include <General/Log.h>
#include <General/TranslationHost.h>
#include <General/YSSProject.h>
#include <Utility/QtSSHelper.h>
#include "Editor/MainEditor/FileEditWidgetArea.h"
#include "Editor/MainEditor/MainWin.h"
#include "Editor/MainEditor/private/StackComponents_p.h"
#include "Editor/MainEditor/TextEditConfigOperator.h"
namespace YSS::Editor {
	namespace {
		// YSSProject 仍以"本地路径"为索引：本机文件给本地绝对路径（正斜杠，与
		// QFileInfo::absoluteFilePath() 一致），虚拟文件给 @ext!name?param，
		// 远端文件暂时给 URI 字符串——项目级远程路由尚未实现。
		QString toProjectPath(const YSSCore::General::YSSURI& uri) {
			if (not uri.isValid()) {
				return QString();
			}
			if (uri.isVirtual()) {
				return uri.getVirtualPath();
			}
			if (uri.isLocal()) {
				return QDir::fromNativeSeparators(uri.getLocalPath());
			}
			return uri.toString();
		}
	}

	class AreaRegistry {
	protected:
		FileEditWidgetArea* mainArea = nullptr;
		FileEditWidgetArea* currentArea = nullptr;
		YSSCore::Editor::FileEditWidget* currentFocusedWidget = nullptr;
		QSet<QString> usedIDs;
		QMap<QString, FileEditWidgetArea*> areas;
	public:
		static AreaRegistry& instance() {
			static AreaRegistry registry;
			return registry;
		}
		QString allocateFreeID() const {
			qint32 size = usedIDs.size();
			for (int i = 0; i < size + 1; i++) {
				if (not usedIDs.contains(QString::number(i))) {
					return QString::number(i);
				}
			}
			return QString::number(size);
		}
		bool acquire(const QString& id, FileEditWidgetArea* area) {
			if (usedIDs.contains(id)) {
				return false;
			}
			usedIDs.insert(id);
			areas.insert(id, area);
			return true;
		}
		void release(const QString& id) {
			usedIDs.remove(id);
			areas.remove(id);
		}
		FileEditWidgetArea* find(const QString& id) const {
			return areas.value(id, nullptr);
		}
		FileEditWidgetArea* main() const {
			return mainArea;
		}
		void setMain(FileEditWidgetArea* area) {
			mainArea = area;
		}
		FileEditWidgetArea* current() const {
			return currentArea;
		}
		YSSCore::Editor::FileEditWidget* currentWidget() const {
			return currentFocusedWidget;
		}
		void setCurrent(FileEditWidgetArea* area, YSSCore::Editor::FileEditWidget* widget) {
			currentArea = area;
			currentFocusedWidget = widget;
		}
		QList<FileEditWidgetArea*> allAreas() const {
			return areas.values();
		}
	};

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
		bool closeSilently = false;
	};

	FileEditWidgetArea::FileEditWidgetArea(QWidget* parent, const QString& requestedID) :Visindigo::Widgets::BorderFrame(parent) {
		// NOTE: acceptDrops() is only a getter (returns bool); must use
		// setAcceptDrops(true) to actually enable accepting drops.
		this->setAcceptDrops(true);
		this->setFocusPolicy(Qt::StrongFocus);

		d = new FileEditWidgetAreaPrivate;
		if (AreaRegistry::instance().main() == nullptr) {
			AreaRegistry::instance().setMain(this);
		}
		QString id = requestedID.isEmpty() ? AreaRegistry::instance().allocateFreeID() : requestedID;
		if (not AreaRegistry::instance().acquire(id, this)) {
			id = AreaRegistry::instance().allocateFreeID();
			AreaRegistry::instance().acquire(id, this);
		}
		d->areaID = id;
		d->Layout = new QVBoxLayout(this);
		d->Layout->setSpacing(0);
		d->Layout->setContentsMargins(0, 0, 0, 0);
		d->TagArea = new StackTagWidget(this);
		d->TagArea->setArea(this);
		d->TagArea->setUseGlobalFocus(true);
		d->TagArea->setFixedHeight(32);

		d->CentralArea = new DefaultStackWidgetCentralArea(this);
		d->CentralArea->setText(VITRL("YSS::editor.stackWidgetArea.noFileOpened"));
		d->ContentArea = d->CentralArea;

		d->Layout->addWidget(d->TagArea);
		d->Layout->addWidget(d->ContentArea);

		connect(d->TagArea, &StackTagWidget::switchToFile, this, [this](const YSSCore::General::YSSURI& uri) {
			setCurrentWidget(uri);
			});
		connect(d->TagArea, &StackTagWidget::closeFile, this, [this](const YSSCore::General::YSSURI& uri) {
			closeWidget(uri);
			});
		connect(d->TagArea, &StackTagWidget::renameRequested, this, [this](const YSSCore::General::YSSURI& uri) {
			emit renameRequested(uri);
			});
		connect(d->TagArea, &StackTagWidget::saveRequested, this, [this](const YSSCore::General::YSSURI& uri) {
			YSSCore::Editor::FileEditWidget* widget = YSSFSM->getFileEditWidget(uri);
			if (widget) {
				widget->saveFile();
			}
			});
		connect(d->TagArea, &StackTagWidget::saveAsRequested, this, [this](const YSSCore::General::YSSURI& uri) {
			emit saveAsRequested(uri);
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
		d->DragInMsgLabel->setText(VITRL("YSS::editor.stackWidgetArea.dragInFile"));
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
		for (auto widget : d->TagArea->getFileEditWidgets()) {
			if (widget == nullptr) {
				continue;
			}
			widget->disconnect(this);
			widget->disconnect(d->TagArea);
			if (auto textEdit = qobject_cast<YSSCore::Editor::TextEdit*>(widget)) {
				textEdit->setHoverArea(nullptr);
			}
			widget->setParent(nullptr);
			widget->closeFile(true);
		}
		if (AreaRegistry::instance().main() == this) {
			AreaRegistry::instance().setMain(nullptr);
		}
		if (AreaRegistry::instance().current() == this) {
			// This area owned the global current; hand it back so no stale widget
			// keeps the highlight or the bottom info alive.
			setCurrentFocusedWidget(nullptr, nullptr);
		}
		AreaRegistry::instance().release(d->areaID);
		if (not d->closeSilently) {
			emit areaClosed(d->areaID);
		}
		vgDebug << "FileEditWidgetArea destroyed: " << d->areaID;
		delete d;
	}

	void FileEditWidgetArea::setCloseSilently(bool silent) {
		d->closeSilently = silent;
	}

	void FileEditWidgetArea::setAreaID(const QString& areaID) {
		if (AreaRegistry::instance().find(areaID)) {
			return;
		}
		if (not d->areaID.isEmpty()) {
			AreaRegistry::instance().release(d->areaID);
		}
		AreaRegistry::instance().acquire(areaID, this);
		d->areaID = areaID;
	}

	QString FileEditWidgetArea::getAreaID() const {
		return d->areaID;
	}

	FileEditWidgetArea* FileEditWidgetArea::getAreaByID(const QString& areaID) {
		return AreaRegistry::instance().find(areaID);
	}

	FileEditWidgetArea* FileEditWidgetArea::getMainArea() {
		return AreaRegistry::instance().main();
	}

	QList<FileEditWidgetArea*> FileEditWidgetArea::getAllAreas() {
		return AreaRegistry::instance().allAreas();
	}

	void FileEditWidgetArea::changeMainArea(FileEditWidgetArea* newMainArea) {
		AreaRegistry::instance().setMain(newMainArea);
	}

	FileEditWidgetArea* FileEditWidgetArea::getCurrentArea() {
		return AreaRegistry::instance().current();
	}

	YSSCore::Editor::FileEditWidget* FileEditWidgetArea::getCurrentFocusedWidget() {
		return AreaRegistry::instance().currentWidget();
	}

	void FileEditWidgetArea::setCurrentFocusedWidget(FileEditWidgetArea* area, YSSCore::Editor::FileEditWidget* widget) {
		AreaRegistry& registry = AreaRegistry::instance();
		if (registry.current() == area and registry.currentWidget() == widget) {
			return;
		}
		registry.setCurrent(area, widget);
		// The "current" tag highlight is global: only the tag matching the globally
		// focused widget keeps the close button and bottom accent line.
		const YSSCore::General::YSSURI currentUri = widget ? widget->getUri() : YSSCore::General::YSSURI();
		for (FileEditWidgetArea* target : registry.allAreas()) {
			target->d->TagArea->setGlobalCurrentUri(currentUri);
		}
		if (MainWin* win = MainWin::getInstance()) {
			win->onGlobalCurrentChanged(area, widget);
		}
	}

	void FileEditWidgetArea::compressAreaID() {
		QList<FileEditWidgetArea*> areas = getAllAreas();
		if (areas.isEmpty()) {
			return;
		}

		QList<int> ids;
		for (FileEditWidgetArea* area : areas) {
			bool ok = false;
			int id = area->getAreaID().toInt(&ok);
			if (ok and not ids.contains(id)) {
				ids.append(id);
			}
		}
		std::sort(ids.begin(), ids.end());
		QMap<QString, QString> remap;
		bool changed = false;
		for (int i = 0; i < ids.size(); i++) {
			QString oldID = QString::number(ids.at(i));
			QString newID = QString::number(i);
			if (oldID != newID) {
				changed = true;
			}
			remap.insert(oldID, newID);
		}
		if (not changed) {
			return;
		}
		YSSCore::General::YSSProject* project = YSSCore::General::YSSProject::getCurrentProject();
		QMap<QString, QStringList> filesInArea;
		if (project) {
			filesInArea = project->getEditorOpenedFilesInArea();
		}

		for (FileEditWidgetArea* area : areas) {
			area->setAreaID("_tmp_" + area->getAreaID());
		}
		for (FileEditWidgetArea* area : areas) {
			QString oldID = area->getAreaID().mid(5);
			area->setAreaID(remap.value(oldID, oldID));
		}

		if (project) {
			QMap<QString, QStringList> newFilesInArea;
			for (auto it = filesInArea.constBegin(); it != filesInArea.constEnd(); ++it) {
				QString key = it.key();
				if (remap.contains(key)) {
					key = remap.value(key);
				}
				newFilesInArea[key] = it.value();
			}
			project->setEditorOpenedFilesInArea(newFilesInArea);
		}
	}

	void FileEditWidgetArea::addWidget(YSSCore::Editor::FileEditWidget* widget) {
		const YSSCore::General::YSSURI uri = widget->getUri();
		vgDebug << uri.toString();
		if (d->TagArea->containsStackLabel(uri)) {
			setCurrentWidget(uri);
			return;
		}
		widget->setParent(this);
		d->TagArea->addStackLabel(uri, QString(), widget);
		if (widget->isFileChanged()) {
			d->TagArea->setFileChanged(uri);
		}

		connect(widget, &YSSCore::Editor::FileEditWidget::fileChanged, d->TagArea,
			[this](const YSSCore::General::YSSURI& changed) { d->TagArea->setFileChanged(changed); });
		connect(widget, &YSSCore::Editor::FileEditWidget::fileChangeCanceled, d->TagArea,
			[this](const YSSCore::General::YSSURI& changed) { d->TagArea->cancelFileChanged(changed); });
		connect(widget, &YSSCore::Editor::FileEditWidget::fileSaved, d->TagArea,
			[this](const YSSCore::General::YSSURI& saved) { d->TagArea->cancelFileChanged(saved); });
		connect(widget, &YSSCore::Editor::FileEditWidget::fileClosed, this, [this, widget]() {
			const YSSCore::General::YSSURI closedUri = widget->getUri();
			d->TagArea->removeStackLabel(closedUri); // this function handle re-choice if the closed widget is current one
			if (YSSCore::General::YSSProject* project = YSSCore::General::YSSProject::getCurrentProject()) {
				project->removeEditorOpenedFile(toProjectPath(closedUri));
			}
			d->OpenFileCount--;
			if (d->OpenFileCount <= 0) {
				d->OpenFileCount = 0;
				emit allFileClosed();
			}
			});
		connect(widget, &YSSCore::Editor::FileEditWidget::fileRenamed, this,
			[this](const YSSCore::General::YSSURI& oldUri, const YSSCore::General::YSSURI& newUri) {
				d->TagArea->changeStackLabel(oldUri, newUri);
				if (YSSCore::General::YSSProject* project = YSSCore::General::YSSProject::getCurrentProject()) {
					project->removeEditorOpenedFile(toProjectPath(oldUri));
					project->addEditorOpenedFile(toProjectPath(newUri), getAreaID());
				}
			});
		YSSCore::Editor::TextEdit* textEdit = qobject_cast<YSSCore::Editor::TextEdit*>(widget);
		if (textEdit) {
			textEdit->setHoverArea(this->window());
			connect(textEdit, &YSSCore::Editor::TextEdit::cursorPositionChanged, this, [this, textEdit]() {
				emit textEditCursorPositionChanged(textEdit->getUri(), textEdit->getTextCursor());
				});
			YSS::Editor::TextEditConfigOperator::applyTo(textEdit);
		}
		if (YSSCore::General::YSSProject* project = YSSCore::General::YSSProject::getCurrentProject()) {
			project->addEditorOpenedFile(toProjectPath(uri), d->areaID);
		}
		d->OpenFileCount++;
		setCurrentWidget(uri);
	}

	bool FileEditWidgetArea::containsWidget(const YSSCore::General::YSSURI& uri) const {
		return d->TagArea->containsStackLabel(uri);
	}

	void FileEditWidgetArea::closeAll(bool autoGiveup) {
		for (auto widget : d->TagArea->getFileEditWidgets()) {
			vgDebug << "close " << widget->getUri().toString();
			widget->closeFile(autoGiveup);
		}
	}

	void FileEditWidgetArea::closeSaved() {
		for (auto widget : d->TagArea->getFileEditWidgets()) {
			const YSSCore::General::YSSURI uri = widget->getUri();
			if (not widget->isFileChanged() and not d->TagArea->isStackLabelPinned(uri)) {
				vgDebug << "close " << uri.toString();
				widget->closeFile();
			}
		}
	}

	bool FileEditWidgetArea::closeAllWidgetsWithConfirm() {
		QList<YSSCore::General::YSSURI> uris;
		for (auto widget : d->TagArea->getFileEditWidgets()) {
			uris.append(widget->getUri());
		}
		for (const auto& uri : uris) {
			if (YSSCore::Editor::FileEditWidget* widget = YSSFSM->getFileEditWidget(uri)) {
				widget->closeFile();
			}
		}
		for (const auto& uri : uris) {
			if (d->TagArea->containsStackLabel(uri)) {
				return false;
			}
		}
		return true;
	}
	void FileEditWidgetArea::closeWidget(YSSCore::Editor::FileEditWidget* widget) {
		closeWidget(widget->getUri());
	}

	void FileEditWidgetArea::closeWidget(const YSSCore::General::YSSURI& uri) {
		YSSCore::Editor::FileEditWidget* widget = YSSFSM->getFileEditWidget(uri);
		if (not widget) {
			return;
		}
		widget->closeFile();
	}

	void FileEditWidgetArea::setCurrentWidget(YSSCore::Editor::FileEditWidget* widget) {
		if (widget) {
			setCurrentWidget(widget->getUri());
		}
	}

	void FileEditWidgetArea::setCurrentWidget(const YSSCore::General::YSSURI& uri) {
		if (uri.isValid() and uri.toIdentityKey() == getCurrentWidgetUri().toIdentityKey()) {
			return;
		}
		if (not uri.isValid()) {
			d->ContentArea->hide();
			d->Layout->removeWidget(d->ContentArea);
			d->ContentArea = d->CentralArea;
			d->Layout->insertWidget(1, d->ContentArea);
			d->ContentArea->show();
			emit currentFileChanged(YSSCore::General::YSSURI());
			return;
		}
		YSSCore::Editor::FileEditWidget* widget = YSSFSM->getFileEditWidget(uri);
		vgDebug << "setCurrentWidget: " << uri.toString() << " widget: " << widget;
		if (not widget) {
			vgErrorF << "File uri not found in FileServerManager: " << uri.toString();
			return;
		}
		if (d->ContentArea != widget) {
			d->ContentArea->hide();
			d->Layout->removeWidget(d->ContentArea);
			d->ContentArea = widget;
			d->Layout->insertWidget(1, d->ContentArea);
			d->ContentArea->show();
		}
		if (YSSCore::General::YSSProject* project = YSSCore::General::YSSProject::getCurrentProject()) {
			project->setFocusedFile(toProjectPath(uri));
		}
		emit currentFileChanged(uri);
		d->TagArea->setCurrentStackLabel(uri);
	}

	void FileEditWidgetArea::setCurrentWidget(YSSCore::Editor::FileEditWidget* widget, qint32 lineNumber, qint32 column) {
		setCurrentWidget(widget->getUri(), lineNumber, column);
	}

	void FileEditWidgetArea::setCurrentWidget(const YSSCore::General::YSSURI& uri, qint32 lineNumber, qint32 column) {
		if (not uri.isValid()) {
			d->ContentArea->hide();
			d->Layout->removeWidget(d->ContentArea);
			d->ContentArea = d->CentralArea;
			d->Layout->insertWidget(1, d->ContentArea);
			d->ContentArea->show();
		}
		YSSCore::Editor::FileEditWidget* widget = YSSFSM->getFileEditWidget(uri);
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

	YSSCore::General::YSSURI FileEditWidgetArea::getCurrentWidgetUri() const {
		auto widget = getCurrentWidget();
		if (widget) {
			return widget->getUri();
		}
		return YSSCore::General::YSSURI();
	}

	void FileEditWidgetArea::moveWidgetTo(const YSSCore::General::YSSURI& uri, FileEditWidgetArea* otherArea) {
		const bool pinned = d->TagArea->isStackLabelPinned(uri);
		d->TagArea->removeStackLabel(uri);
		YSSCore::Editor::FileEditWidget* widget = YSSFSM->getFileEditWidget(uri);
		if (YSSCore::General::YSSProject* project = YSSCore::General::YSSProject::getCurrentProject()) {
			project->removeEditorOpenedFile(toProjectPath(uri));
		}
		if (widget) {
			widget->disconnect(this);
			widget->disconnect(d->TagArea);
			otherArea->addWidget(widget);
			if (pinned) {
				otherArea->d->TagArea->setStackLabelPinned(uri, true);
			}
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
		if (StackTag::canAcceptDrag(event->mimeData())) {
			StackTag* selfLabel = StackTag::draggedTag(event->mimeData());
			// The drag source serializes the "area" key: the StackTagWidget the dragged
			// tag stays in (i.e. the SOURCE tag bar), not "parent".
			StackTagWidget* sourceTagArea = StackTag::draggedTagArea(event->mimeData());
			if (selfLabel && sourceTagArea && sourceTagArea != d->TagArea) {
				FileEditWidgetArea* from = sourceTagArea->getArea();
				if (from && from != this) {
					from->moveWidgetTo(selfLabel->getUri(), this);
				}
			}
		}
		d->DragInMsgLabel->hide();
	}

	void FileEditWidgetArea::resizeEvent(QResizeEvent* event) {
		QFrame::resizeEvent(event);

	}
}