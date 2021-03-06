#include "bitmapwindow.hh"
#include "scanmode.hh"
#include "continuationmode.hh"
#include <QPainter>
#include <QCoreApplication>

#define PREVIEW_SIZE 250.0
#define PREVIEW_AREA 272

bool imagePrepared = false;
bool imageRejected = false;

BitmapWindow::BitmapWindow(QWidget *parent) :
    QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    mImage = NULL;
    mBitmapMode = NULL;
    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(update()));
    mTimer->start(300);

    pthread_mutex_init(&mMutex, NULL);

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
    setAttribute(Qt::WA_DeleteOnClose);

    mMessageBox = new QMessageBox(this);
    mMessageBox = new QMessageBox(QMessageBox::NoIcon ,"transforming..."
                                  , "wait for a minute...", QMessageBox::NoButton, this);
    mMessageBox->setText("transfroming image, wait for a minute ...");
    mMessageBox->setStandardButtons(QMessageBox::Cancel);

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
    if(!imagePrepared)
        return;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
//    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(250, 250, 200));
    painter.drawRect(480 - PREVIEW_AREA, 0 , 480, PREVIEW_AREA);
    painter.setBrush(QColor(0, 0, 0));
    painter.drawText(130, 40, QString("X:") + QString::number(coordX) + "  Y:" + QString::number(coordY));
    pthread_mutex_lock(&mMutex);
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
        if(mBitmapMode){
            painter.drawText(130, 90, QString("cost ") + QString::number(mBitmapMode->getTime()) + " S");
        }
    }
    pthread_mutex_unlock(&mMutex);
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


void* prepareImage(void* arg)
{
    //LogUtil::debug("thread running");
    BitmapWindow* window = (BitmapWindow*)arg;
    pthread_mutex_lock(&window->mMutex);
    if(window->mBitmapMode)
        window->mBitmapMode->cancel();
    window->mImage = NULL;
    window->mBitmapMode = NULL;
    pthread_mutex_unlock(&window->mMutex);
    Qt::ImageConversionFlags flag = Qt::DiffuseDither;
    int flagIndex = window->mDiffBox->currentIndex();
    if(1 == flagIndex)
        flag = Qt::OrderedDither;
    else if(2 == flagIndex)
        flag = Qt::ThresholdDither;

    if(window->mFile.endsWith("jpg",Qt::CaseInsensitive) || window->mFile.endsWith("jpeg",Qt::CaseInsensitive))
    {
        QString cmd = QString("cat ") + window->mFile + " | /root/workspace/jpegtopng > /tmp/transformed.png";
        LogUtil::debug("it's a jpeg image, it will take some time to decompress it ... ");
        system(cmd.toAscii().constData());
        LogUtil::debug("decompress jpeg image finished.");
        window->mFile = "/tmp/transformed.png";
    }

    if(0 == window->mModeBox->currentIndex())
        window->mBitmapMode = ContinuationMode::createObj(window->mFile.toAscii().constData(), flag);
    else
        window->mBitmapMode = ScanMode::createObj(window->mFile.toAscii().constData(), flag);
    window->mImage = window->mBitmapMode->getImageP();
    QObject::connect(window->mBitmapMode, SIGNAL(threadFinished()), window, SLOT(close()));
#if 0
    window->mStartBtn->setDisabled(false);
    window->mAbortBtn->setDisabled(false);
    window->mPauseBtn->setDisabled(true);
    window->mResumeBtn->setDisabled(true);
#endif
//    window->mMessageBox->close();
    imagePrepared = true;
    //LogUtil::debug("prepareImage finished");
    //window->mMessageBox->close();
   // window->mMessageBox->accept();
    //LogUtil::debug("accept called");
    //window->mMessageBox->done(QMessageBox::Ok);
    return NULL;
}

void BitmapWindow::reject()
{
    imageRejected = true;
}

bool BitmapWindow::prepare(QString file)
{
    mFile = file;
    //LogUtil::info("start to tranform image, wait for a minute ...");
    imagePrepared = false;
    imageRejected = false;
//#if 0
    mDiffBox->setDisabled(true);
    mModeBox->setDisabled(true);

    mStartBtn->setDisabled(true);
 //   mAbortBtn->setDisabled(true);
 //   mPauseBtn->setDisabled(true);
 //   mResumeBtn->setDisabled(true);
//# endif
    //LogUtil::debug("to create thread");
    pthread_t thread =createThread(prepareImage, this);
    //LogUtil::debug("thread created");
//    QCoreApplication::processEvents();
//    while(!imagePrepared)
//        usleep(100000);


//#if 0
    if(!imagePrepared)
    {
        LogUtil::debug("image not prepared");
        mMessageBox->show();
        connect(mMessageBox, SIGNAL(rejected()), this, SLOT(reject()));
        while(!imagePrepared)
        {
            QCoreApplication::processEvents();
            if(imageRejected)
            {
                pthread_cancel(thread);
                this->close();
                return false;
            }
            usleep(10000);
        }
        //mMessageBox->accept();
        mMessageBox->close();
//        if(QMessageBox::Cancel == mMessageBox->exec())
//        {
//            pthread_cancel(thread);
//            this->close();
//            return false;
//        }
    }
    //LogUtil::debug("image prepared");
    mDiffBox->setDisabled(false);
    mModeBox->setDisabled(false);

    mStartBtn->setDisabled(false);
    mAbortBtn->setDisabled(false);
    mPauseBtn->setDisabled(true);
    mResumeBtn->setDisabled(true);
    return true;
}


