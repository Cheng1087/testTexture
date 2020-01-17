#include <iostream>
#include "Base/String.h"

#include <QWindow>
#include <QApplication>
#include <QStyleFactory>
#include <QDebug>

#include "DisplayCuttingResultWindow.h"


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    qDebug() << app.applicationDirPath();
    DisplayCuttingResultWindow mw;
    mw.show();
    mw.resize(1000,600);
    return app.exec();
}
