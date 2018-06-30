#ifndef BITMAPWINDOW_HH
#define BITMAPWINDOW_HH

#include <QWidget>
#include <QImage>
#include <QTimer>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QLabel>
#include <QMessageBox>
#include "logtext.hh"
#include "bitmapmode.hh"

class BitmapWindow : public QWidget
{
    Q_OBJECT
public:
    explicit BitmapWindow(QWidget *parent = 0);
    bool prepare(QString file);
    ~BitmapWindow();
    friend void* prepareImage(void* arg);

protected:
    void paintEvent(QPaintEvent * event);

private:
    QImage* mImage;
    LogText* mLogText;
    QTimer* mTimer;
    QMessageBox* mMessageBox;
    pthread_mutex_t mMutex;
    QPushButton* mStartBtn;
    QPushButton* mAbortBtn;
    QPushButton* mPauseBtn;
    QPushButton* mResumeBtn;
    BitmapMode* mBitmapMode;
    QSlider* mSlider;
    QComboBox* mDiffBox;
    QComboBox* mModeBox;
    QLabel* mDelayLabel;
    QString mFile;

signals:

public slots:
    void doStart(bool clicked);
    void doAbort(bool clicked);
    void doPause(bool clicked);
    void doResume(bool clicked);
    void modeConfigChange(int index);
    void delayChange(int value);
    void reject();

};

#endif // BITMAPWINDOW_HH
