#ifndef BASICMODE_HH
#define BASICMODE_HH

#include <QObject>
#include <time.h>
#include "machine_base.hh"

class BasicMode : public QObject
{
    Q_OBJECT

public:
    void pause();
    void resume();
    void cancel();
    void start();
    int getTime();
    ~BasicMode();
    static inline int getObjNum(){return BasicMode::sObjNum;}
//    static BasicMode* CreateObj();
    virtual void run()=0;
protected:
    BasicMode();
    void checkPause();
    static int sObjNum;
private:
    time_t mBeginTime;
    time_t mPauseBeginTime;
    time_t mPauseTime;
    bool _pauseFlag;
    pthread_t mThread;
    static void* thread_entry(void* arg);
public slots:
    void defaultExitHandler();
signals:
    void threadFinished();

};

#endif // BASICMODE_HH
