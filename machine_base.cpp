#include "machine_base.hh"
#include <signal.h>

int laserFD, enableFD, xMotorFD, xDirectFD, yMotorFD, yDirectFD;
bool laserFDStatue, enableFDStatue, xMotorFDStatue, xDirectFDStatue, yMotorFDStatue, yDirectFDStatue;
int const gpioNums[] = {GPIO_LASER, GPIO_ENABLE, GPIO_X_MOTOR, GPIO_X_DIRECTION
                        , GPIO_Y_MOTOR, GPIO_Y_DIRECTION};
int* const pgpioFDs[] = {&laserFD, &enableFD, &xMotorFD, &xDirectFD, &yMotorFD, &yDirectFD};
bool allowLaserWhenMove = false;
bool machineExit = false;
static pthread_mutex_t goto_mutex;

int pixelDuration = 30;
int stepDuration = 3;
int pixelSize, coordX, coordY;
int maxX, maxY;

int initFD()
{
    int ret = 0;
    int exportFD = open("/sys/class/gpio/export", O_WRONLY);
    if(exportFD == -1)
    {
        LogUtil::error("open `export` fail: %s", strerror(errno));
        ret = -1;
        goto initfd_out;
    }
    for(int i = 0; i < ARRY_SIZE(gpioNums); i++)
    {
        char buf[100];
        sprintf(buf, "%d", gpioNums[i]);
        if(write(exportFD, buf, 4) == -1)
        {
            if(errno != EBUSY)
            {
                LogUtil::error("write %d to `export` fail: %s", gpioNums[i], strerror(errno));
                ret = -2;
                goto initfd_out;
            }
        }
        sprintf(buf, "/sys/class/gpio/gpio%d/direction", gpioNums[i]);
        LogUtil::info("opening %s ...", buf);
        int direction = open(buf, O_WRONLY);
        if(direction == -1)
        {
            LogUtil::error("open %s fail: %s", buf, strerror(errno));
            ret = -3;
            goto initfd_out;
        }
        if(write(direction, "out", 4) == -1)
        {
            LogUtil::error("write `out` to %s fail: %s", buf, strerror(errno));
            ret = -4;
            goto initfd_out;
        }
        close(direction);
        sprintf(buf, "/sys/class/gpio/gpio%d/value", gpioNums[i]);
        LogUtil::info("opening %s ...", buf);
        if((*pgpioFDs[i] = open(buf, O_WRONLY)) == -1)
        {
            LogUtil::error("open %s fail: %s", buf, strerror(errno));
            ret = -5;
            goto initfd_out;
        }
    }
initfd_out:
    if(exportFD > 0)
        close(exportFD);
    if(ret != 0)
    {
        for(int i = 0; i < ARRY_SIZE(pgpioFDs); i++)
        {
            int fd = *pgpioFDs[i];
            if(fd > 0)
                close(fd);
        }
    }
    return ret;
}


pthread_t createThread(void *(*routine) (void *), void *arg)
{
    pthread_attr_t pthread_attr;
    pthread_t pthread;
    pthread_attr_init(&pthread_attr);
    pthread_attr_setdetachstate(&pthread_attr, 1);
    pthread_create(&pthread, &pthread_attr, routine, arg);
    LogUtil::info("new thread %u.", (unsigned int)pthread);
    return pthread;
}


void stepX(int steps)
{
    if(coordX + steps > maxX)
    {
        LogUtil::warning("coordX(%d+%d) will be greater then maxX(%d), ignore.", coordX, steps, maxX);
        return;
    }
    pthread_mutex_lock(&goto_mutex);
    TEST_RESET_FD(xDirectFD);
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    for(int i=0; i < steps * pixelSize; i++)
    {
        usleep(stepDuration * 1000 / 2);
        SET_FD(xMotorFD);
        usleep(stepDuration * 1000 / 2);
        RESET_FD(xMotorFD);
    }
    coordX += steps;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_mutex_unlock(&goto_mutex);
}

