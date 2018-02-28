#include "homewidget.hh"
#include "machine_base.hh"
#include "bitmapwindow.hh"
#include "logtext.hh"
#include "ncwindow.hh"
#include <QPushButton>
#include <QPalette>
#include <QTimerEvent>
#include <QFileDialog>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QMessageBox>
#include <QPainter>

#define DIRECT_GEOM_X 70
#define DIRECT_GEOM_Y 15
#define DIRECT_BTN_SIZE 50
#define DIRECT_REPEAT_TIME 30
#define MOVE_STEP 1

HomeWidget::HomeWidget(QWidget *parent) :
    QWidget(parent)
{
    disableTime = 0;
    QPalette palette = QPalette();
    palette.setColor(QPalette::Window, QColor(255, 255, 255));
    palette.setColor(QPalette::Button, QColor(230, 220, 230));
    setPalette(palette);
    setBackgroundRole(QPalette::Window);
    LogText* logText = new LogText(this);
    logText->setGeometry(QRect(0, 180, 480, 92));
    logText->show();

    QPushButton* upBtn = new QPushButton(QIcon(":/icon/res/ic_move_up.png"), "", this);
    upBtn->setIconSize(QSize(DIRECT_BTN_SIZE, DIRECT_BTN_SIZE));
    upBtn->setGeometry(QRect(DIRECT_BTN_SIZE + DIRECT_GEOM_X, DIRECT_GEOM_Y, DIRECT_BTN_SIZE, DIRECT_BTN_SIZE));
    upBtn->setFlat(true);
    upBtn->setAutoRepeat(true);
    upBtn->setAutoRepeatDelay(DIRECT_REPEAT_TIME);
    upBtn->setAutoRepeatInterval(DIRECT_REPEAT_TIME);
    upBtn->show();
    connect(upBtn, SIGNAL(clicked(bool)), this, SLOT(moveUp(bool)));

    QPushButton* downBtn = new QPushButton(QIcon(":/icon/res/ic_move_down.png"), "", this);
    downBtn->setIconSize(QSize(DIRECT_BTN_SIZE, DIRECT_BTN_SIZE));
    downBtn->setGeometry(QRect(DIRECT_BTN_SIZE + DIRECT_GEOM_X, DIRECT_BTN_SIZE * 2 + DIRECT_GEOM_Y
                             , DIRECT_BTN_SIZE, DIRECT_BTN_SIZE));
    downBtn->setFlat(true);
    downBtn->setAutoRepeat(true);
    downBtn->setAutoRepeatDelay(DIRECT_REPEAT_TIME);
    downBtn->setAutoRepeatInterval(DIRECT_REPEAT_TIME);
    downBtn->show();
    connect(downBtn, SIGNAL(clicked(bool)), this, SLOT(moveDown(bool)));

    QPushButton* upLeft = new QPushButton(QIcon(":/icon/res/ic_move_left.png"), "", this);
    upLeft->setIconSize(QSize(DIRECT_BTN_SIZE, DIRECT_BTN_SIZE));
    upLeft->setGeometry(QRect(DIRECT_GEOM_X, DIRECT_BTN_SIZE + DIRECT_GEOM_Y, DIRECT_BTN_SIZE, DIRECT_BTN_SIZE));
    upLeft->setFlat(true);
    upLeft->setAutoRepeat(true);
    upLeft->setAutoRepeatDelay(DIRECT_REPEAT_TIME);
    upLeft->setAutoRepeatInterval(DIRECT_REPEAT_TIME);
    upLeft->show();
    connect(upLeft, SIGNAL(clicked(bool)), this, SLOT(moveLeft(bool)));

    QPushButton* rightBtn = new QPushButton(QIcon(":/icon/res/ic_move_right.png"), "", this);
    rightBtn->setIconSize(QSize(DIRECT_BTN_SIZE, DIRECT_BTN_SIZE));
    rightBtn->setGeometry(QRect(DIRECT_BTN_SIZE * 2 + DIRECT_GEOM_X, DIRECT_BTN_SIZE + DIRECT_GEOM_Y
                             , DIRECT_BTN_SIZE, DIRECT_BTN_SIZE));
    rightBtn->setFlat(true);
    rightBtn->setAutoRepeat(true);
    rightBtn->setAutoRepeatDelay(DIRECT_REPEAT_TIME);
    rightBtn->setAutoRepeatInterval(DIRECT_REPEAT_TIME);
    rightBtn->show();
    connect(rightBtn, SIGNAL(clicked(bool)), this, SLOT(moveRight(bool)));

    laserBtn = new QPushButton(QIcon(":/icon/res/ic_laser_black.png"), tr("开/关"), this);
    laserBtn->setIconSize(QSize(DIRECT_BTN_SIZE * 1 / 2, DIRECT_BTN_SIZE * 1 / 2));
    laserBtn->setGeometry(QRect(DIRECT_BTN_SIZE + DIRECT_GEOM_X, DIRECT_BTN_SIZE + DIRECT_GEOM_Y
                                , DIRECT_BTN_SIZE, DIRECT_BTN_SIZE));
    laserBtn->setFlat(true);
    laserBtn->setCheckable(true);
    laserBtn->setAutoFillBackground(true);
    laserBtn->show();
    connect(laserBtn, SIGNAL(clicked(bool)), this, SLOT(turnLaser(bool)));

    QPushButton* resetBtn = new QPushButton(QIcon(":/icon/res/ic_machine_reset.png"), tr("复位"), this);
    resetBtn->setIconSize(QSize(DIRECT_BTN_SIZE * 1 / 2, DIRECT_BTN_SIZE * 1 / 2));
    resetBtn->setGeometry(QRect(DIRECT_GEOM_X, DIRECT_BTN_SIZE * 2 + DIRECT_GEOM_Y
                                , DIRECT_BTN_SIZE, DIRECT_BTN_SIZE));
    resetBtn->setFlat(true);
//    resetBtn->setAutoFillBackground(true);
    resetBtn->show();
    connect(resetBtn, SIGNAL(clicked(bool)), this, SLOT(returnZero(bool)));

    QPushButton* markBtn = new QPushButton(QIcon(":/icon/res/ic_set_point.png"), tr("置零"), this);
    markBtn->setIconSize(QSize(DIRECT_BTN_SIZE * 1 / 2, DIRECT_BTN_SIZE * 1 / 2));
    markBtn->setGeometry(QRect(DIRECT_BTN_SIZE * 2 + DIRECT_GEOM_X, DIRECT_BTN_SIZE * 2 + DIRECT_GEOM_Y
                                , DIRECT_BTN_SIZE, DIRECT_BTN_SIZE));
    markBtn->setFlat(true);
//    markBtn->setAutoFillBackground(true);
    markBtn->show();
    connect(markBtn, SIGNAL(clicked(bool)), this, SLOT(setZero(bool)));

    QPushButton* openBtn = new QPushButton(tr("Open File"), this);
    openBtn->setGeometry(QRect(300, 80, 80, 60));
    openBtn->setFlat(true);
    openBtn->setAutoFillBackground(true);
    openBtn->show();
    connect(openBtn, SIGNAL(clicked(bool)), this, SLOT(openFile(bool)));

    coordLable = new QLabel("X:-1    Y:-1", this);
    coordLable->setGeometry(QRect(310, 140, 100, 50));
    startTimer(300);

    setCursor(QCursor(Qt::BlankCursor));

    machineInit();
}

