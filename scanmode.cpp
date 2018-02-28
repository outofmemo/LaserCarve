#include "scanmode.hh"
#include "malloc.h"

ScanMode::ScanMode(const char *path, Qt::ImageConversionFlags flag):BitmapMode(path, flag)
{
    mBoundaryTable = NULL;
    mLines = NULL;
}

ScanMode* ScanMode::createObj(const char *path, Qt::ImageConversionFlags flag)
{
    return new ScanMode(path, flag);
}

ScanMode::~ScanMode()
{
    if(mBoundaryTable)
        free(mBoundaryTable);
    if(mLines)
        free(mLines);
}

void ScanMode::getBoundary()
{
    if(mBoundaryTable)
        free(mBoundaryTable);
    if(mLines)
        free(mLines);
    mBoundaryTable = (unsigned short*)malloc(mHeight * 2 * sizeof(unsigned short));
    mLines = (uchar**)malloc(mHeight * sizeof(uchar*));
    for(int i=0; i<mHeight; i++)
    {
        const uchar* line = mImage.constScanLine(i);
        mLines[i] = (uchar*)line;
        mBoundaryTable[i] = mWidth;
        for(int j=0; j<mWidth; j++)
        {
            if(line[j] == mBlackIndex)
            {
                mBoundaryTable[i] = j;
                break;
            }
        }
        if(mWidth == mBoundaryTable[i])
            continue;
        for(int k=mWidth-1; k>=0; k--)
        {
            if(line[k] == mBlackIndex)
            {
                mBoundaryTable[mHeight + i] = k;
                break;
            }
        }
    }
}

void ScanMode::scanLine(int line)
{
    if(mWidth == mBoundaryTable[line])
        return;
    printf("#%d ", line);
    gotoCoord(mBoundaryTable[line], line);
    for(int j=mBoundaryTable[line]; j<=mBoundaryTable[mHeight+line]; j++)
    {
//            printf("(%d,%d)", coordX, coordY);
        checkPause();
        if(mBlackIndex == mLines[line][j])
        {
            laserOn();
            mLines[line][j] = mSolvedIndex;
            usleep(1000 * pixelDuration);
        }
        else
            laserOff();
        stepX();
    }
}

void ScanMode::run()
{
    getBoundary();
    LogUtil::info("ScanMode start...");
    for(int i=0; i<mHeight; i++)
    {
        scanLine(i);
    }
    LogUtil::info("ScanMode finished.");
//    mImage.save("/home/echo/Desktop/test_modified.bmp");
}
