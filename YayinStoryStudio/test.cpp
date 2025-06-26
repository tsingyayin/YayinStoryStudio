#include <QtWidgets/qapplication.h>
#include <Utility/JsonConfig.h>
#include <General/YSSProject.h>
#include <General/private//UpdateChainSyntaxTest.h>
#include <Utility/CodeDiff.h>
#include <QtCore/qfile.h>
#include <QtCore/qdir.h>
#include <Utility/FileUtility.h>
#include <Utility/AsyncFunction.h>
#include <Widgets/ConfigWidget.h>

void test_main(QApplication& a) {
	YSSCore::Utility::CodeDiff codeDiff;
	codeDiff.setOldCode(YSSCore::Utility::FileUtility::readLines("./resource/old.txt"));
	codeDiff.setNewCode(YSSCore::Utility::FileUtility::readLines("./resource/new.txt"));
	codeDiff.compare();
	//codeDiff.debugPrint();

	YSSAsync<int, int, int>(
		{ 3, 4 },
		[](int a, int b)->int {
			QThread::msleep(10000);
			return a + b;
		},

		[](int rtn) {
			qDebug() << rtn;
		}
	);
}

void test_loaded() {
	//YSSCore::Editor::ConfigWidget* configWidget = new YSSCore::Editor::ConfigWidget();
	//configWidget->setAttribute(Qt::WA_DeleteOnClose);
	//QString jsonStr = YSSCore::Utility::FileUtility::readAll("./resource/config_widget.json");
	//configWidget->loadCWJson(jsonStr);
	//configWidget->show();
}