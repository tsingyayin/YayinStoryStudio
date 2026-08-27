#include "Editor/MainEditor/private/StackComponents_p.h"
#include "Editor/MainEditor/FileEditWidgetArea.h"
#include <QtCore/qfileinfo.h>
#include <Editor/DocumentMessageManager.h>
#include <General/TranslationHost.h>
#include <Editor/SyntaxHighlighter.h>
#include <General/Log.h>
#include <QtWidgets/qheaderview.h>
#include <QtWidgets/qscrollbar.h>
#include <Utility/FileUtility.h>
#include <Editor/FileEditWidget.h>
#include <Editor/FileServerManager.h>
#include <General/VIApplication.h>
#include <QtGui/qpainter.h>
#include <Editor/VirtualFilePath.h>
#include <QtCore/qmimedata.h>
#include <QtGui/qdrag.h>
namespace YSS::Editor {
	const QString StackTag::stackTagDragMimeType = QString("application/x-yss-stacktag");

	StackTag::StackTag(QWidget* parent, bool toolWidgetMode, Qt::Orientation orientation) :QFrame(parent) {
		Orientation = orientation;
		this->setFixedWidth(200);
		TitleLabel = new QLabel(this);
		TitleLabel->setContentsMargins(5, 0, 5, 0);
		PinLabel = new QToolButton(this);
		//PinLabel->setIcon(QIcon(":/resource/cn.yxgeneral.yayinstorystudio/icon/pin.png"));
		CloseLabel = new QToolButton(this);
		//CloseLabel->setIcon(QIcon(":/resource/cn.yxgeneral.yayinstorystudio/icon/close.png"));
		this->setContentsMargins(0, 0, 0, 0);
		Layout = new QHBoxLayout(this);
		Layout->setContentsMargins(0, 0, 0, 0);
		Layout->setSpacing(0);
		Layout->addWidget(TitleLabel);
		Layout->addWidget(PinLabel);
		Layout->addWidget(CloseLabel);
		PinLabel->hide();
		CloseLabel->hide();

		connect(PinLabel, &QToolButton::clicked, this, [this]() {
			setPinned(!isPinned());
			emit pinClicked(FilePath);
			});

		connect(CloseLabel, &QToolButton::clicked, this, [this]() {
			emit closeClicked(FilePath);
			});

		setContextMenuPolicy(Qt::ActionsContextMenu);
		ActionClose = new QAction(VITR("Visindigo::general.close"), this);
		ActionPin = new QAction(VITR("Visindigo::general.pin"), this);
		ActionCloseAll = new QAction(VITR("YSS::menu.file.closeAll"), this);

		connect(ActionClose, &QAction::triggered, this, [this]() {
			emit closeClicked(FilePath);
			});
		connect(ActionPin, &QAction::triggered, this, [this]() {
			setPinned(!isPinned());
			emit pinClicked(FilePath);
			});
		connect(ActionCloseAll, &QAction::triggered, this, [this]() {
			emit closeAllRequested();
			});

		this->toolWidgetMode = toolWidgetMode;
		ActionReload = new QAction(VITR("Visindigo::general.reload"), this);
		ActionRename = new QAction(VITR("Visindigo::general.rename"), this);
		ActionSave = new QAction(VITR("Visindigo::general.save"), this);
		ActionSaveAs = new QAction(VITR("Visindigo::general.saveAs"), this);
		ActionShowInExplorer = new QAction(VITR("YSS::menu.file.showInExplorer"), this);
		ActionCloseSaved = new QAction(VITR("YSS::menu.file.closeSaved"), this);
		connect(ActionReload, &QAction::triggered, this, [this]() {
			YSSCore::Editor::FileEditWidget* editor = YSSFSM->getFileEditWidget(FilePath);
			if (editor) {
				editor->reloadFile();
			}
			});
		connect(ActionRename, &QAction::triggered, this, [this]() {
			emit renameRequested(FilePath);
			});
		connect(ActionSave, &QAction::triggered, this, [this]() {
			emit saveRequested(FilePath);
			});
		connect(ActionSaveAs, &QAction::triggered, this, [this]() {
			emit saveAsRequested(FilePath);
			});
		connect(ActionShowInExplorer, &QAction::triggered, this, [this]() {
			Visindigo::Utility::FileUtility::openExplorer(FilePath);
			});
		connect(ActionCloseSaved, &QAction::triggered, this, [this]() {
			emit closeSavedRequested();
			});

		if (not toolWidgetMode) {
			this->addActions({ ActionClose, ActionPin, ActionReload,
				ActionRename, ActionSave, ActionSaveAs, ActionShowInExplorer,
				ActionCloseAll, ActionCloseSaved });
		}
		else {
			this->TitleLabel->setAlignment(Qt::AlignCenter);
			this->addActions({ ActionClose, ActionPin, ActionCloseAll });
		}
	}

