#include "mainwindow.h"
#include <QApplication>
#include <QTextCodec>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    qApp->setStyleSheet("QLabel{background-color:blue;}");

    MainWindow w;
    w.show();

    return a.exec();
}