void stepY(int steps)
{
    if(coordY + steps > maxY)
    {
        LogUtil::warning("coordY(%d+%d) will be greater then maxY(%d), ignore.", coordY, steps, maxY);
        return;
    }
    pthread_mutex_lock(&goto_mutex);
    TEST_RESET_FD(yDirectFD);
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    for(int i=0; i < steps * pixelSize; i++)
    {
        usleep(stepDuration * 1000 / 2);
        SET_FD(yMotorFD);
        usleep(stepDuration * 1000 / 2);
        RESET_FD(yMotorFD);
    }
    coordY += steps;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_mutex_unlock(&goto_mutex);
}

void reverseX(int steps)
{
    if(coordX - steps < 0)
    {
        LogUtil::warning("coordX(%d-%d) will be less then 0, ignore.", coordX, steps);
        return;
    }
    pthread_mutex_lock(&goto_mutex);
    TEST_SET_FD(xDirectFD);
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    for(int i=0; i < steps * pixelSize; i++)
    {
        usleep(stepDuration * 1000 / 2);
        SET_FD(xMotorFD);
        usleep(stepDuration * 1000 / 2);
        RESET_FD(xMotorFD);
    }
    coordX -= steps;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_mutex_unlock(&goto_mutex);
}

void reverseY(int steps)
{
    if(coordY - steps < 0)
    {
        LogUtil::warning("coordY(%d-%d) will be less then 0, ignore.", coordY, steps);
        return;
    }
    pthread_mutex_lock(&goto_mutex);
    TEST_SET_FD(yDirectFD);
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    for(int i=0; i < steps * pixelSize; i++)
    {
        usleep(stepDuration * 1000 / 2);
        SET_FD(yMotorFD);
        usleep(stepDuration * 1000 / 2);
        RESET_FD(yMotorFD);
    }
    coordY -= steps;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_mutex_unlock(&goto_mutex);
}


