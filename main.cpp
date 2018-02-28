#include "mainwindow.hh"
#include <QApplication>
#include <QTextCodec>

#include "scanmode.hh"
#include "continuationmode.hh"
#include "ncmode.hh"
#include <stdio.h>


void test()
{
    if(0 != machineInit())
        return;
//    ScanMode::createObj("/home/echo/Desktop/test.png", Qt::OrderedDither)->start();
//    ScanMode::createObj("/home/echo/Desktop/test.png")->start();
//    ScanMode::createObj("/home/echo/Desktop/test.png")->start();
    NCMode::createObj("/home/echo/Desktop/test.nc")->start();
//    ContinuationMode::createObj("/home/echo/Desktop/test.png", Qt::ThresholdDither)->start();
//    ContinuationMode::createObj("/home/echo/Desktop/test.png", Qt::OrderedDither)->start();
}

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
//    test();
    return a.exec();
}
