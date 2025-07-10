#pragma once
#include <QtCore/qobject.h>
#include <QtCore/qfilesystemwatcher.h>
class AStoryResourceMonitor :public QObject{
	Q_OBJECT;
private:
	QFileSystemWatcher* Watcher = nullptr;
public:
	AStoryResourceMonitor(QObject* parent = nullptr) : QObject(parent) {}

};