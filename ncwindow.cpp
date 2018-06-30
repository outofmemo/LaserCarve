#include "ncwindow.hh"
#include <QMessageBox>

#define PREVIEW_AREA 272

NCWindow::NCWindow(QWidget *parent) :
    QWidget(parent)
{

    setAttribute(Qt::WA_DeleteOnClose);
    mMode = NULL;
//    mTimer = new QTimer(this);
//    connect(mTimer, SIGNAL(timeout()), this, SLOT(update()));
//    mTimer->start(300);

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

    mLogText = new LogText(this);
    mLogText->setGeometry(QRect(0, 120, 480 - PREVIEW_AREA, 272 - 120));

    mNCText = new QTextEdit(this);
    mNCText->setReadOnly(true);
    mNCText->setAcceptRichText(true);
    mNCText->setGeometry(480 - PREVIEW_AREA, 0, PREVIEW_AREA, PREVIEW_AREA);

    mTimeLable = new QLabel(this);
    mCoordLable = new QLabel(this);
    mTimeLable->setGeometry(130, 30, 80, 30);
    mCoordLable->setGeometry(130, 70, 80, 60);

    connect(mStartBtn, SIGNAL(clicked(bool)), this, SLOT(doStart(bool)));
    connect(mAbortBtn, SIGNAL(clicked(bool)), this, SLOT(doAbort(bool)));
    connect(mPauseBtn, SIGNAL(clicked(bool)), this, SLOT(doPause(bool)));
    connect(mResumeBtn, SIGNAL(clicked(bool)), this, SLOT(doResume(bool)));
    mTimer = startTimer(300);
}


void NCWindow::prepare(QString file)
{
    if(mMode)
        mMode->cancel();
    mMode = NCMode::createObj(file.toAscii().constData());
    connect(mMode, SIGNAL(threadFinished()), this, SLOT(doExit()));
    connect(mMode, SIGNAL(NCError(QString)), this, SLOT(doError(QString)));
    mStartBtn->setDisabled(false);
    mAbortBtn->setDisabled(false);
    mPauseBtn->setDisabled(true);
    mResumeBtn->setDisabled(true);
}

void NCWindow::doError(QString error)
{
    QMessageBox::critical(NULL, "NC code error", error);
}

void NCWindow::doExit()
{
    //mMode = NULL;
    killTimer(mTimer);
    this->close();
}

void NCWindow::timerEvent(QTimerEvent *)
{
    float x = NCMode::coordToMM(COORD_X);
    float y = NCMode::coordToMM(COORD_Y);
    mCoordLable->setText(QString("X:%1\nY:%2").arg(x, 0, 'f', 1).arg(y, 0, 'f', 1));
    if(mMode)
        mTimeLable->setText(QString("line:%1 \ncost %L2 S").arg(mMode->getIndex()+1).arg(mMode->getTime()));
    else
        mTimeLable->setText("");

    if(!mMode)
        return;
    mNCText->clear();
    QStringList lines = mMode->getLines();
    int index = mMode->getIndex();
    if(lines.size() > index)
    {
        mNCText->insertHtml(QString("<font color='red'>") + lines[index] + "<br/></font>");
        for(int i=index+1; i<lines.size(); i++){
            mNCText->insertHtml(QString("<font color='black'>") + lines[i] + "<br/></font>");
        }
    }
    QTextCursor cursor = mNCText->textCursor();
    cursor.movePosition(QTextCursor::Start);
    mNCText->setTextCursor(cursor);
    for(int i=0; i<index; i++){
         mNCText->insertHtml(QString("<font color='gray'>") + lines[i] + "<br/></font>");
    }
    mNCText->ensureCursorVisible();

}

void NCWindow::doStart(bool)
{
    if(!mMode)
        return;
    mStartBtn->setDisabled(true);
    mAbortBtn->setDisabled(false);
    mPauseBtn->setDisabled(false);
    mResumeBtn->setDisabled(true);
    mMode->start();
}


void NCWindow::doAbort(bool)
{
    if(mMode)
        mMode->cancel();
    mStartBtn->setDisabled(true);
    mAbortBtn->setDisabled(true);
    mPauseBtn->setDisabled(true);
    mResumeBtn->setDisabled(true);
    this->close();
}

void NCWindow::doResume(bool)
{
    if(!mMode)
        return;
    mMode->resume();
    mResumeBtn->setDisabled(true);
    mPauseBtn->setDisabled(false);
    mStartBtn->setDefault(true);
    mAbortBtn->setDefault(false);
}

void NCWindow::doPause(bool)
{
    if(!mMode)
        return;
    mMode->pause();
    mResumeBtn->setDisabled(false);
    mPauseBtn->setDisabled(true);
    mStartBtn->setDisabled(true);
    mAbortBtn->setDisabled(false);
}



