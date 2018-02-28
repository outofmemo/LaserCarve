#include "basicmode.hh"

BasicMode::BasicMode()
{
    _pauseFlag = false;
    mThread = 0;
    mBeginTime = mPauseBeginTime = mPauseTime = 0;
    sObjNum ++;
}

BasicMode::~BasicMode()
{
    mThread = 0;
    sObjNum --;
}

int BasicMode::sObjNum = 0;
/*
 * NOTICE： 在调用start（）后会在新线程中运行run（）。
 * 若对象声明为局部变量，函数退出时， run（）可能仍在运行。
 * 故必须使用`new`获得对象，构造方法设为protected，外部调用CreateObj（）。
 * run（）完成后或调用cancel（）时对象自杀，无需外部delete
 * sObjNum记录对象数量。通常同时只需一个对象，0==getObjNum()即可判断对象是否被析构
*/

/*no need, abstract class*/
/*
static BasicMode* BasicMode::CreateObj()
{
    return new BasicMode();
}
*/

int BasicMode::getTime()
{
    if(0 == mBeginTime)
        return 0;
    if(_pauseFlag)
    {
        return mPauseBeginTime - mBeginTime - mPauseTime;
    }else{
        return time(NULL) - mBeginTime - mPauseTime;
    }
}

void BasicMode::pause()
{
    if(_pauseFlag)
        return;
    _pauseFlag = true;
    mPauseBeginTime = time(NULL);
    LogUtil::info("pause.");
}

void BasicMode::resume()
{
    if(!_pauseFlag)
        return;
    _pauseFlag = false;
    mPauseTime += (time(NULL) - mPauseBeginTime);
    LogUtil::info("resume.");
}

void BasicMode::cancel()
{
    if(mThread)
    {
        _pauseFlag = true;
        pthread_t thread = mThread;
        delete this;
        /*NOTICE: cancel() may be called by self, thus the codes after `pthread_cancel` won't work*/
        /*NOTICE: don't use `mThread` directly after `delete this`*/
        laserOff();
        motorDisable();
        pthread_cancel(thread);
        LogUtil::info("thread canceled!");
    }else{
        LogUtil::warning("thread not start.");
        delete this;
    }

}

void* BasicMode::thread_entry(void* arg)
{
    machineReset();
    motorEnable();
    ((BasicMode*)arg)->mBeginTime = time(NULL);
    ((BasicMode*)arg)->run();
    machineReset();
    LogUtil::info("thread finished, cost %d S.", ((BasicMode*)arg)->getTime());
    emit ((BasicMode*)arg)->threadFinished();
//    delete ((BasicMode*)arg)
    ((BasicMode*)arg)->deleteLater();
    return NULL;
}

void BasicMode::start()
{
//    QObject::connect(this, SIGNAL(threadFinished()), this, SLOT(defaultExitHandler()));
    if(mThread){
        LogUtil::warning("thread already started, ignore.");
        return;
    }
    mThread = createThread(BasicMode::thread_entry, this);
}

void BasicMode::checkPause()
{
    if(machineExit)
    {
        LogUtil::info("machine exit, do cancel!");
        cancel();
    }
    if(_pauseFlag)
    {
        bool __laserStatue = laserFDStatue;
        laserOff();
        motorDisable();
        while (_pauseFlag)
            sleep(1);
        if(__laserStatue)
            laserOn();
        motorEnable();
    }
}

void BasicMode::defaultExitHandler()
{
    exit(0);
}
