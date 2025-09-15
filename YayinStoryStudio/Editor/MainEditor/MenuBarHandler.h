#pragma once
#include <QtCore/qobject.h>
#include <General/Log.h>
class Menu_File_FileOptions :public QObject {
	Q_OBJECT;
public:
	Menu_File_FileOptions(QObject* parent = nullptr) : QObject(parent) {
		this->setObjectName("fileOptions");
	}
public slots:
	void newfile(){
		yDebug << "New File";
	}
	void open() {
		yDebug << "Open File";
	}
	void save() {
		yDebug << "Save File";
	}
	void saveAll() {
		yDebug << "Save All Files";
	}
	void saveAs() {
		yDebug << "Save File As";
	}
};
class Menu_File_ProgramOptions :public QObject {
	Q_OBJECT;
public:
	Menu_File_ProgramOptions(QObject* parent = nullptr) : QObject(parent) {
		this->setObjectName("programOptions");
	}
public slots:
	void recentFiles() {
		yDebug << "Recent Files";
	}
	void preferences() {
		yDebug << "Preferences";
	}
	void exit() {
		yDebug << "Exit Program";
	}
};
class Menu_File :public QObject {
	Q_OBJECT;
public:
	Menu_File(QObject* parent = nullptr): QObject(parent) {
		this->setObjectName("file");
		new Menu_File_FileOptions(this);
		new Menu_File_ProgramOptions(this);
	}
};
class MenuActionHandler :public QObject {
	Q_OBJECT;
public:
	MenuActionHandler(QObject* parent = nullptr) {
		new Menu_File(this);
	}
};