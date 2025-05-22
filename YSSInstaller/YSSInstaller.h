#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_YSSInstaller.h"

class YSSInstaller : public QMainWindow
{
    Q_OBJECT

public:
    YSSInstaller(QWidget *parent = nullptr);
    ~YSSInstaller();

private:
    Ui::YSSInstallerClass ui;
};
