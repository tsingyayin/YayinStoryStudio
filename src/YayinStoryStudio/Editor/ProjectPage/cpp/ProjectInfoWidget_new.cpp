#include <QtCore/qdatetime.h>
#include <QtGui/qpixmap.h>
#include <QtWidgets/qApplication.h>
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qpushbutton.h>
#include <General/Log.h>
#include <General/TranslationHost.h>
#include <General/YSSLogger.h>
#include <General/YSSProject.h>
#include <Utility/AsyncFunction.h>
#include <Utility/FileUtility.h>
#include "Editor/ProjectPage/ProjectInfoWidget_new.h"
namespace YSS::ProjectPage {
	ProjectInfoWidget_new::ProjectInfoWidget_new(QWidget* parent) :QFrame(parent) {
		this->setObjectName("ProjectInfoWidget_new");
		this->setAttribute(Qt::WA_StyledBackground, true);
		CoverLabel = new QLabel(this);
		CoverLabel->setAlignment(Qt::AlignCenter);
		CoverLabel->setObjectName("ProjectCoverLabel");
		CoverLabel->setFixedHeight(220);
		CoverLabel->setMinimumWidth(200);
		TitleLabel = new QLabel(this);
		TitleLabel->setObjectName("ProjectTitleLabel");
		TitleLabel->setWordWrap(true);
		auto font = qApp->font();
		font.setPointSizeF(font.pointSizeF() * 1.6);
		font.setWeight(QFont::DemiBold);
		TitleLabel->setFont(font);
		CreateTimeLabel = new QLabel(this);
		LastModifiedTimeLabel = new QLabel(this);
		SizeLabel = new QLabel(this);
		ShowInBrowserButton = new QPushButton(this);
		ShowInBrowserButton->setToolTip(VITRL("YSS::tooltips.projectWin.showInBrowser"));
		ShowInBrowserButton->setCursor(Qt::PointingHandCursor);
		DeleteButton = new QPushButton(this);
		DeleteButton->setToolTip(VITRL("YSS::tooltips.projectWin.delete"));
		DeleteButton->setCursor(Qt::PointingHandCursor);
		Layout = new QGridLayout(this);
		DeleteButton->setObjectName("DeleteButton");
		this->setLayout(Layout);
		Layout->setContentsMargins(18, 18, 18, 18);
		Layout->setSpacing(8);
		Layout->addWidget(CoverLabel, 0, 0, 1, 4);
		Layout->addWidget(TitleLabel, 1, 0, 1, 4);
		Layout->addWidget(SizeLabel, 2, 0, 1, 4);
		Layout->addWidget(CreateTimeLabel, 3, 0, 1, 4);
		Layout->addWidget(LastModifiedTimeLabel, 4, 0, 1, 4);
		Layout->addWidget(ShowInBrowserButton, 6, 0, 1, 4);
		Layout->addWidget(DeleteButton, 7, 0, 1, 4);
		Layout->setRowStretch(5, 1);
		Layout->setColumnStretch(0, 1);

		connect(DeleteButton, &QPushButton::clicked, this, &ProjectInfoWidget_new::onDeleteButtonClicked);
		connect(ShowInBrowserButton, &QPushButton::clicked, this, &ProjectInfoWidget_new::onShowInBrowserButtonClicked);
		initWidget();
		setColorfulEnable(true);
		onThemeChanged();
	}

	void ProjectInfoWidget_new::initWidget() {
		SizeRequestId++;
		Project = nullptr;
		CoverSource = QPixmap();
		CoverLabel->clear();
		CoverLabel->setText(VITRL("YSS::project.noCover"));
		TitleLabel->setText(VITRL("YSS::project.selectToShow"));
		CreateTimeLabel->setText(VITRL("YSS::project.createTime") + ": ");
		LastModifiedTimeLabel->setText(VITRL("YSS::project.lastModifiedTime") + ": ");
		SizeLabel->setText(VITRL("YSS::project.size") + ": ");
		ShowInBrowserButton->setText(VITRL("YSS::project.showInBrowser"));
		DeleteButton->setText(VITRL("YSS::project.delete"));
	}