	void StackTag::setStayInWidget(StackTagWidget* widget) {
		StayInWidget = widget; // not parent. StackTag`s parent should be a scrollArea in StackTagWidget, not StackTagWidget itself.
	}

	StackTagWidget* StackTag::getStayInWidget() const {
		return StayInWidget;
	}

	void StackTag::setText(const QString& text) {
		TitleLabel->setText(text);
		if (toolWidgetMode && Orientation == Qt::Horizontal) {
			qint32 fixedWidth = TitleLabel->fontMetrics().horizontalAdvance(text) + 40;
			if (fixedWidth < 100) {
				fixedWidth = 100;
			}
			this->setFixedWidth(fixedWidth);
		}
	}

	void StackTag::setFilePath(const QString& filePath) {
		FilePath = filePath;
		// Virtual files have a virtual save but no virtual "save as", so disable
		// the actions that don't apply to them.
		const bool virtualFile = YSSCore::Editor::VirtualFilePath::isVirtualFilePath(filePath);
		ActionRename->setEnabled(!virtualFile);
		ActionSaveAs->setEnabled(!virtualFile);
		ActionShowInExplorer->setEnabled(!virtualFile);
	}

	QString StackTag::getFilePath() const {
		return FilePath;
	}

	QString StackTag::getText() const {
		return TitleLabel->text();
	}

	void StackTag::setFocusOn(bool focus) {
		Focused = focus;
		repaint();
	}

	bool StackTag::isFocusOn() const {
		return Focused;
	}

	void StackTag::updateToolTip() {
		const int textWidth = QFontMetrics(TitleLabel->font()).horizontalAdvance(TitleLabel->text());
		setToolTip(textWidth > TitleLabel->width() ? TitleLabel->text() : QString());
	}

	void StackTag::setPinned(bool pinned) {
		if (Pinned == pinned) {
			return;
		}
		Pinned = pinned;
		if (Pinned) {
			//PinLabel->show();
			PinLabel->setIcon(VIApp->getFontIcon("\uE841\uE840", 64, { VISTM->getPaletteAccentColor(), 
				VISTM->getPaletteTextColor() }));
		}
		else {
			//PinLabel->hide();
			PinLabel->setIcon(VIApp->getFontIcon("\uE840", 64, { VISTM->getPaletteTextColor() }));
		}
	}

	bool StackTag::isPinned() const {
		return Pinned;
	}

	void StackTag::mousePressEvent(QMouseEvent* event) {
		QFrame::mousePressEvent(event);
		if (event->button() == Qt::LeftButton) {
			Pressed = true;
			PressedPos = event->pos();
		}
	}

	void StackTag::mouseReleaseEvent(QMouseEvent* event) {
		QFrame::mouseReleaseEvent(event);
		if (Pressed && event->button() == Qt::LeftButton) {
			Pressed = false;
			emit clicked(FilePath);
		}
	}
	
	void StackTag::mouseMoveEvent(QMouseEvent* event) {
		QFrame::mouseMoveEvent(event);
		if (Pressed) {
			QPoint currentPos = event->pos();
			if ((currentPos - PressedPos).manhattanLength() > QApplication::startDragDistance()) {
				Pressed = false;
				QDrag* drag = new QDrag(this);
				QImage dragImage = this->grab().toImage();
				drag->setPixmap(QPixmap::fromImage(dragImage));
				QMimeData* mimeData = new QMimeData;
				quint64 self = reinterpret_cast<quint64>(this);
				QString selfPtr = QString::number(self, 16);
				quint64 area = reinterpret_cast<quint64>(StayInWidget);
				QString areaPtr = QString::number(area, 16);
				Visindigo::Utility::JsonConfig json;
				json.setString("self", selfPtr);
				json.setString("area", areaPtr);
				mimeData->setData(StackTag::stackTagDragMimeType, json.toString().toUtf8());
				drag->setMimeData(mimeData);
				drag->exec(Qt::MoveAction);
			}
		}
	}

