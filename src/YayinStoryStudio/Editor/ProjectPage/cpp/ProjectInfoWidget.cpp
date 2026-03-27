#include "../ProjectInfoWidget.h"
#include <Utility/FileUtility.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qgridlayout.h>
#include <General/YSSProject.h>
#include <General/TranslationHost.h>
#include <Utility/AsyncFunction.h>
#include <QtCore/qdatetime.h>
#include <General/Log.h>
#include <QtWidgets/qmessagebox.h>
#include <General/YSSLogger.h>

namespace YSS::ProjectPage {
	ProjectInfoWidget::ProjectInfoWidget(QWidget* parent) :QFrame(parent) {
		this->setObjectName("ProjectInfoWidget");
		CoverLabel = new QLabel(this);
		CoverLabel->setAlignment(Qt::AlignCenter);
		CoverLabel->setObjectName("ProjectCoverLabel");
		TitleLabel = new QLabel(this);
		TitleLabel->setObjectName("ProjectTitleLabel");
		CreateTimeLabel = new QLabel(this);
		LastModifiedTimeLabel = new QLabel(this);
		SizeLabel = new QLabel(this);
		ShowInBrowserButton = new QPushButton(this);
		ShowInBrowserButton->setToolTip(VITR("YSS::tooltips.projectWin.showInBrowser"));
		DeleteButton = new QPushButton(this);
		DeleteButton->setToolTip(VITR("YSS::tooltips.projectWin.delete"));
		Layout = new QGridLayout(this);
		DeleteButton->setObjectName("DeleteButton");
		this->setLayout(Layout);
		Layout->addWidget(CoverLabel, 0, 0, 4, 1);
		Layout->addWidget(TitleLabel, 0, 1, 1, 3);
		Layout->addWidget(SizeLabel, 1, 1, 1, 1);
		Layout->addWidget(CreateTimeLabel, 2, 1, 1, 1);
		Layout->addWidget(LastModifiedTimeLabel, 3, 1, 1, 1);
		Layout->addWidget(ShowInBrowserButton, 2, 3, 1, 1);
		Layout->addWidget(DeleteButton, 3, 3, 1, 1);
		
		connect(DeleteButton, &QPushButton::clicked, this, &ProjectInfoWidget::onDeleteButtonClicked);
		connect(ShowInBrowserButton, &QPushButton::clicked, this, &ProjectInfoWidget::onShowInBrowserButtonClicked);
		initWidget();
		setColorfulEnable(true);
		onThemeChanged();
	}

	void ProjectInfoWidget::initWidget() {
		Project = nullptr;
		CoverLabel->setText(VITR("YSS::project.noCover"));
		TitleLabel->setText(VITR("YSS::project.selectToShow"));
		CreateTimeLabel->setText(VITR("YSS::project.createTime") + ": ");
		LastModifiedTimeLabel->setText(VITR("YSS::project.lastModifiedTime") + ": ");
		SizeLabel->setText(VITR("YSS::project.size") + ": ");
		ShowInBrowserButton->setText(VITR("YSS::project.showInBrowser"));
		DeleteButton->setText(VITR("YSS::project.delete"));
	}

	void ProjectInfoWidget::showProject(YSSCore::General::YSSProject* project) {
		if (project == nullptr) {
			return;
		}
		initWidget();
		Project = project;
		QString path = project->getProjectFolder();
		YSSAsync<qint64, QString>(
			{ QString(path) },
			[](QString path)->qint64 {
				return Visindigo::Utility::FileUtility::sizeBytes(path);
			},
			[this](qint64 size) {
				QString readable = Visindigo::Utility::FileUtility::readableSize(size);
				SizeLabel->setText(VITR("YSS::project.size") + ": " + readable);
			}
		);
		SizeLabel->setText(VITR("YSS::project.size") + ": " + VITR("YSS::project.calculating"));
		CoverLabel->setText("");
		CoverLabel->setStyleSheet("QLabel{border-image:url(" + project->getProjectFolder() + "/" + project->getProjectIconPath() + ")}");
		yDebugF << project->getProjectFolder() + "/" + project->getProjectIconPath();
		TitleLabel->setText(project->getProjectName());
		CreateTimeLabel->setText(VITR("YSS::project.createTime") + ": " + project->getProjectCreateTime().toString("yyyy-MM-dd hh:mm:ss"));
		LastModifiedTimeLabel->setText(VITR("YSS::project.lastModifiedTime") + ": " + project->getProjectLastModifyTime().toString("yyyy-MM-dd hh:mm:ss"));
	}

	void ProjectInfoWidget::onDeleteButtonClicked() {
		if (Project == nullptr) {
			return;
		}
		QMessageBox msgBox;
		msgBox.setWindowTitle(VITR("YSS::project.delete"));
		msgBox.setText(VITR("YSS::project.deleteConfirm"));
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

	void ProjectInfoWidget::onRemoveFromListButtonClicked() {
		if (Project == nullptr) {
			return;
		}
		QMessageBox msgBox;
		msgBox.setWindowTitle(VITR("YSS::project.delete"));
		msgBox.setText(VITR("YSS::project.deleteConfirm"));
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

	void ProjectInfoWidget::onShowInBrowserButtonClicked() {
		if (Project != nullptr) {
			QString dir = Project->getProjectFolder();
			yDebugF << dir;
			Visindigo::Utility::FileUtility::openExplorer(dir);
		}
	}

	void ProjectInfoWidget::resizeEvent(QResizeEvent* event) {
		QFrame::resizeEvent(event);
		this->CoverLabel->setFixedHeight((float)this->CoverLabel->width() / 16 * 9); // 16:9 aspect ratio
	}

	void ProjectInfoWidget::onThemeChanged() {
		vgDebugF << "ProjectInfo";
		this->applyVIStyleTemplate("YSS::ProjectInfo");
	}
}