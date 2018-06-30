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
/*
 * machine configurations
*/
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define ARRY_SIZE(a) ((int)(0 == sizeof(a) ? 0 : sizeof(a) / sizeof(a[0])))

#define SET_FD(fd) do{write(fd, "1", 2);fd##Statue = true;}while(0)
#define RESET_FD(fd) do{write(fd, "0", 2);fd##Statue = false;}while(0)
#define TEST_SET_FD(fd) do{if(!fd##Statue)SET_FD(fd);}while(0)
#define TEST_RESET_FD(fd) do{if(fd##Statue)RESET_FD(fd);}while(0)

/* 脉冲当量，每脉冲步进距离，单位mm */
#define STEP_SIZE 0.15
/* X轴有效行程，单位mm */
#define X_SIZE 45
/* Y轴有效行程，单位mm */
#define Y_SIZE 45
/* 当前坐标信息保存路径 */
#define COORD_FILE "/root/laser_coord"
/* 激光器使能IO，高电平有效 */
#define GPIO_LASER 76
/* 电机使能IO，低电平有效 */
#define GPIO_ENABLE 78
/* X轴电机PWM输出IO */
#define GPIO_X_MOTOR 123
/* X轴电机方向控制IO */
#define GPIO_X_DIRECTION 117
/* Y轴电机PWM输出IO */
#define GPIO_Y_MOTOR 122
/* Y轴电机方向控制IO */
#define GPIO_Y_DIRECTION 116
/* 像素点大小，单位STEP_SIZE，运动最小单位为像素点大小 */
#define DEFAULT_PIXEL_SIZE 1
/* 默认像素点延时，取决于激光功率和材料 */
#define DEFUALT_PIXEL_DURATION 24
/* 快速进给下的默认脉冲周期，单位ms */
#define DEFAULT_STEP_DURATION 3
/* 开始运动时将脉冲周期从CUSHION_MAX_DURATION，每次减小CUSHION_DURATION_RATE
 * ，直到当前的像素点延时，用于减小启动频率 */
#define DEFAULT_CUSHION_STATE false
#define CUSHION_MAX_DURATION 8
#define CUSHION_DURATION_RATE 2
/* 在无行程开关下，为保证回零时能准确回到原点
 * ，向两轴负方向多运动的距离，单位：像素点大小 */
#define X_INTERFERENCE 40
#define Y_INTERFERENCE 40

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
extern bool cushionState;

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