	void StackTag::resizeEvent(QResizeEvent* event) {
		QFrame::resizeEvent(event);
		PinLabel->setFixedHeight(this->height() - 2);
		CloseLabel->setFixedHeight(this->height() - 2);
		PinLabel->setFixedWidth(PinLabel->height());
		CloseLabel->setFixedWidth(CloseLabel->height());
		updateToolTip();
	}

	void StackTag::enterEvent(QEnterEvent* event) {
		QFrame::enterEvent(event);
		Hovering = true;
		PinLabel->show();
		CloseLabel->show();
		updateToolTip();
	}

	void StackTag::leaveEvent(QEvent* event) {
		QFrame::leaveEvent(event);
		Hovering = false;
		if (not Pinned) {
			PinLabel->hide();
		}
		CloseLabel->hide();
		updateToolTip();
	}

	void StackTag::paintEvent(QPaintEvent* event) {
		QFrame::paintEvent(event);
		QStyleOptionButton option;
		option.initFrom(this);
		if (Pressed || Focused) {
			option.state |= QStyle::State_MouseOver;
		}else if (Hovering) {
			option.state |= QStyle::State_Raised;
		}
		else {
			option.state |= QStyle::State_Sunken;
		}
		QPainter painter(this);
		QStyle* style = this->style();
		style->drawControl(QStyle::CE_PushButton, &option, &painter, this);
		if (Pressed || Focused) {
			QPen pen(VISTM->getPaletteAccentColor());
			pen.setWidth(2);
			pen.setCapStyle(Qt::RoundCap);
			painter.setPen(pen);
			if (Orientation == Qt::Horizontal) {
				painter.drawLine(6, this->height() - 3, this->width() - 6, this->height() - 3); // bottom line
			}
			else {
				painter.drawLine(3, 6, 3, this->height() - 6); // left line
			}
		}
		TitleLabel->setPalette(qApp->palette());
	}

	StackTagWidget::StackTagWidget(QWidget* parent, Qt::Orientation orientation) :QFrame(parent) {
		// NOTE: must use setAcceptDrops(true); acceptDrops() is only a getter
		// (returns bool) and does NOT enable drop acceptance.
		this->setAcceptDrops(true);
		Orientation = orientation;
		ScrollContent = new QWidget(this);
		ScrollContent->setObjectName("StackTagScrollContent");
		ScrollContent->setStyleSheet("QWidget#StackTagScrollContent { background: transparent; }");
		if (orientation == Qt::Horizontal) {
			ContentLayout = new QHBoxLayout(ScrollContent);
		}
		else {
			ContentLayout = new QVBoxLayout(ScrollContent);
		}
		ContentLayout->setContentsMargins(0, 0, 0, 0);
		ContentLayout->setSpacing(0);
		ScrollContent->setLayout(ContentLayout);
		ScrollArea = new QScrollArea(this);
		ScrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");
		ScrollArea->setContentsMargins(0, 0, 0, 0);
		ScrollArea->setWidget(ScrollContent);
		ScrollArea->setWidgetResizable(true);
		ScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		ScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		adjustScrollArea();
		WidgetSelector = new QComboBox(this);
		if (orientation == Qt::Horizontal) {
			WidgetSelector->setMinimumWidth(200);
			Layout = new QHBoxLayout(this);
			WidgetSelector->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
		}
		else {
			WidgetSelector->setMinimumHeight(32);
			Layout = new QVBoxLayout(this);
			WidgetSelector->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
		}
		Layout->setContentsMargins(0, 0, 0, 0);
		Layout->addWidget(ScrollArea);
		Layout->addWidget(WidgetSelector);
		connect(WidgetSelector, &QComboBox::currentIndexChanged, this, [this](int index) {
			QString filePath = WidgetSelector->itemData(index).toString();
			setCurrentStackLabel(filePath);
			emit switchToFile(filePath);
			});
		connect(VISTM, &Visindigo::Widgets::ThemeManager::programThemeChanged, this, [this](const QString& themeID) {
			auto textColor = VISTM->getPaletteTextColor();
			auto accentColor = VISTM->getPaletteAccentColor();
			applyIcon(nullptr, textColor, accentColor);
			});
		setColorfulEnable(true);
		onThemeChanged();
	}

