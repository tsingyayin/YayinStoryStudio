#include "Editor/MainEditor/TreeLayoutWidget.h"
#include <QtWidgets/qsplitter.h>
#include <QtWidgets/qapplication.h>
#include <QtCore/qmimedata.h>
#include <QtCore/qtimer.h>
#include <QtCore/qpair.h>
#include <QtCore/qrect.h>
#include <QtCore/qpoint.h>
#include <QtGui/qevent.h>
#include <QtGui/qpolygon.h>
#include <QtGui/qscreen.h>
#include <Widgets/ThemeManager.h>
#include "Editor/MainEditor/FileEditWidgetArea.h"
#include "Editor/MainEditor/private/StackComponents_p.h"
#include "Editor/MainEditor/private/TreeLayoutWidget_p.h"
namespace YSS::Editor {
	class TreeLayoutWidgetPrivate {
		friend class TreeLayoutWidget;
		friend class TreeLayoutDropZone;
	protected:
		static QList<TreeLayoutWidget*> AllTopLevelLayouts;
		TreeLayoutWidget* q;
		QSplitter* Container = nullptr;
		QList<QWidget*> Childrens;
		QList<bool> IsLayout;
		Qt::Orientation Orientation = Qt::Horizontal;
		bool notSelectOrientation = true;
		TreeLayoutWidget* ParentLayout = nullptr;
		QTimer* HideTimer = nullptr;
		QList<TreeLayoutDropZone*> DropZones;
	protected:
		TreeLayoutWidgetPrivate(TreeLayoutWidget* owner) :q(owner) {
			HideTimer = new QTimer(q);
			HideTimer->setSingleShot(true);
			HideTimer->setInterval(200);
			QObject::connect(HideTimer, &QTimer::timeout, q, [this]() {
				hideDropIndicators();
				});
		}
	protected:
		void relayout();
		void balanceNewChild(int index);
		void handleAllFileClosed(FileEditWidgetArea* area);
		void insertChildAt(int index, QWidget* child, bool isLayout);
		QWidget* removeChildAt(int index);
		TreeLayoutWidget* replaceAreaWithNewLayoutAt(int index, FileEditWidgetArea* newArea, bool newFirst);
		TreeLayoutWidget* collapseLayoutAt(int index);
		void clearAllChildren();
		void onChildClosed(QWidget* child);
		double computePercent(int index) const;
		void writeChildrenToList(QList<Visindigo::Utility::JsonConfig>& out) const;
		void buildStructureFromJson(const QList<Visindigo::Utility::JsonConfig>& children, QList<QPair<FileEditWidgetArea*, QString>>& recovered);
		void applyPercentsTopDown(const QList<Visindigo::Utility::JsonConfig>& children);
		void applyChildrenPercents(const QList<Visindigo::Utility::JsonConfig>& children);
		void collectAreas(QList<FileEditWidgetArea*>& out) const;
		void showDropIndicators();
		void hideDropIndicators();
		void scheduleHideDropIndicators();
		void handleDrop(TreeLayoutDropZone::DropZone zone, int childIndex, QDropEvent* event);
	};

	QList<TreeLayoutWidget*> TreeLayoutWidgetPrivate::AllTopLevelLayouts;

	TreeLayoutDropZone::TreeLayoutDropZone(TreeLayoutWidgetPrivate* owner, DropZone zone, int childIndex)
		:QWidget(owner->q), Owner(owner), Zone(zone), ChildIndex(childIndex) {
		setAcceptDrops(true);
		setAttribute(Qt::WA_TransparentForMouseEvents, false);
		hide();
	}
	void TreeLayoutDropZone::setZoneShape(const QPainterPath& shape) {
		Shape = shape;
		setMask(QRegion(shape.toFillPolygon().toPolygon()));
		update();
	}

