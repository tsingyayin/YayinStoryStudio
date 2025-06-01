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

namespace YSS::ProjectPage {
	ProjectInfoWidget::ProjectInfoWidget(QWidget* parent) :QFrame(parent) {
		this->setObjectName("ProjectInfoWidget");
		CoverLabel = new QLabel(this);
		CoverLabel->setAlignment(Qt::AlignCenter);
		CoverLabel->setObjectName("ProjectCoverLabel");
		TitleLabel = new QLabel(this);
		CreateTimeLabel = new QLabel(this);
		LastModifiedTimeLabel = new QLabel(this);
		SizeLabel = new QLabel(this);
		ShowInBrowserButton = new QPushButton(this);
		DeleteButton = new QPushButton(this);
		Layout = new QGridLayout(this);

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
	}

	void ProjectInfoWidget::initWidget(){
		Project = nullptr;
		CoverLabel->setText(YSSTR("YSS::project.noCover"));
		TitleLabel->setText(YSSTR("YSS::project.selectToShow"));
		CreateTimeLabel->setText(YSSTR("YSS::project.createTime")+": ");
		LastModifiedTimeLabel->setText(YSSTR("YSS::project.lastModifiedTime")+": ");
		SizeLabel->setText(YSSTR("YSS::project.size") + ": ");
		ShowInBrowserButton->setText(YSSTR("YSS::project.showInBrowser"));
		DeleteButton->setText(YSSTR("YSS::project.delete"));
	}

	
	void ProjectInfoWidget::showProject(YSSCore::General::YSSProject* project) {
		if (project == nullptr) {
			return;
		}
		initWidget();
		Project = project;
		QString path = project->getProjectFolder();
		YSSAsync<qint64>(
			[&]()->qint64 {
				return YSSCore::Utility::FileUtility::sizeBytes(path);
			}, 
			{},
			[this](qint64 size) {
				QString readable = YSSCore::Utility::FileUtility::readableSize(size);
				SizeLabel->setText(YSSTR("YSS::project.size") + ": " + readable);
			}
		);
		SizeLabel->setText(YSSTR("YSS::project.size") + ": " + YSSTR("YSS::project.calculating"));
		CoverLabel->setText("");
		CoverLabel->setStyleSheet("QLabel{border-image:url(" + project->getProjectFolder() + "/" + project->getProjectIconPath() + ")}");
		yDebugF << project->getProjectFolder() + "/" + project->getProjectIconPath();
		TitleLabel->setText(project->getProjectName());
		CreateTimeLabel->setText(YSSTR("YSS::project.createTime") + ": " + project->getProjectCreateTime().toString("yyyy-MM-dd hh:mm:ss"));
		LastModifiedTimeLabel->setText(YSSTR("YSS::project.lastModifiedTime") + ": " + project->getProjectLastModifyTime().toString("yyyy-MM-dd hh:mm:ss"));
	}

	void ProjectInfoWidget::onDeleteButtonClicked() {
		
	}

	void ProjectInfoWidget::onShowInBrowserButtonClicked() {
		
	}
	
	void ProjectInfoWidget::resizeEvent(QResizeEvent* event) {
		QFrame::resizeEvent(event);
		this->CoverLabel->setFixedHeight((float)this->CoverLabel->width() / 16 * 9); // 16:9 aspect ratio
	}
}