void HomeWidget::moveLeft(bool)
{
    if(coordX - MOVE_STEP < 0)
        return;
    disableDelay();
    reverseX(MOVE_STEP);
}

void HomeWidget::moveUp(bool)
{
    if(coordY - MOVE_STEP < 0)
        return;
    disableDelay();
    reverseY(MOVE_STEP);
}

void HomeWidget::moveRight(bool)
{
    if(coordX + MOVE_STEP > maxX)
        return;
    disableDelay();
    stepX(MOVE_STEP);
}

void HomeWidget::moveDown(bool)
{
    if(coordY + MOVE_STEP > maxY)
        return;
    disableDelay();
    stepY(MOVE_STEP);
}

void HomeWidget::setZero(bool)
{
    if(QMessageBox::Ok == QMessageBox::question(this, tr("Set as origin point")
    , tr("Sure to set the current point as the origin point?")
    , QMessageBox::Yes|QMessageBox::No, QMessageBox::No)){
        coordX = coordY = 0;
        saveCoord();
    }
}

void HomeWidget::returnZero(bool)
{
    if(disableTime){
        killTimer(disableTime);
        disableTime = 0;
    }
    machineReset();
}

void HomeWidget::turnLaser(bool checked)
{
    if(checked){
        laserOn();
        laserBtn->setIcon(QIcon(":/icon/res/ic_laser_red.png"));
    }else{
        laserOff();
        laserBtn->setIcon(QIcon(":/icon/res/ic_laser_black.png"));
    }
}

