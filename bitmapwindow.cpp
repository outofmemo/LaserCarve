#include "bitmapwindow.hh"
#include "scanmode.hh"
#include "continuationmode.hh"
#include <QPainter>

#define PREVIEW_SIZE 250.0
#define PREVIEW_AREA 272

BitmapWindow::BitmapWindow(QWidget *parent) :
    QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    mImage = NULL;
    mBitmapMode = NULL;
    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(update()));
    mTimer->start(300);

    QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    setSizePolicy(sizePolicy);
    setMinimumSize(QSize(480, 272));
    setMaximumSize(QSize(480, 272));
    resize(480, 272);
    setCursor(QCursor(Qt::BlankCursor));
    setAcceptDrops(false);
    setWindowFlags(Qt::FramelessWindowHint);

    mStartBtn = new QPushButton(tr("Start"), this);
    mStartBtn->setGeometry(10, 20, 50, 40);
    mStartBtn->setDisabled(true);
    mAbortBtn = new QPushButton(tr("Abort"), this);
    mAbortBtn->setGeometry(70, 20, 50, 40);
    mAbortBtn->setDisabled(false);
    mPauseBtn = new QPushButton(tr("Pause"), this);
    mPauseBtn->setGeometry(10, 70, 50, 40);
    mPauseBtn->setDisabled(true);
    mResumeBtn = new QPushButton(tr("Resume"), this);
    mResumeBtn->setGeometry(70, 70, 50, 40);
    mResumeBtn->setDisabled(true);

    mDiffBox = new QComboBox(this);
    mDiffBox->setGeometry(QRect(0, 120, 95, 30));
    mDiffBox->insertItems(0, QStringList()
            << tr("DiffuseDither")
            << tr("OrderedDither")
            << tr("ThresholdDither")
           );

    mModeBox = new QComboBox(this);
    mModeBox->setGeometry(QRect(105, 120, 95, 30));
    mModeBox->insertItems(0, QStringList()
            << tr("ContinuationMode")
            << tr("ScanMode")
           );

    mSlider = new QSlider(this);
    mSlider->setGeometry(QRect(60, 160, 140, 20));
    mSlider->setMinimum(0);
    mSlider->setMaximum(80);
    mSlider->setValue(pixelDuration);
    mSlider->setOrientation(Qt::Horizontal);


    mDelayLabel = new QLabel(QString::fromUtf8("delay(") + QString::number(mSlider->value())
                               + QString::fromUtf8(")"), this);
    mDelayLabel->setGeometry(2, 160, 60, 20);

    mLogText = new LogText(this);
    mLogText->setGeometry(QRect(0, 180, 480 - PREVIEW_AREA, 272 - 180));

    connect(mStartBtn, SIGNAL(clicked(bool)), this, SLOT(doStart(bool)));
    connect(mAbortBtn, SIGNAL(clicked(bool)), this, SLOT(doAbort(bool)));
    connect(mPauseBtn, SIGNAL(clicked(bool)), this, SLOT(doPause(bool)));
    connect(mResumeBtn, SIGNAL(clicked(bool)), this, SLOT(doResume(bool)));
    connect(mDiffBox, SIGNAL(currentIndexChanged(int)), this, SLOT(modeConfigChange(int)));
    connect(mModeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(modeConfigChange(int)));
    connect(mSlider, SIGNAL(valueChanged(int)), this, SLOT(delayChange(int)));

}

BitmapWindow::~BitmapWindow()
{
    /*Qt will delete they automatically*/
//    delete mLogText;
//    delete mTimer;

}

void BitmapWindow::delayChange(int value)
{
    mDelayLabel->setText(QString::fromUtf8("delay(") + QString::number(value) + QString::fromUtf8(")"));
    pixelDuration = value;
}

void BitmapWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
//    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(250, 250, 200));
    painter.drawRect(480 - PREVIEW_AREA, 0 , 480, PREVIEW_AREA);
    if(mImage){
        float rate, ratex, ratey;
        ratex = PREVIEW_SIZE / mImage->width();
        ratey = PREVIEW_SIZE / mImage->height();
        if(ratex>=1 && ratey>=1)
            rate = 1;
        else
            rate = qMin(ratex, ratey);

        int tw = mImage->width()*rate;
        int th = mImage->height()*rate;
        int tx = 480 - PREVIEW_AREA + (PREVIEW_AREA - PREVIEW_SIZE) / 2 + (PREVIEW_SIZE - tw) / 2;
        int ty = (PREVIEW_AREA - PREVIEW_SIZE) / 2 + (PREVIEW_SIZE - th) / 2;
        painter.drawImage(QRect(tx, ty, tw, th), *mImage);
    }
    painter.setBrush(QColor(0, 0, 0));
    painter.drawText(130, 40, QString("X:") + QString::number(coordX) + "  Y:" + QString::number(coordY));
    if(mBitmapMode){
        painter.drawText(130, 90, QString("cost ") + QString::number(mBitmapMode->getTime()) + " S");
    }
}


void BitmapWindow::doStart(bool)
{
    if(!mBitmapMode)
        return;
    mStartBtn->setDisabled(true);
    mAbortBtn->setDisabled(false);
    mPauseBtn->setDisabled(false);
    mResumeBtn->setDisabled(true);
    pixelDuration = mSlider->value();
    mBitmapMode->start();
}


void BitmapWindow::doAbort(bool)
{
    if(mBitmapMode)
        mBitmapMode->cancel();
    mStartBtn->setDisabled(true);
    mAbortBtn->setDisabled(true);
    mPauseBtn->setDisabled(true);
    mResumeBtn->setDisabled(true);
    this->close();
}

void BitmapWindow::doResume(bool)
{
    if(!mBitmapMode)
        return;
    mBitmapMode->resume();
    mResumeBtn->setDisabled(true);
    mPauseBtn->setDisabled(false);
    mStartBtn->setDefault(true);
    mAbortBtn->setDefault(false);
}

void BitmapWindow::doPause(bool)
{
    if(!mBitmapMode)
        return;
    mBitmapMode->pause();
    mResumeBtn->setDisabled(false);
    mPauseBtn->setDisabled(true);
    mStartBtn->setDisabled(true);
    mAbortBtn->setDisabled(false);
}

void BitmapWindow::modeConfigChange(int)
{
    if(mFile.isEmpty())
        return;
    prepare(mFile);
}

void BitmapWindow::prepare(QString file)
{
    if(mBitmapMode)
        mBitmapMode->cancel();
    Qt::ImageConversionFlags flag = Qt::DiffuseDither;
    int flagIndex = mDiffBox->currentIndex();
    if(1 == flagIndex)
        flag = Qt::OrderedDither;
    else if(2 == flagIndex)
        flag = Qt::ThresholdDither;
    if(0 == mModeBox->currentIndex())
        mBitmapMode = ContinuationMode::createObj(file.toAscii().constData(), flag);
    else
        mBitmapMode = ScanMode::createObj(file.toAscii().constData(), flag);
    mImage = mBitmapMode->getImageP();
    mFile = file;
    connect(mBitmapMode, SIGNAL(threadFinished()), this, SLOT(close()));
    mStartBtn->setDisabled(false);
    mAbortBtn->setDisabled(false);
    mPauseBtn->setDisabled(true);
    mResumeBtn->setDisabled(true);
}


