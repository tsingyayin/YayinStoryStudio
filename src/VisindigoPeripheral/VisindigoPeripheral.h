#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_VisindigoPeripheral.h"

class VisindigoPeripheral : public QMainWindow
{
    Q_OBJECT

public:
    VisindigoPeripheral(QWidget *parent = nullptr);
    ~VisindigoPeripheral();

private:
    Ui::VisindigoPeripheralClass ui;
};