void _gotoCoord(int x, int y,bool allowBreak)
{
    if(x == coordX && y == coordY)
        return;
    if(x < 0 || y < 0 || x > maxX || y > maxY)
    {
        LogUtil::warning("coordinate(%d,%d) is not in range (0,0)-(%d,%d), ignore.",x, y, maxX, maxY);
        return;
    }
    pthread_mutex_lock(&goto_mutex);
    if(!allowLaserWhenMove)
        laserOff();
    int xadd, yadd;
    if(x < coordX)
    {
        xadd = -1;
        TEST_SET_FD(xDirectFD);
    }else{
        xadd = 1;
        TEST_RESET_FD(xDirectFD);
    }
    if(y < coordY)
    {
        yadd = -1;
        TEST_SET_FD(yDirectFD);
    }else{
        yadd = 1;
        TEST_RESET_FD(yDirectFD);
    }
//    printf("{(%d,%d) goto (%d,%d)}", coordX, coordY, x, y);
    int count = MAX(abs(x - coordX),abs(y - coordY));
    for(int i=0; i<count; i++)
    {
        if(machineExit && allowBreak)
        {
            LogUtil::info("_gotoCoord break for machine exit.");
            break;
        }
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        for(int j=0; j<pixelSize; j++)
        {
            usleep(stepDuration * 1000 / 2);
            if(coordX != x)
                SET_FD(xMotorFD);
            if(coordY != y)
                SET_FD(yMotorFD);
            usleep(stepDuration * 1000 / 2);
            if(coordX != x)
                RESET_FD(xMotorFD);
            if(coordY != y)
                RESET_FD(yMotorFD);
        }
        if(coordX != x)
            coordX += xadd;
        if(coordY != y)
            coordY += yadd;
//        printf("(%d,%d)", coordX, coordY);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    }
    pthread_mutex_unlock(&goto_mutex);
}

void gotoCoord(int x, int y)
{
    _gotoCoord(x, y, true);
}

int loadCoord()
{
    int ret = 0;
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    int* pcoords[] = {&coordX, &coordY, &pixelSize};
    if(NULL == (fp = fopen(COORD_FILE, "r")))
    {
        ret = -1;
        LogUtil::warning("open %s fail: %s.", COORD_FILE, strerror(errno));
        goto loadcoord_out;
    }
    for(int i=0; i<ARRY_SIZE(pcoords); i++)
    {
        if(-1 == (read = getline(&line, &len, fp)))
        {
            ret = -2;
            LogUtil::error("can't %s read line: %s.", COORD_FILE, strerror(errno));
            goto loadcoord_out;
        }
        line[read - 1] = 0;
        *pcoords[i] = atoi(line);
    }

loadcoord_out:
    if(ret != 0)
    {
        coordX = coordY = 0;
        pixelSize = 1;
    }
    maxX = X_SIZE / STEP_SIZE / pixelSize;
    maxY = Y_SIZE / STEP_SIZE / pixelSize;
    LogUtil::info("coordinate load, coordX:%d, coordY:%d:, pixelSize:%d, maxX:%d, maxY:%d."
           , coordX, coordY, pixelSize, maxX, maxY);
    if(fp)
        fclose(fp);
    if (line)
        free(line);
    return ret;
}


int saveCoord()
{
    int ret = 0;
    FILE * fp;
    char buf[100];
    if(NULL == (fp = fopen(COORD_FILE, "w")))
    {
        ret = -1;
        LogUtil::error("open %s fail: %s.", COORD_FILE, strerror(errno));
        goto savecoord_out;
    }
    sprintf(buf, "%d\n%d\n%d\n", coordX, coordY, pixelSize);
    if(0 == fwrite(buf, 1, strlen(buf) + 1, fp))
    {
        ret = -2;
        LogUtil::error("write %s fail: %s.", COORD_FILE, strerror(errno));
        goto savecoord_out;
    }
//    LogUtil::info("coordinate saved, coordX:%d, coordY:%d, pixelSize:%d.", coordX, coordY, pixelSize);

savecoord_out:
    if(fp)
        fclose(fp);
    return ret;
}

void pixelResize(int newsize)
{
    coordX = coordX * pixelSize / newsize;
    coordY = coordY * pixelSize / newsize;
    maxX = X_SIZE / STEP_SIZE / newsize;
    maxY = Y_SIZE / STEP_SIZE / newsize;
    pixelSize = newsize;
    LogUtil::info("resize pixel to %d.", newsize);
    saveCoord();
}

void sighandler(int signum)
{
    machineExit = true;
    LogUtil::info("receive a signal %d, reset the machine and exit!", signum);
    usleep(150000);
    machineReset();
    laserOff();
    exit(100);
}

void registerSignal()
{
    int signs[] = {SIGINT, SIGQUIT, SIGABRT, SIGTERM};
    for(int i=0; i<ARRY_SIZE(signs); i++)
    {
        if(SIG_ERR == signal(signs[i], sighandler))
            LogUtil::error("register signal %d fail: %s.", signs[i], strerror(errno));
        else
            LogUtil::info("register signal %d.", signs[i]);
    }
}

void machineReset()
{
    LogUtil::info("machine reset.");
    motorEnable();
    laserOff();
    coordX += maxX / 8;
    coordY += maxY / 4;
    _gotoCoord(0 ,0, false);
    motorDisable();
    saveCoord();
}

int machineInit()
{
    int ret =0;
    if(NULL == LogUtil::getDefaultLog())
        LogUtil::setDefaultLog(new LogUtil("/root/laser_carve.log"));
    pthread_mutex_init(&goto_mutex, NULL);
    loadCoord();
    laserFDStatue = enableFDStatue = xMotorFDStatue = xDirectFDStatue
            = yMotorFDStatue = yDirectFDStatue = false;
    if(0 != (ret = initFD()))
    {
        LogUtil::error("init fds fail!");
        return ret;
    }
    registerSignal();
//    machineReset();
    motorDisable();
    return ret;
}
