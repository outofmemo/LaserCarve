#ifndef MACHINE_BASE_HH
#define MACHINE_BASE_HH

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "logutil.hh"

#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define ARRY_SIZE(a) ((int)(0 == sizeof(a) ? 0 : sizeof(a) / sizeof(a[0])))

#define SET_FD(fd) do{write(fd, "1", 2);fd##Statue = true;}while(0)
#define RESET_FD(fd) do{write(fd, "0", 2);fd##Statue = false;}while(0)
#define TEST_SET_FD(fd) do{if(!fd##Statue)SET_FD(fd);}while(0)
#define TEST_RESET_FD(fd) do{if(fd##Statue)RESET_FD(fd);}while(0)

#define STEP_SIZE 0.15
#define X_SIZE 45
#define Y_SIZE 45
#define COORD_FILE "/root/laser_coord"
#define GPIO_LASER 76
#define GPIO_ENABLE 78
#define GPIO_X_MOTOR 123
#define GPIO_X_DIRECTION 117
#define GPIO_Y_MOTOR 122
#define GPIO_Y_DIRECTION 116

extern int laserFD, enableFD, xMotorFD, xDirectFD, yMotorFD, yDirectFD;
extern bool laserFDStatue, enableFDStatue, xMotorFDStatue, xDirectFDStatue, yMotorFDStatue, yDirectFDStatue;

//dark green 30; dark blue 35; flat light green 75; white 80
extern int pixelDuration; //delay pixelDuration(ms) for each pixel
extern int stepDuration; //fast step speed: 1000 / stepDuration * STEP_SIZE mm/s
extern int pixelSize; //pixelSize * STEP_SIZE mm

extern int coordX, coordY;
extern int maxX, maxY;
extern bool machineExit;
extern bool allowLaserWhenMove;

int loadCoord();
int saveCoord();

void machineReset();
int machineInit();

inline void laserOn(){TEST_SET_FD(laserFD);}
inline void laserOff(){TEST_RESET_FD(laserFD);}

inline void motorEnable(){TEST_RESET_FD(enableFD);}
inline void motorDisable(){TEST_SET_FD(enableFD);}

void stepX(int steps=1);
void stepY(int steps=1);
void reverseX(int steps=1);
void reverseY(int steps=1);
void gotoCoord(int x, int y);
void pixelResize(int newsize);

int initFD();
pthread_t createThread(void *(*routine) (void *), void *arg);


#endif // MACHINE_BASE_HH