	StackTagWidget::~StackTagWidget() {}

	void StackTagWidget::setArea(FileEditWidgetArea* area) {
		Area = area;
	}

	FileEditWidgetArea* StackTagWidget::getArea() const {
		return Area;
	}

	void StackTagWidget::applyIcon(StackTag* label, const QColor& textColor, const QColor& accentColor) {
		QIcon pinIcon = VIApp->getFontIcon("\uE840", 64, { textColor });
		QIcon pinnedIcon = VIApp->getFontIcon("\uE841\uE840", 64, { accentColor, textColor });
		QIcon closeIcon = VIApp->getFontIcon("\uE711", 64, { textColor });
		QIcon reloadIcon = VIApp->getFontIcon("\uE895", 64, { textColor });
		QIcon renameIcon = VIApp->getFontIcon("\uE8AC", 64, { textColor });
		QIcon saveIcon = VIApp->getNamedFontIcon("Save", 64, { textColor });
		QIcon saveAsIcon = VIApp->getFontIcon("\uE792", 64, { textColor });
		QIcon showInExplorerIcon = VIApp->getFontIcon("\uE8A7", 64, { textColor });
		QIcon closeAllIcon = VIApp->getFontIcon("\uEA39", 64, { textColor});
		QIcon closeSavedIcon = VIApp->getFontIcon("\uE711", 64, { textColor });
		QList<StackTag*> targets;
		if (not label) {
			targets = Labels;
		}
		else {
			targets.append(label);
		}
		for (auto label : targets) {
			label->CloseLabel->setIcon(closeIcon);
			label->PinLabel->setIcon(label->isPinned() ? pinnedIcon : pinIcon);
			label->ActionClose->setIcon(closeIcon);
			label->ActionPin->setIcon(label->isPinned() ? pinnedIcon : pinIcon);
			label->ActionReload->setIcon(reloadIcon);
			label->ActionRename->setIcon(renameIcon);
			label->ActionSave->setIcon(saveIcon);
			label->ActionSaveAs->setIcon(saveAsIcon);
			label->ActionShowInExplorer->setIcon(showInExplorerIcon);
			label->ActionCloseAll->setIcon(closeAllIcon);
			label->ActionCloseSaved->setIcon(closeSavedIcon); 
		}
	}

	void StackTagWidget::addStackLabel(const QString& filePath, const QString& displayName) {
		QFileInfo fileInfo(filePath);
		StackTag* tagLabel = new StackTag(ScrollContent, ToolWidgetMode, Orientation);
		tagLabel->setFilePath(filePath);
		tagLabel->setStayInWidget(this);
		const QString text = displayName.isEmpty() ? fileInfo.fileName() : displayName;
		tagLabel->setText(text);
		WidgetSelector->addItem(text, filePath);
		ContentLayout->addWidget(tagLabel);
		Labels.append(tagLabel);

		connect(tagLabel, &StackTag::pinClicked, this, [this](const QString& filePath) {
			pinStackLabel(filePath);
			});
		connect(tagLabel, &StackTag::closeClicked, this, [this](const QString& filePath) {
			emit closeFile(filePath); // NOTICE: close should be handled by some dialog, cannot directly do any other operation here.
			});
		connect(tagLabel, &StackTag::clicked, this, [this](const QString& filePath) {
			emit switchToFile(filePath);
			});
		connect(tagLabel, &StackTag::renameRequested, this, [this](const QString& filePath) {
			emit renameRequested(filePath);
			});
		connect(tagLabel, &StackTag::saveAsRequested, this, [this](const QString& filePath) {
			emit saveAsRequested(filePath);
			});
		connect(tagLabel, &StackTag::closeAllRequested, this, [this]() {
			emit closeAllRequested();
			});
		connect(tagLabel, &StackTag::closeSavedRequested, this, [this]() {
			emit closeSavedRequested();
			});
		connect(tagLabel, &StackTag::saveRequested, this, [this](const QString& filePath) {
			emit saveRequested(filePath);
			});
		ScrollArea->horizontalScrollBar()->setMaximum(Labels.size() * Labels.last()->width() - ScrollArea->width());
		/*tagLabel->setStyleSheet(VISTMGT("YSS::Editor.StackTag.Normal"),
			VISTMGT("YSS::Editor.StackTag.Hover"),
			VISTMGT("YSS::Editor.StackTag.Pressed"));*/
		auto textColor = VISTM->getPaletteTextColor();
		auto accentColor = VISTM->getPaletteAccentColor();
		applyIcon(tagLabel, textColor, accentColor);
		adjustScrollArea();
		
	}

