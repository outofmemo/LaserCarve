#include "continuationmode.hh"

ContinuationMode::ContinuationMode(const char *path, Qt::ImageConversionFlags flag):BitmapMode(path, flag)
{
    mLines = NULL;
    vagueSize = 3;
}

ContinuationMode::~ContinuationMode()
{
    if(mLines)
        free(mLines);
}

ContinuationMode* ContinuationMode::createObj(const char *path, Qt::ImageConversionFlags flag)
{
    return new ContinuationMode(path, flag);
}

bool ContinuationMode::pointInRegion(int distance)
{
    int x, y;
    int x1, x2, y1, y2;

#define CHECK_PIXEL \
    if(mBlackIndex == mLines[y][x])\
    {\
        gotoCoord(x, y);\
        return true;\
    }

#define SEARCH_COLUMN \
    {\
        y1 = MAX(0, coordY - distance);\
        y2 = MIN(mHeight - 1, coordY + distance);\
        for(y=y1; y<=y2; y++)\
        {\
            CHECK_PIXEL\
        }\
    }

#define SEARCH_LINE \
    {\
        x1 = MAX(0, coordX - distance);\
        x2 = MIN(mWidth - 1, coordX + distance);\
        for(x=x1; x<=x2; x++)\
        {\
            CHECK_PIXEL\
        }\
    }

    x = coordX - distance;
    if(x >= 0)
        SEARCH_COLUMN;

    y = coordY - distance;
    if(y >= 0)
        SEARCH_LINE;

    x = coordX + distance;
    if(x <= mWidth - 1)
        SEARCH_COLUMN;

    y = coordY + distance;
    if(y <= mHeight - 1)
        SEARCH_LINE;

    return false;
}

bool ContinuationMode::nextEntry()
{
    int maxSquare = MAX(coordX, mWidth - 1 - coordX);
    maxSquare = MAX(maxSquare, coordY);
    maxSquare = MAX(maxSquare, mHeight -1 - coordY);
    for(int i=0; i<=maxSquare; i++)
    {
        if(pointInRegion(i))
            return true;
    }
    return false;
}

//void ContinuationMode::judgeTrend()
//{

//}

//bool ContinuationMode::paintBlock()
//{
//    return false;
//}

bool ContinuationMode::checkPixel(int x, int y)
{
    if(mBlackIndex == mLines[y][x])
    {
        gotoCoord(x, y);
        laserOn();
        mLines[y][x] = mSolvedIndex;
        usleep(1000 * pixelDuration);
        return true;
    }
    return false;
}

bool ContinuationMode::nextPixel()
{
    int x, y, i;
    if(checkPixel(coordX, coordY))
        return true;

#define BEGIN_LOOP \
    for(i=1; i<=vagueSize; i++)\
    {

#define END_LOOP \
    if(x >= 0 && y >= 0 && x < mWidth && y < mHeight)\
        if(checkPixel(coordX, y))\
            return true;\
    }

    BEGIN_LOOP;
    x = coordX;
    y = coordY - i;
    END_LOOP;

    BEGIN_LOOP;
    x = coordX - i;
    y = coordY;
    END_LOOP;

    BEGIN_LOOP;
    x = coordX + i;
    y = coordY;
    END_LOOP;

    BEGIN_LOOP;
    x = coordX;
    y = coordY + i;
    END_LOOP;

    BEGIN_LOOP;
    x = coordX - i;
    y = coordY - i;
    END_LOOP;

    BEGIN_LOOP;
    x = coordX + i;
    y = coordY - i;
    END_LOOP;

    BEGIN_LOOP;
    x = coordX - i;
    y = coordY + i;
    END_LOOP;

    BEGIN_LOOP;
    x = coordX + i;
    y = coordY + i;
    END_LOOP;

    return false;
}


void ContinuationMode::touch()
{
    while(nextPixel())
        checkPause();
}

void ContinuationMode::run()
{
    if(mLines)
        free(mLines);
    mLines = (uchar**)malloc(mHeight * sizeof(uchar*));
    for(unsigned short i=0; i<mHeight; i++)
    {
        mLines[i] = (uchar*)mImage.constScanLine(i);
    }
    LogUtil::info("ContinuationMode start...");
    while(nextEntry())
    {
        touch();
    }
    LogUtil::info("ContinuationMode finished.");
}