void HomeWidget::openFile(bool)
{
    QFileDialog fDlg(this);
    fDlg.setFileMode(QFileDialog::ExistingFile);
    fDlg.setViewMode(QFileDialog::List);
    fDlg.setAcceptDrops(false);
//    fDlg.setDirectory("/");
    fDlg.setGeometry(0, 0, 480, 272);
    QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    fDlg.setSizePolicy(sizePolicy);
//    fDlg.setNameFilter(tr("Supported files (*.bmp *.gif *.jpg *.jpeg *.png *.tiff *.txt *.nc)"));
    QStringList filters;
    filters << "Supported files (*.bmp *.gif *.jpg *.jpeg *.png *.tiff *.txt *.nc)"
            << "Image files (*.bmp *.gif *.jpg *.jpeg *.png *.tiff)"
            << "NC files (*.txt *.nc)"
            << "All files (*)";
    fDlg.setNameFilters(filters);
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/")
         << QUrl::fromLocalFile("/root")
         << QUrl::fromLocalFile("/home")
         << QUrl::fromLocalFile("/media")
         << QUrl::fromLocalFile("/media/sd-mmcblk0p1");
    fDlg.setSidebarUrls(urls);
    fDlg.setCursor(QCursor(Qt::BlankCursor));
    if(!fDlg.exec())
        return;
    QStringList files = fDlg.selectedFiles();
    if(0 == files.size())
        return;
    QString file = files[0];
    if(file.endsWith(QString(".nc"), Qt::CaseInsensitive)
            || file.endsWith(QString(".txt"), Qt::CaseInsensitive))
    {
        NCWindow* ncWindow = new NCWindow();
        ncWindow->prepare(file);
        ncWindow->show();
    }else{
        LogUtil::debug("before new");
        BitmapWindow* bitmapWindow = new BitmapWindow();
        LogUtil::debug("before prepare");
        bitmapWindow->prepare(file);
        LogUtil::debug("before show");
        bitmapWindow->show();
    }

}

void HomeWidget::timerEvent(QTimerEvent *event)
{
    if(disableTime == event->timerId()){
        motorDisable();
        killTimer(disableTime);
        disableTime = 0;
    }else
        coordLable->setText(QString("X:") + QString::number(coordX) + "    Y:" + QString::number(coordY));
}

void HomeWidget::disableDelay()
{
    motorEnable();
    if(disableTime)
        killTimer(disableTime);
    disableTime = startTimer(1000);
}

void HomeWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(210, 220, 210));
    painter.drawRoundedRect(DIRECT_GEOM_X, DIRECT_GEOM_Y, DIRECT_BTN_SIZE * 3, DIRECT_BTN_SIZE * 3, 20, 20);
}