	void ProjectInfoWidget_new::updateCoverPixmap() {
		if (CoverSource.isNull() || CoverLabel->width() <= 0 || CoverLabel->height() <= 0) {
			return;
		}
		const qreal dpr = devicePixelRatioF();
		const QSize target(qMax(1, qRound(CoverLabel->width() * dpr)),
			qMax(1, qRound(CoverLabel->height() * dpr)));
		const QPixmap scaled = CoverSource.scaled(target, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
		const QRect crop((scaled.width() - target.width()) / 2, (scaled.height() - target.height()) / 2,
			target.width(), target.height());
		QPixmap pixmap = scaled.copy(crop);
		pixmap.setDevicePixelRatio(dpr);
		CoverLabel->setPixmap(pixmap);
	}

	void ProjectInfoWidget_new::showProject(YSSCore::General::YSSProject* project) {
		if (project == nullptr) {
			initWidget();
			return;
		}
		
		Project = project;
		QString path = project->getProjectFolder().getLocalPath();
		int requestId = ++SizeRequestId;
		if (path.isEmpty()) {
			SizeLabel->setText(VITRL("YSS::project.size") + ": " + VITRL("YSS::project.calculating"));
		}
		else {
			YSSAsync<qint64, QString>(
				{ QString(path) },
				[](QString path)->qint64 {
					return Visindigo::Utility::FileUtility::sizeBytes(path);
				},
				// 传入this作为上下文：本控件在求值完成前被销毁时，这段回调不会再执行。
				[this, requestId](qint64 size) {
					if (requestId != SizeRequestId) {
						// 用户已经切到别的项目，这个结果是过期的。
						return;
					}
					QString readable = Visindigo::Utility::FileUtility::readableSize(size);
					SizeLabel->setText(VITRL("YSS::project.size") + ": " + readable);
				},
				this
			);
			SizeLabel->setText(VITRL("YSS::project.size") + ": " + VITRL("YSS::project.calculating"));
		}
		CoverLabel->setText("");
		const QString coverPath = project->getProjectIconPath().getLocalPath();
		CoverSource = QPixmap(coverPath);
		if (coverPath.isEmpty() || CoverSource.isNull()) {
			CoverSource = QPixmap();
			CoverLabel->clear();
			CoverLabel->setText(VITRL("YSS::project.noCover"));
		}
		else {
			updateCoverPixmap();
		}
		TitleLabel->setText(project->getProjectName());
		CreateTimeLabel->setText(VITRL("YSS::project.createTime") + ": " + project->getProjectCreateTime().toString("yyyy-MM-dd hh:mm:ss"));
		LastModifiedTimeLabel->setText(VITRL("YSS::project.lastModifiedTime") + ": " + project->getProjectLastModifyTime().toString("yyyy-MM-dd hh:mm:ss"));
	}

	void ProjectInfoWidget_new::onDeleteButtonClicked() {
		if (Project == nullptr) {
			return;
		}
		QMessageBox msgBox;
		msgBox.setWindowTitle(VITRL("YSS::project.delete"));
		msgBox.setText(VITRL("YSS::project.deleteConfirm"));
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Cancel);
		int ret = msgBox.exec();
		switch (ret) {
		case QMessageBox::Yes:
			emit removeConfirmed(Project);
			return;
		case QMessageBox::Cancel:
			return;
		}
	}

	void ProjectInfoWidget_new::onRemoveFromListButtonClicked() {
		if (Project == nullptr) {
			return;
		}
		QMessageBox msgBox;
		msgBox.setWindowTitle(VITRL("YSS::project.delete"));
		msgBox.setText(VITRL("YSS::project.deleteConfirm"));
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Cancel);
		int ret = msgBox.exec();
		switch (ret) {
		case QMessageBox::Yes:
			emit removeConfirmed(Project);
			return;
		case QMessageBox::Cancel:
			return;
		}
	}

	void ProjectInfoWidget_new::onShowInBrowserButtonClicked() {
		if (Project != nullptr) {
			QString dir = Project->getProjectFolder().getLocalPath();
			//yDebugF << dir;
			Visindigo::Utility::FileUtility::openExplorer(dir);
		}
	}

	void ProjectInfoWidget_new::resizeEvent(QResizeEvent* event) {
		QFrame::resizeEvent(event);
		updateCoverPixmap();
	}

	void ProjectInfoWidget_new::onThemeChanged() {
		//vgDebugF << "ProjectInfo";
		//this->applyVIStyleTemplate("YSS::ProjectInfo");
	}
}