	void StackTagWidget::changeStackLabel(const QString& oldFilePath, const QString& newFilePath, const QString& newDisplayName) {
		StackTag* label = findLabel(oldFilePath);
		if (not label) {
			return;
		}
		const QString text = newDisplayName.isEmpty() ? QFileInfo(newFilePath).fileName() : newDisplayName;
		label->setText(text);
		label->setFilePath(newFilePath);
		for (int i = 0; i < WidgetSelector->count(); ++i) {
			if (WidgetSelector->itemData(i).toString() == oldFilePath) {
				WidgetSelector->setItemText(i, text);
				WidgetSelector->setItemData(i, newFilePath);
				break;
			}
		}
	}

	void StackTagWidget::pinStackLabel(const QString& filePath) {
		StackTag* targetLabel = findLabel(filePath);
		// NOTICE:
		// This move-afterward operation working in two situations:
		// 1. When pinning, it will move the label to the leftmost of all unpinned labels,
		// which is the most intuitive way to do it.
		// 2. When unpinning, it will move the label to the rightmost of all pinned labels,
		// Fortunately, these two situations are actually the same operation,
		// which is to move the label to the boundary of pinned and unpinned labels.
		if (targetLabel) {
			Labels.removeAll(targetLabel);
			ContentLayout->removeWidget(targetLabel);
			for (int i = 0; i < Labels.size(); ++i) {
				if (not Labels[i]->isPinned()) {
					Labels.insert(i, targetLabel);
					ContentLayout->insertWidget(i, targetLabel);
					return;
				}
			}
			// All pinned.
			Labels.append(targetLabel);
			ContentLayout->addWidget(targetLabel);
		}
	}

	void StackTagWidget::removeStackLabel(const QString& filePath) {
		StackTag* targetLabel = findLabel(filePath);
		int index = -1;
		vgDebug << "Attempting to remove stack label with file path: " << filePath;
		if (targetLabel) {
			index = Labels.indexOf(targetLabel);
			vgDebug << targetLabel;
			Labels.removeAll(targetLabel);
			ContentLayout->removeWidget(targetLabel);
			targetLabel->deleteLater();
			for (int i = 0; i < WidgetSelector->count(); ++i) {
				if (WidgetSelector->itemData(i).toString() == filePath) {
					WidgetSelector->removeItem(i);
					break;
				}
			}
		}
		if (CurrentSelected == filePath) {
			// Prefer the label that slid into the removed slot (its successor);
			// if none exists, fall back to the first remaining label.
			StackTag* next = nullptr;
			if (index > 1 && index < Labels.size()) {
				next = Labels[index];
			}
			else if (not Labels.isEmpty()) {
				next = Labels.first();
			}
			if (next) {
				setCurrentStackLabel(next->getFilePath());
				emit switchToFile(next->getFilePath());
			}
			else {
				CurrentSelected = "";
				emit switchToFile("");
			}
		}
		adjustScrollArea();
	}

