#ifndef NCWINDOW_HH
#define NCWINDOW_HH

#include <QWidget>
#include <QPushButton>
#include <QImage>
#include <QTextEdit>
#include <QLabel>
#include "logtext.hh"
#include "ncmode.hh"

class NCWindow : public QWidget
{
    Q_OBJECT
public:
    explicit NCWindow(QWidget *parent = 0);
    void prepare(QString file);

private:
    NCMode* mMode;
    QPushButton* mStartBtn;
    QPushButton* mAbortBtn;
    QPushButton* mPauseBtn;
    QPushButton* mResumeBtn;
    LogText* mLogText;
    QTextEdit* mNCText;
    QLabel* mTimeLable;
    QLabel* mCoordLable;
    virtual void timerEvent( QTimerEvent *event);
signals:

public slots:
    void doStart(bool clicked);
    void doAbort(bool clicked);
    void doPause(bool clicked);
    void doResume(bool clicked);
    void doExit();

};

#endif // NCWINDOW_HH
