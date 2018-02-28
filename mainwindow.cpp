#include "mainwindow.hh"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QScrollBar>
#include "homewidget.hh"
#include "logutil.hh"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


//    ui->textEdit->insertHtml("<font color='green'>DEBUG: ferfew123321.</font><br/>");
//    ui->textEdit->verticalScrollBar()->setSliderPosition(ui->textEdit->verticalScrollBar()->maximum());
//    ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
//    ui->textEdit->verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
//    ui->textEdit->verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMinimum);

//    ui->textEdit->document()->setMaximumBlockCount(1000);
//    QTextCursor c = ui->textEdit->textCursor();
//    c.movePosition(QTextCursor::End);
//    ui->textEdit->setTextCursor(c);
//    ui->textEdit->insertHtml("<font color='green'>DEBUG: ferfew123321.<br/></font>");
//    ui->textEdit->insertHtml("<font color='red'>ERROR: ferfew1ddddddced3321.<br/></font>");
//    ui->textEdit->ensureCursorVisible();

    resize(480, 272);
    QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    setSizePolicy(sizePolicy);
    setMinimumSize(QSize(480, 272));
    setMaximumSize(QSize(480, 272));
//    setCursor(QCursor(Qt::ArrowCursor));
    setAcceptDrops(false);
    setLocale(QLocale(QLocale::Chinese, QLocale::China));
    setCentralWidget(new HomeWidget(this));
    setWindowFlags(Qt::FramelessWindowHint);

    QPalette palette = QPalette();
    palette.setColor(QPalette::Window, QColor(255, 255, 255));
    setPalette(palette);
//    setBackgroundRole(QPalette::Window);

//    for(int i=0; i<4; i++){
//        LOG_INFO("info test %d", i);
//        LOG_WARNING("warning test %d", i);
//        LOG_ERROR("error test %d", i);
//        LOG_DEBUG("debug test %d", i);
//    }
//    QMessageBox::critical(NULL, "test", "test");

}

MainWindow::~MainWindow()
{
    delete ui;
}