	void StackTagWidget::setCurrentStackLabel(const QString& filePath) {
		if (filePath == CurrentSelected) {
			return;
		}
		bool finded = false;
		int i = 0;
		int cache = i;
		for (StackTag* label : Labels) {
			if (label->getFilePath() == filePath) {
				label->setFocusOn(true);
				finded = true;
				CurrentSelected = filePath;
				cache = i;
			}
			else {
				label->setFocusOn(false);
			}
			i++;
		}
		if (finded) {
			for (int i = 0; i < WidgetSelector->count(); ++i) {
				if (WidgetSelector->itemData(i).toString() == filePath) {
					WidgetSelector->setCurrentIndex(i);
					break;
				}
			}
			if (Orientation == Qt::Horizontal) {
				ScrollArea->horizontalScrollBar()->setValue(cache * Labels.last()->width());
			}
			else {
				ScrollArea->verticalScrollBar()->setValue(cache * Labels.last()->height());
			}
		}
	}

	QString StackTagWidget::getCurrentSelected() const {
		return CurrentSelected;
	}

	void StackTagWidget::adjustScrollArea() {
		if (Orientation == Qt::Horizontal) {
			if (Labels.size() == 0) {
				ScrollContent->setFixedWidth(0);
				return;
			}
			int totalWidth = 0;
			for (StackTag* label : Labels) {
				label->setFixedHeight(this->height());
				totalWidth += label->width() + ContentLayout->spacing();
			}
			totalWidth -= ContentLayout->spacing();
			ScrollContent->setFixedWidth(totalWidth);
		}
		else {
			if (Labels.size() == 0) {
				ScrollContent->setFixedHeight(0);
				return;
			}
			int totalHeight = 0;
			for (StackTag* label : Labels) {
				label->setFixedWidth(this->width());
				totalHeight += label->height() + ContentLayout->spacing();
			}
			totalHeight -= ContentLayout->spacing();
			ScrollContent->setFixedHeight(totalHeight);
		}
	}

	void StackTagWidget::setFileChanged(const QString& path) {
		StackTag* label = findLabel(path);
		if (not label) {
			return;
		}
		const QString fileName = label->getText();
		if (not fileName.startsWith("* ")) {
			label->setText("* " + fileName);
		}
	}

	void StackTagWidget::cancelFileChanged(const QString& path) {
		StackTag* label = findLabel(path);
		if (not label) {
			return;
		}
		const QString fileName = label->getText();
		if (fileName.startsWith("* ")) {
			label->setText(fileName.mid(2));
		}
	}

	bool StackTagWidget::containsStackLabel(const QString& filePath) const {
		return findLabel(filePath) != nullptr;
	}

	bool StackTagWidget::isStackLabelPinned(const QString& filePath) const {
		StackTag* label = findLabel(filePath);
		return label ? label->isPinned() : false;
	}

	StackTag* StackTagWidget::findLabel(const QString& filePath) const {
		for (StackTag* label : Labels) {
			if (label->getFilePath() == filePath) {
				return label;
			}
		}
		return nullptr;
	}

	void StackTagWidget::setToolWidgetMode(bool toolWidgetMode) {
		ToolWidgetMode = toolWidgetMode;
		if (ToolWidgetMode) {
			WidgetSelector->hide();
		}
		else {
			WidgetSelector->show();
		}
	}

	bool StackTagWidget::isToolWidgetMode() const {
		return ToolWidgetMode;
	}

	void StackTagWidget::wheelEvent(QWheelEvent* event) {
		QFrame::wheelEvent(event);
		int numDegrees = event->angleDelta().y() / 8;
		int numSteps = numDegrees / 15;
		if (Orientation == Qt::Vertical) {
			int stepSize = Labels.size() > 0 ? Labels[0]->height() + ContentLayout->spacing() : 32;
			ScrollArea->verticalScrollBar()->setValue(ScrollArea->verticalScrollBar()->value() - numSteps * stepSize);
		}
		else {
			int stepSize = Labels.size() > 0 ? Labels[0]->width() + ContentLayout->spacing() : 100;
			ScrollArea->horizontalScrollBar()->setValue(ScrollArea->horizontalScrollBar()->value() - numSteps * stepSize);
		}
	}

	void StackTagWidget::onThemeChanged() {

	}

	void StackTagWidget::resizeEvent(QResizeEvent* event) {
		QFrame::resizeEvent(event);
		if (Orientation == Qt::Horizontal) {
			WidgetSelector->setFixedHeight(this->height());
		}else{
			WidgetSelector->setFixedWidth(this->width());
		}
		adjustScrollArea();
	}