	void TreeLayoutDropZone::paintEvent(QPaintEvent* event) {
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing, true);
		QColor accent = VISTM->getPaletteAccentColor();
		if (Hovered) {
			QColor light = accent.lighter(150);
			QColor penColor = light;
			penColor.setAlpha(255);
			QColor brushColor = light;
			brushColor.setAlpha(130);
			painter.setPen(QPen(penColor, 2));
			painter.setBrush(brushColor);
		}
		else {
			QColor dark = accent.darker(150);
			QColor penColor = dark;
			penColor.setAlpha(200);
			QColor brushColor = dark;
			brushColor.setAlpha(60);
			painter.setPen(QPen(penColor, 2));
			painter.setBrush(brushColor);
		}
		painter.drawPath(Shape);
	}

	void TreeLayoutDropZone::dragEnterEvent(QDragEnterEvent* event) {
		if (StackTag::canAcceptDrag(event->mimeData())) {
			event->acceptProposedAction();
			setHovered(true);
			Owner->showDropIndicators();
		}
	}

	void TreeLayoutDropZone::dragMoveEvent(QDragMoveEvent* event) {
		if (StackTag::canAcceptDrag(event->mimeData())) {
			event->acceptProposedAction();
			setHovered(true);
		}
	}

	void TreeLayoutDropZone::dragLeaveEvent(QDragLeaveEvent* event) {
		QWidget::dragLeaveEvent(event);
		setHovered(false);
		Owner->scheduleHideDropIndicators();
	}

	void TreeLayoutDropZone::dropEvent(QDropEvent* event) {
		Owner->handleDrop(Zone, ChildIndex, event);
	}

	void TreeLayoutDropZone::setHovered(bool hovered) {
		if (Hovered == hovered) {
			return;
		}
		Hovered = hovered;
		update();
	}

	QList<TreeLayoutWidget*> TreeLayoutWidget::getAllTopLevelLayouts() {
		return TreeLayoutWidgetPrivate::AllTopLevelLayouts;
	}

	TreeLayoutWidget::TreeLayoutWidget(QWidget* parent, FileEditWidgetArea* firstArea) :QFrame() {
		d = new TreeLayoutWidgetPrivate(this);
		if (qobject_cast<TreeLayoutWidget*>(parent)) {
			d->ParentLayout = static_cast<TreeLayoutWidget*>(parent); 
			// if parent is a TreeLayoutWidget, parent will be managed by the parent-node.
			// may change during relayout, so we don't set parent here.
		}
		else {
			d->ParentLayout = nullptr;
			setParent(parent); // if parent is not a TreeLayoutWidget, set parent to this widget.
			TreeLayoutWidgetPrivate::AllTopLevelLayouts.append(this);
		}
		this->setWindowIcon(QIcon(":/resource/cn.yxgeneral.yayinstorystudio/icon.png"));
		this->setAcceptDrops(true);
		FileEditWidgetArea* area = firstArea ? firstArea : new FileEditWidgetArea(this);
		d->Childrens.append(area);
		d->IsLayout.append(false);
		QObject::connect(area, &QObject::destroyed, this, [this, area]() {
			d->onChildClosed(area);
			});
		QObject::connect(area, &FileEditWidgetArea::allFileClosed, this, [this, area]() {
			d->handleAllFileClosed(area);
			});
		area->installEventFilter(this);
		area->show();

	}

	TreeLayoutWidget::~TreeLayoutWidget() {
		for (int i = 0; i < d->Childrens.size(); ++i) {
			QWidget* child = d->Childrens[i];
			QObject::disconnect(child, &QObject::destroyed, this, nullptr);
			if (not d->IsLayout[i]) {
				QObject::disconnect(static_cast<FileEditWidgetArea*>(child), &FileEditWidgetArea::allFileClosed, this, nullptr);
			}
		}
		if (d->ParentLayout == nullptr) {
			TreeLayoutWidgetPrivate::AllTopLevelLayouts.removeAll(this);
		}
		delete d;
	}

	FileEditWidgetArea* TreeLayoutWidget::createFileEditAreaFirst() {
		return createFileEditAreaAt(0);
	}

	FileEditWidgetArea* TreeLayoutWidget::createFileEditAreaLast() {
		return createFileEditAreaAt(d->Childrens.size());
	}

	FileEditWidgetArea* TreeLayoutWidget::createFileEditAreaAt(int index) {
		FileEditWidgetArea* area = new FileEditWidgetArea();
		d->insertChildAt(index, area, false);
		return area;
	}

	TreeLayoutWidget* TreeLayoutWidget::createLayoutFirst() {
		return createLayoutAt(0);
	}

	TreeLayoutWidget* TreeLayoutWidget::createLayoutLast() {
		return createLayoutAt(d->Childrens.size());
	}

	TreeLayoutWidget* TreeLayoutWidget::createLayoutAt(int index) {
		TreeLayoutWidget* layout = new TreeLayoutWidget(this);
		layout->d->Orientation = (d->Orientation == Qt::Horizontal) ? Qt::Vertical : Qt::Horizontal;
		layout->d->notSelectOrientation = false;
		d->insertChildAt(index, layout, true);
		return layout;
	}

	TreeLayoutWidget* TreeLayoutWidget::replaceFileEditAt(int index, FileEditWidgetArea* newArea, bool up) {
		if (not newArea) {
			newArea = new FileEditWidgetArea();
		}
		return d->replaceAreaWithNewLayoutAt(index, newArea, up);
	}

	TreeLayoutWidget* TreeLayoutWidget::replaceLayoutAt(int index) {
		return d->collapseLayoutAt(index);
	}

	FileEditWidgetArea* TreeLayoutWidget::getFileEditAreaAt(int index) const {
		if (index < 0 || index >= d->Childrens.size()) {
			return nullptr;
		}
		if (d->IsLayout[index]) {
			return nullptr;
		}
		return qobject_cast<FileEditWidgetArea*>(d->Childrens[index]);
	}

	TreeLayoutWidget* TreeLayoutWidget::getLayoutAt(int index) const {
		if (index < 0 || index >= d->Childrens.size()) {
			return nullptr;
		}
		if (not d->IsLayout[index]) {
			return nullptr;
		}
		return qobject_cast<TreeLayoutWidget*>(d->Childrens[index]);
	}

	QList<bool> TreeLayoutWidget::getIsLayoutList() const {
		return d->IsLayout;
	}

	int TreeLayoutWidget::getChildCount() const {
		return d->Childrens.size();
	}

	bool TreeLayoutWidget::isTopLevel() const {
		return d->ParentLayout == nullptr;
	}

	Qt::Orientation TreeLayoutWidget::getOrientation() const {
		return d->Orientation;
	}

	bool TreeLayoutWidget::isOrientationSelected() const {
		return not d->notSelectOrientation;
	}

	Visindigo::Utility::JsonConfig TreeLayoutWidget::saveToJson() const {
		Visindigo::Utility::JsonConfig json;
		if (not isTopLevel()) {
			return json;
		}
		json.setInt("x", this->x());
		json.setInt("y", this->y());
		json.setInt("w", this->width());
		json.setInt("h", this->height());
		QScreen* screen = this->screen();
		json.setInt("screen", screen ? QApplication::screens().indexOf(screen) : -1);
		if (d->notSelectOrientation) {
			json.setString("orientation", "none");
		}
		else {
			json.setString("orientation", d->Orientation == Qt::Horizontal ? "horizontal" : "vertical");
		}
		QList<Visindigo::Utility::JsonConfig> children;
		d->writeChildrenToList(children);
		json.setArray("children", children);
		return json;
	}

	void TreeLayoutWidget::recoverFromJson(const Visindigo::Utility::JsonConfig& json) {
		if (not isTopLevel()) {
			return;
		}
		QString orientation = json.getString("orientation");
		if (orientation == "horizontal") {
			d->Orientation = Qt::Horizontal;
			d->notSelectOrientation = false;
		}
		else if (orientation == "vertical") {
			d->Orientation = Qt::Vertical;
			d->notSelectOrientation = false;
		}
		else {
			d->Orientation = Qt::Horizontal;
			d->notSelectOrientation = true;
		}
		int x = int(json.getInt("x"));
		int y = int(json.getInt("y"));
		int w = int(json.getInt("w"));
		int h = int(json.getInt("h"));
		if (w > 0 && h > 0) {
			this->setGeometry(x, y, w, h);
		}
		int screenIndex = int(json.getInt("screen"));
		if (this->isWindow() && screenIndex >= 0 && screenIndex < QApplication::screens().size()) {
			this->setScreen(QApplication::screens().at(screenIndex));
		}
		d->clearAllChildren();
		QList<QPair<FileEditWidgetArea*, QString>> recovered;
		d->buildStructureFromJson(json.getArray("children"), recovered);
		d->applyPercentsTopDown(json.getArray("children"));
		for (int i = 0; i < recovered.size(); ++i) {
			recovered[i].first->setAreaID(QString::number(-(i + 1)));
		}
		for (const auto& pair : recovered) {
			pair.first->setAreaID(pair.second);
		}
	}

	QList<FileEditWidgetArea*> TreeLayoutWidget::getAllFileEditAreas() const {
		QList<FileEditWidgetArea*> result;
		d->collectAreas(result);
		return result;
	}

	void TreeLayoutWidget::resizeEvent(QResizeEvent* event) {
		QFrame::resizeEvent(event);
		if (d->Childrens.size() <= 1) {
			if (not d->Childrens.isEmpty()) {
				d->Childrens.first()->setGeometry(this->rect());
				d->Childrens.first()->show();
			}
		}
		else if (d->Container) {
			d->Container->resize(this->size());
			d->Container->show();
		}
	}

	void TreeLayoutWidget::dragEnterEvent(QDragEnterEvent* event) {
		QFrame::dragEnterEvent(event);
		if (StackTag::canAcceptDrag(event->mimeData())) {
			event->acceptProposedAction();
			d->showDropIndicators();
		}
	}

	void TreeLayoutWidget::dragMoveEvent(QDragMoveEvent* event) {
		QFrame::dragMoveEvent(event);
		if (StackTag::canAcceptDrag(event->mimeData())) {
			event->acceptProposedAction();
		}
	}

	void TreeLayoutWidget::dragLeaveEvent(QDragLeaveEvent* event) {
		QFrame::dragLeaveEvent(event);
		d->scheduleHideDropIndicators();
	}

	void TreeLayoutWidget::dropEvent(QDropEvent* event) {
		QFrame::dropEvent(event);
		if (StackTag::canAcceptDrag(event->mimeData())) {
			event->acceptProposedAction();
		}
		d->hideDropIndicators();
	}

	bool TreeLayoutWidget::eventFilter(QObject* watched, QEvent* event) {
		if (event->type() == QEvent::DragEnter) {
			QDragEnterEvent* dragEnter = static_cast<QDragEnterEvent*>(event);
			if (StackTag::canAcceptDrag(dragEnter->mimeData())) {
				d->showDropIndicators();
			}
		}
		else if (event->type() == QEvent::DragLeave) {
			d->scheduleHideDropIndicators();
		}
		else if (event->type() == QEvent::Drop) {
			d->hideDropIndicators();
		}
		return QObject::eventFilter(watched, event);
	}

	void TreeLayoutWidgetPrivate::relayout() {
		if (Childrens.size() <= 1) {
			if (Container) {
				for (QWidget* child : Childrens) {
					child->setParent(q);
				}
				Container->hide();
				Container->deleteLater();
				Container = nullptr;
			}
			if (not Childrens.isEmpty()) {
				Childrens.first()->setGeometry(q->rect());
				Childrens.first()->show();
			}
			return;
		}
		if (not Container) {
			Container = new QSplitter(Orientation, q);
			Container->setHandleWidth(8);
			Container->setContentsMargins(0, 0, 0, 0);
			Container->setAcceptDrops(false);
			Container->show();
		}
		else {
			Container->setOrientation(Orientation);
		}
		for (int i = 0; i < Childrens.size(); ++i) {
			Container->insertWidget(i, Childrens[i]);
		}
		Container->resize(q->size());
		Container->show();
	}

	void TreeLayoutWidgetPrivate::balanceNewChild(int index) {
		if (not Container || Childrens.size() <= 1) {
			return;
		}
		if (index < 0 || index >= Childrens.size()) {
			return;
		}
		QList<int> sizes;
		sizes.reserve(Childrens.size());
		for (QWidget* child : Childrens) {
			sizes.append(Orientation == Qt::Horizontal ? child->width() : child->height());
		}
		int start = index;
		int end = index;
		if (index > 0) {
			start = index - 1;
		}
		if (index < Childrens.size() - 1) {
			end = index + 1;
		}
		int total = 0;
		for (int i = start; i <= end; ++i) {
			total += sizes[i];
		}
		int count = end - start + 1;
		if (total <= 0 || count <= 1) {
			return;
		}
		int each = total / count;
		int remainder = total - each * count;
		for (int i = start; i <= end; ++i) {
			sizes[i] = each + ((i - start) < remainder ? 1 : 0);
		}
		Container->setSizes(sizes);
	}

	void TreeLayoutWidgetPrivate::handleAllFileClosed(FileEditWidgetArea* area) {
		// 顶级布局必须至少保留一个区域，否则顶级 TreeLayout 将失去功能。
		if (ParentLayout == nullptr && Childrens.size() <= 1) {
			return;
		}
		area->deleteLater();
	}

	void TreeLayoutWidgetPrivate::insertChildAt(int index, QWidget* child, bool isLayout) {
		if (index < 0) {
			index = 0;
		}
		if (index > Childrens.size()) {
			index = Childrens.size();
		}
		Childrens.insert(index, child);
		IsLayout.insert(index, isLayout);
		if (Childrens.size() > 1 && notSelectOrientation) {
			Orientation = Qt::Horizontal;
			notSelectOrientation = false;
		}
		QObject::connect(child, &QObject::destroyed, q, [this, child]() {
			onChildClosed(child);
			});
		if (not isLayout) {
			child->installEventFilter(q);
			FileEditWidgetArea* area = static_cast<FileEditWidgetArea*>(child);
			QObject::connect(area, &FileEditWidgetArea::allFileClosed, q, [this, area]() {
				handleAllFileClosed(area);
				});
		}
		relayout();
		if (not isLayout) {
			balanceNewChild(index);
		}
		child->show();
	}

	QWidget* TreeLayoutWidgetPrivate::removeChildAt(int index) {
		if (index < 0 || index >= Childrens.size()) {
			return nullptr;
		}
		bool isLayout = IsLayout[index];
		QWidget* child = Childrens.takeAt(index);
		IsLayout.removeAt(index);
		QObject::disconnect(child, &QObject::destroyed, q, nullptr);
		if (not isLayout) {
			QObject::disconnect(static_cast<FileEditWidgetArea*>(child), &FileEditWidgetArea::allFileClosed, q, nullptr);
		}
		child->removeEventFilter(q);
		child->setParent(nullptr);
		return child;
	}

	TreeLayoutWidget* TreeLayoutWidgetPrivate::replaceAreaWithNewLayoutAt(int index, FileEditWidgetArea* newArea, bool newFirst) {
		if (index < 0 || index >= Childrens.size()) {
			return nullptr;
		}
		if (IsLayout[index]) {
			return nullptr;
		}
		FileEditWidgetArea* original = qobject_cast<FileEditWidgetArea*>(Childrens[index]);
		if (not original) {
			return nullptr;
		}
		removeChildAt(index);
		TreeLayoutWidget* layout = new TreeLayoutWidget(q, original);
		layout->d->Orientation = (Orientation == Qt::Horizontal) ? Qt::Vertical : Qt::Horizontal;
		layout->d->notSelectOrientation = false;
		layout->d->insertChildAt(newFirst ? 0 : 1, newArea, false);
		insertChildAt(index, layout, true);
		return layout;
	}

	TreeLayoutWidget* TreeLayoutWidgetPrivate::collapseLayoutAt(int index) {
		if (index < 0 || index >= Childrens.size()) {
			return nullptr;
		}
		if (not IsLayout[index]) {
			return nullptr;
		}
		TreeLayoutWidget* layout = qobject_cast<TreeLayoutWidget*>(Childrens[index]);
		if (not layout) {
			return nullptr;
		}
		if (layout->d->Childrens.size() != 1) {
			return nullptr;
		}
		if (layout->d->IsLayout.first()) {
			return nullptr;
		}
		QWidget* area = layout->d->Childrens.first();
		layout->d->removeChildAt(0);
		removeChildAt(index);
		insertChildAt(index, area, false);
		return layout;
	}

	void TreeLayoutWidgetPrivate::clearAllChildren() {
		hideDropIndicators();
		for (int i = Childrens.size() - 1; i >= 0; --i) {
			QWidget* child = Childrens[i];
			bool isLayout = IsLayout[i];
			QObject::disconnect(child, &QObject::destroyed, q, nullptr);
			if (not isLayout) {
				QObject::disconnect(static_cast<FileEditWidgetArea*>(child), &FileEditWidgetArea::allFileClosed, q, nullptr);
			}
			child->removeEventFilter(q);
			Childrens.removeAt(i);
			IsLayout.removeAt(i);
			delete child;
		}
		if (Container) {
			Container->hide();
			Container->deleteLater();
			Container = nullptr;
		}
	}

	void TreeLayoutWidgetPrivate::onChildClosed(QWidget* child) {
		int index = Childrens.indexOf(child);
		if (index < 0) {
			return;
		}
		Childrens.removeAt(index);
		IsLayout.removeAt(index);
		hideDropIndicators();
		if (Childrens.isEmpty()) {
			if (ParentLayout != nullptr) {
				q->deleteLater();
			}
			return;
		}
		relayout();
	}

	double TreeLayoutWidgetPrivate::computePercent(int index) const {
		if (Childrens.size() <= 1) {
			return 1.0;
		}
		if (not Container) {
			return 1.0 / Childrens.size();
		}
		QList<int> sizes = Container->sizes();
		int total = 0;
		for (int size : sizes) {
			total += size;
		}
		if (total <= 0) {
			return 1.0 / Childrens.size();
		}
		return double(sizes.value(index, 0)) / double(total);
	}

	void TreeLayoutWidgetPrivate::writeChildrenToList(QList<Visindigo::Utility::JsonConfig>& out) const {
		for (int i = 0; i < Childrens.size(); ++i) {
			Visindigo::Utility::JsonConfig child;
			child.setString("type", IsLayout[i] ? "layout" : "area");
			child.setDouble("percent", computePercent(i));
			if (IsLayout[i]) {
				TreeLayoutWidget* layout = static_cast<TreeLayoutWidget*>(Childrens[i]);
				QList<Visindigo::Utility::JsonConfig> subChildren;
				layout->d->writeChildrenToList(subChildren);
				child.setArray("children", subChildren);
			}
			else {
				FileEditWidgetArea* area = static_cast<FileEditWidgetArea*>(Childrens[i]);
				child.setString("areaID", area->getAreaID());
			}
			out.append(child);
		}
	}

	void TreeLayoutWidgetPrivate::buildStructureFromJson(const QList<Visindigo::Utility::JsonConfig>& children, QList<QPair<FileEditWidgetArea*, QString>>& recovered) {
		for (const Visindigo::Utility::JsonConfig& childJson : children) {
			QString type = childJson.getString("type");
			if (type == "layout") {
				TreeLayoutWidget* child = new TreeLayoutWidget(q);
				child->d->Orientation = (Orientation == Qt::Horizontal) ? Qt::Vertical : Qt::Horizontal;
				child->d->notSelectOrientation = false;
				child->d->clearAllChildren();
				insertChildAt(Childrens.size(), child, true);
				child->d->buildStructureFromJson(childJson.getArray("children"), recovered);
			}
			else {
				FileEditWidgetArea* area = new FileEditWidgetArea();
				insertChildAt(Childrens.size(), area, false);
				recovered.append(qMakePair(area, childJson.getString("areaID")));
			}
		}
	}

	void TreeLayoutWidgetPrivate::applyPercentsTopDown(const QList<Visindigo::Utility::JsonConfig>& children) {
		applyChildrenPercents(children);
		for (int i = 0; i < children.size() && i < Childrens.size(); ++i) {
			if (children[i].getString("type") == "layout") {
				TreeLayoutWidget* layout = static_cast<TreeLayoutWidget*>(Childrens[i]);
				layout->d->applyPercentsTopDown(children[i].getArray("children"));
			}
		}
	}

	void TreeLayoutWidgetPrivate::applyChildrenPercents(const QList<Visindigo::Utility::JsonConfig>& children) {
		if (not Container || Childrens.size() <= 1) {
			return;
		}
		if (children.size() != Childrens.size()) {
			return;
		}
		int total = (Orientation == Qt::Horizontal) ? Container->width() : Container->height();
		if (total <= 0) {
			total = (Orientation == Qt::Horizontal) ? q->width() : q->height();
		}
		if (total <= 0) {
			return;
		}
		QList<int> sizes;
		for (int i = 0; i < children.size(); ++i) {
			double percent = children[i].getDouble("percent");
			if (percent < 0.0) {
				percent = 0.0;
			}
			if (percent > 1.0) {
				percent = 1.0;
			}
			sizes.append(percent * total < 20 ? 20 : int(percent * total));
		}
		Container->setSizes(sizes);
	}

	void TreeLayoutWidgetPrivate::collectAreas(QList<FileEditWidgetArea*>& out) const {
		for (int i = 0; i < Childrens.size(); ++i) {
			if (IsLayout[i]) {
				static_cast<TreeLayoutWidget*>(Childrens[i])->d->collectAreas(out);
			}
			else {
				out.append(static_cast<FileEditWidgetArea*>(Childrens[i]));
			}
		}
	}

	void TreeLayoutWidgetPrivate::showDropIndicators() {
		if (HideTimer) {
			HideTimer->stop();
		}
		if (not DropZones.isEmpty()) {
			return;
		}
		const int thickness = 26;
		const QRect rect = q->rect();
		const int w = rect.width();
		const int h = rect.height();
		const bool fullRing = notSelectOrientation || ParentLayout != nullptr;

		QList<QPair<TreeLayoutDropZone::DropZone, int>> outerZones;
		QList<QPair<TreeLayoutDropZone::DropZone, int>> innerZones;
		if (notSelectOrientation) {
			outerZones.append(qMakePair(TreeLayoutDropZone::DropZone::OuterLeft, -1));
			outerZones.append(qMakePair(TreeLayoutDropZone::DropZone::OuterRight, -1));
			outerZones.append(qMakePair(TreeLayoutDropZone::DropZone::OuterTop, -1));
			outerZones.append(qMakePair(TreeLayoutDropZone::DropZone::OuterBottom, -1));
		}
		else if (Orientation == Qt::Horizontal) {
			outerZones.append(qMakePair(TreeLayoutDropZone::DropZone::OuterLeft, -1));
			outerZones.append(qMakePair(TreeLayoutDropZone::DropZone::OuterRight, -1));
			if (ParentLayout) {
				outerZones.append(qMakePair(TreeLayoutDropZone::DropZone::OuterTop, -1));
				outerZones.append(qMakePair(TreeLayoutDropZone::DropZone::OuterBottom, -1));
			}
			for (int i = 0; i < Childrens.size(); ++i) {
				if (IsLayout[i]) {
					continue;
				}
				innerZones.append(qMakePair(TreeLayoutDropZone::DropZone::ItemTop, i));
				innerZones.append(qMakePair(TreeLayoutDropZone::DropZone::ItemBottom, i));
			}
		}
		else {
			outerZones.append(qMakePair(TreeLayoutDropZone::DropZone::OuterTop, -1));
			outerZones.append(qMakePair(TreeLayoutDropZone::DropZone::OuterBottom, -1));
			if (ParentLayout) {
				outerZones.append(qMakePair(TreeLayoutDropZone::DropZone::OuterLeft, -1));
				outerZones.append(qMakePair(TreeLayoutDropZone::DropZone::OuterRight, -1));
			}
			for (int i = 0; i < Childrens.size(); ++i) {
				if (IsLayout[i]) {
					continue;
				}
				innerZones.append(qMakePair(TreeLayoutDropZone::DropZone::ItemLeft, i));
				innerZones.append(qMakePair(TreeLayoutDropZone::DropZone::ItemRight, i));
			}
		}

		auto createIndicator = [this](TreeLayoutDropZone::DropZone zone, int childIndex, const QPainterPath& parentShape) {
			QRectF bounds = parentShape.boundingRect();
			TreeLayoutDropZone* indicator = new TreeLayoutDropZone(this, zone, childIndex);
			indicator->setGeometry(bounds.toRect());
			QPainterPath localShape = parentShape;
			localShape.translate(-bounds.topLeft());
			indicator->setZoneShape(localShape);
			indicator->show();
			indicator->raise();
			DropZones.append(indicator);
			};

		for (const auto& zone : innerZones) {
			int index = zone.second;
			QWidget* child = Childrens[index];
			QPoint topLeft = child->mapTo(q, QPoint(0, 0));
			QRect childRect(topLeft, child->size());
			QPainterPath shape;
			if (Orientation == Qt::Horizontal) {
				int xLeft = childRect.x();
				int xRight = childRect.x() + childRect.width();
				if (xLeft < thickness) {
					xLeft = thickness;
				}
				if (xRight > w - thickness) {
					xRight = w - thickness;
				}
				if (xRight <= xLeft) {
					continue;
				}
				if (xRight - xLeft >= 2 * thickness) {
					if (zone.first == TreeLayoutDropZone::DropZone::ItemTop) {
						QPolygonF poly;
						poly << QPointF(xLeft, thickness) << QPointF(xRight, thickness) << QPointF(xRight - thickness, 2 * thickness) << QPointF(xLeft + thickness, 2 * thickness);
						shape.addPolygon(poly);
						shape.closeSubpath();
					}
					else {
						QPolygonF poly;
						poly << QPointF(xLeft + thickness, h - 2 * thickness) << QPointF(xRight - thickness, h - 2 * thickness) << QPointF(xRight, h - thickness) << QPointF(xLeft, h - thickness);
						shape.addPolygon(poly);
						shape.closeSubpath();
					}
				}
				else if (zone.first == TreeLayoutDropZone::DropZone::ItemTop) {
					shape.addRect(xLeft, thickness, xRight - xLeft, thickness);
				}
				else {
					shape.addRect(xLeft, h - 2 * thickness, xRight - xLeft, thickness);
				}
			}
			else {
				int yTop = childRect.y();
				int yBottom = childRect.y() + childRect.height();
				if (yTop < thickness) {
					yTop = thickness;
				}
				if (yBottom > h - thickness) {
					yBottom = h - thickness;
				}
				if (yBottom <= yTop) {
					continue;
				}
				if (yBottom - yTop >= 2 * thickness) {
					if (zone.first == TreeLayoutDropZone::DropZone::ItemLeft) {
						QPolygonF poly;
						poly << QPointF(thickness, yTop) << QPointF(thickness, yBottom) << QPointF(2 * thickness, yBottom - thickness) << QPointF(2 * thickness, yTop + thickness);
						shape.addPolygon(poly);
						shape.closeSubpath();
					}
					else {
						QPolygonF poly;
						poly << QPointF(w - thickness, yTop) << QPointF(w - 2 * thickness, yTop + thickness) << QPointF(w - 2 * thickness, yBottom - thickness) << QPointF(w - thickness, yBottom);
						shape.addPolygon(poly);
						shape.closeSubpath();
					}
				}
				else if (zone.first == TreeLayoutDropZone::DropZone::ItemLeft) {
					shape.addRect(thickness, yTop, thickness, yBottom - yTop);
				}
				else {
					shape.addRect(w - 2 * thickness, yTop, thickness, yBottom - yTop);
				}
			}
			createIndicator(zone.first, index, shape);
		}

		if (Childrens.size() > 1) {
			const int seamHalf = 6;
			for (int i = 0; i + 1 < Childrens.size(); ++i) {
				QWidget* firstChild = Childrens[i];
				QWidget* secondChild = Childrens[i + 1];
				QPoint firstTop = firstChild->mapTo(q, QPoint(0, 0));
				QPoint secondTop = secondChild->mapTo(q, QPoint(0, 0));
				QPainterPath shape;
				if (Orientation == Qt::Horizontal) {
					int firstRight = firstTop.x() + firstChild->width();
					int secondLeft = secondTop.x();
					int centerX = (firstRight + secondLeft) / 2;
					shape.addRoundedRect(QRectF(centerX - seamHalf, thickness, seamHalf * 2, h - 2 * thickness), 4, 4);
				}
				else {
					int firstBottom = firstTop.y() + firstChild->height();
					int secondTopY = secondTop.y();
					int centerY = (firstBottom + secondTopY) / 2;
					shape.addRoundedRect(QRectF(thickness, centerY - seamHalf, w - 2 * thickness, seamHalf * 2), 4, 4);
				}
				createIndicator(TreeLayoutDropZone::DropZone::Seam, i + 1, shape);
			}
		}

		for (const auto& zone : outerZones) {
			QPainterPath shape;
			if (fullRing) {
				QPolygonF poly;
				if (zone.first == TreeLayoutDropZone::DropZone::OuterLeft) {
					poly << QPointF(0, 0) << QPointF(0, h) << QPointF(thickness, h - thickness) << QPointF(thickness, thickness);
				}
				else if (zone.first == TreeLayoutDropZone::DropZone::OuterRight) {
					poly << QPointF(w, 0) << QPointF(w - thickness, thickness) << QPointF(w - thickness, h - thickness) << QPointF(w, h);
				}
				else if (zone.first == TreeLayoutDropZone::DropZone::OuterTop) {
					poly << QPointF(0, 0) << QPointF(w, 0) << QPointF(w - thickness, thickness) << QPointF(thickness, thickness);
				}
				else if (zone.first == TreeLayoutDropZone::DropZone::OuterBottom) {
					poly << QPointF(thickness, h - thickness) << QPointF(w - thickness, h - thickness) << QPointF(w, h) << QPointF(0, h);
				}
				shape.addPolygon(poly);
				shape.closeSubpath();
			}
			else {
				if (zone.first == TreeLayoutDropZone::DropZone::OuterLeft) {
					shape.addRect(0, 0, thickness, h);
				}
				else if (zone.first == TreeLayoutDropZone::DropZone::OuterRight) {
					shape.addRect(w - thickness, 0, thickness, h);
				}
				else if (zone.first == TreeLayoutDropZone::DropZone::OuterTop) {
					shape.addRect(0, 0, w, thickness);
				}
				else if (zone.first == TreeLayoutDropZone::DropZone::OuterBottom) {
					shape.addRect(0, h - thickness, w, thickness);
				}
			}
			createIndicator(zone.first, -1, shape);
		}
	}

	void TreeLayoutWidgetPrivate::hideDropIndicators() {
		if (HideTimer) {
			HideTimer->stop();
		}
		for (TreeLayoutDropZone* zone : DropZones) {
			zone->hide();
			zone->deleteLater();
		}
		DropZones.clear();
	}

	void TreeLayoutWidgetPrivate::scheduleHideDropIndicators() {
		if (HideTimer) {
			HideTimer->start();
		}
	}

	void TreeLayoutWidgetPrivate::handleDrop(TreeLayoutDropZone::DropZone zone, int childIndex, QDropEvent* event) {
		hideDropIndicators();
		if (not event->mimeData()->hasFormat(StackTag::stackTagDragMimeType)) {
			return;
		}
		QByteArray data = event->mimeData()->data(StackTag::stackTagDragMimeType);
		Visindigo::Utility::JsonConfig json(QString::fromUtf8(data));
		StackTag* selfLabel = reinterpret_cast<StackTag*>(json.getString("self").toULongLong(nullptr, 16));
		StackTagWidget* sourceTagArea = reinterpret_cast<StackTagWidget*>(json.getString("area").toULongLong(nullptr, 16));
		FileEditWidgetArea* sourceArea = sourceTagArea ? sourceTagArea->getArea() : nullptr;
		if (not selfLabel || not sourceArea) {
			return;
		}
		QString filePath = selfLabel->getFilePath();

		auto createAndFill = [this, sourceArea, filePath](int index) -> FileEditWidgetArea* {
			FileEditWidgetArea* area = new FileEditWidgetArea();
			insertChildAt(index, area, false);
			if (sourceArea != area) {
				sourceArea->moveWidgetTo(filePath, area);
			}
			return area;
			};

		auto insertIntoParent = [this, sourceArea, filePath](bool before) {
			if (not ParentLayout) {
				return;
			}
			int parentIndex = ParentLayout->d->Childrens.indexOf(q);
			if (parentIndex < 0) {
				return;
			}
			int insertIndex = before ? parentIndex : parentIndex + 1;
			FileEditWidgetArea* area = new FileEditWidgetArea();
			ParentLayout->d->insertChildAt(insertIndex, area, false);
			if (sourceArea != area) {
				sourceArea->moveWidgetTo(filePath, area);
			}
			};

		switch (zone) {
		case TreeLayoutDropZone::DropZone::OuterLeft:
		case TreeLayoutDropZone::DropZone::OuterRight: {
			bool left = (zone == TreeLayoutDropZone::DropZone::OuterLeft);
			if (notSelectOrientation) {
				Orientation = Qt::Horizontal;
				notSelectOrientation = false;
				createAndFill(left ? 0 : Childrens.size());
			}
			else if (Orientation == Qt::Horizontal) {
				createAndFill(left ? 0 : Childrens.size());
			}
			else {
				insertIntoParent(left);
			}
			break;
		}
		case TreeLayoutDropZone::DropZone::OuterTop:
		case TreeLayoutDropZone::DropZone::OuterBottom: {
			bool top = (zone == TreeLayoutDropZone::DropZone::OuterTop);
			if (notSelectOrientation) {
				Orientation = Qt::Vertical;
				notSelectOrientation = false;
				createAndFill(top ? 0 : Childrens.size());
			}
			else if (Orientation == Qt::Vertical) {
				createAndFill(top ? 0 : Childrens.size());
			}
			else {
				insertIntoParent(top);
			}
			break;
		}
		case TreeLayoutDropZone::DropZone::ItemTop:
		case TreeLayoutDropZone::DropZone::ItemBottom: {
			FileEditWidgetArea* area = new FileEditWidgetArea();
			TreeLayoutWidget* layout = replaceAreaWithNewLayoutAt(childIndex, area, zone == TreeLayoutDropZone::DropZone::ItemTop);
			if (layout && sourceArea != area) {
				sourceArea->moveWidgetTo(filePath, area);
			}
			break;
		}
		case TreeLayoutDropZone::DropZone::ItemLeft:
		case TreeLayoutDropZone::DropZone::ItemRight: {
			FileEditWidgetArea* area = new FileEditWidgetArea();
			TreeLayoutWidget* layout = replaceAreaWithNewLayoutAt(childIndex, area, zone == TreeLayoutDropZone::DropZone::ItemLeft);
			if (layout && sourceArea != area) {
				sourceArea->moveWidgetTo(filePath, area);
			}
			break;
		}
		case TreeLayoutDropZone::DropZone::Seam: {
			createAndFill(childIndex);
			break;
		}
		default:
			break;
		}
		event->acceptProposedAction();
	}
}