	void StackTagWidget::dragEnterEvent(QDragEnterEvent* event) {
		if (event->mimeData()->hasFormat(StackTag::stackTagDragMimeType)) {
			event->acceptProposedAction();
		}
	}

	void StackTagWidget::dragMoveEvent(QDragMoveEvent* event) {
		if (event->mimeData()->hasFormat(StackTag::stackTagDragMimeType)) {
			event->acceptProposedAction();
		}
	}

	void StackTagWidget::dropEvent(QDropEvent* event) {
		if (event->mimeData()->hasFormat(StackTag::stackTagDragMimeType)) {
			event->acceptProposedAction();
			QByteArray data = event->mimeData()->data(StackTag::stackTagDragMimeType);
			Visindigo::Utility::JsonConfig json(QString::fromUtf8(data));
			QString selfPtr = json.getString("self");
			StackTag* selfLabel = reinterpret_cast<StackTag*>(selfPtr.toULongLong(nullptr, 16));
			if (Labels.contains(selfLabel)) {
				int index = ContentLayout->indexOf(selfLabel);
				if (index != -1) {
					// event->position() is relative to this widget; map it into
					// ScrollContent's coordinate space, where every label's geometry()
					// lives, so all comparisons below share one consistent system.
					QPoint dropPos = ScrollContent->mapFrom(this, event->position().toPoint());
					// Dropped inside the dragged label's own area -> no reorder.
					if (selfLabel->geometry().contains(dropPos)) {
						event->accept();
						return;
					}
					
					int newIndex = Labels.size();
					for (int i = 0; i < Labels.size(); ++i) {
						const QPoint center = Labels[i]->geometry().center();
						const int dropCoord = Orientation == Qt::Horizontal ? dropPos.x() : dropPos.y();
						const int centerCoord = Orientation == Qt::Horizontal ? center.x() : center.y();
						if (dropCoord < centerCoord) {
							newIndex = i;
							break;
						}
					}
					StackTag* labelAtNewIndex = Labels.value(newIndex, nullptr);
					if (labelAtNewIndex) {
						if (labelAtNewIndex->isPinned() != selfLabel->isPinned()) {
							event->accept();
							return; // Don't allow moving a pinned label to an unpinned position or vice versa.
						}
						ContentLayout->removeWidget(selfLabel);
						Labels.removeAll(selfLabel);
						ContentLayout->insertWidget(newIndex, selfLabel);
						Labels.insert(newIndex, selfLabel);
					}
					else {
						if (Labels.last()->isPinned() != selfLabel->isPinned()) {
							event->accept();
							return; // Don't allow moving a pinned label to an unpinned position or vice versa.
						}
						ContentLayout->removeWidget(selfLabel);
						Labels.removeAll(selfLabel);
						ContentLayout->addWidget(selfLabel);
						Labels.append(selfLabel);
					}
				}
			}
			else if (selfLabel) {
				// Cross-area: a tag dragged from another StackTagWidget is dropped
				// on this tag bar -> move it from its source area into this area.
				QString sourcePtr = json.getString("area");
				StackTagWidget* sourceTagArea = reinterpret_cast<StackTagWidget*>(sourcePtr.toULongLong(nullptr, 16));
				FileEditWidgetArea* sourceArea = sourceTagArea ? sourceTagArea->getArea() : nullptr;
				if (sourceArea && Area && sourceArea != Area) {
					sourceArea->moveWidgetTo(selfLabel->getFilePath(), Area);
				}
			}
			event->accept();
		}
	}

	DefaultStackWidgetCentralArea::DefaultStackWidgetCentralArea(QWidget* parent) :QFrame(parent) {
		ContentLabel = new QLabel(this);
		Layout = new QGridLayout(this);
		this->setLayout(Layout);
		Layout->addWidget(ContentLabel, 0, 0, Qt::AlignCenter);
		ContentLabel->setAlignment(Qt::AlignCenter);
	}

	void DefaultStackWidgetCentralArea::setText(const QString& text) {
		ContentLabel->setText(text);
	}
}