#include "ncmode.hh"
#include <QMessageBox>

#ifndef __USE_GNU
#define __USE_GNU
#endif
#include "math.h"
#define pi M_PIl

/*
 * NOTICE: machine_base 中使用左手坐标系， NCMode 中使用右手坐标系， 必须交换横纵坐标
*/
#define GOTO_COORD(X, Y) gotoCoord(Y, X);
#define COORD_NOT_IN_RANGE(X, Y) (X<0 || Y<0 || X>maxY || Y>maxX)


NCMode::NCMode(const char *file)
{
    char buf[40 * 1024];
    FILE* f = fopen(file, "r");
    if(NULL == f)
    {
        LogUtil::error("can't open %s: %s.", file, strerror(errno));
        return;
    }
    size_t count = fread(buf, 1, 40 * 1024, f);
    if(0 == count)
    {
        LogUtil::error("can't read %s (%s), or it's empty.", file, strerror(errno));
        initNC(QString(""));
    }else{
        buf[count] = 0;
        initNC(QString(buf));
    }
    fclose(f);
}


NCMode::NCMode(QString codes)
{
    initNC(codes);
}

NCMode* NCMode::createObj(const char *file)
{
    return new NCMode(file);
}


NCMode* NCMode::createObj(QString codes)
{
    return new NCMode(codes);
}

void NCMode::initNC(QString codes)
{
    mLines = codes.replace(QChar('\r'), QString("")).split(QChar('\n'));
    mIndex = 0;
    mPixelDuration = 0;
    mG = 0;
}


void NCMode::lineTo(float x, float y)
{
    double tx, ty;
    double step;
    double rate;
    x = (float)mmToCoord(x);
    y = (float)mmToCoord(y);
    if(COORD_X == x && COORD_Y == y)
    {
        LogUtil::warning("line to the current coordinate, ignore.");
        return;
    }
    if(COORD_NOT_IN_RANGE(x, y))
        throw "destination beyond the range";
    if(abs(x - COORD_X) > abs(y - COORD_Y))
    {
        rate = ((double)(y -COORD_Y)) / (x - COORD_X);
        step = x > COORD_X ? 1 : -1;
        ty = COORD_Y;
        while(x != COORD_X)
        {
            ty += step * rate;
            GOTO_COORD(COORD_X + step, DOUBLE_TO_INT(ty));
            if(mPixelDuration)
                usleep(1000 * mPixelDuration);
        }
    }else{
        rate = ((double)(x - COORD_X)) / (y - COORD_Y);
        step = y > COORD_Y ? 1 : -1;
        tx = COORD_X;
        while(y != COORD_Y)
        {
            checkPause();
            tx += step * rate;
            GOTO_COORD(DOUBLE_TO_INT(tx), COORD_Y + step);
            if(mPixelDuration)
                usleep(1000 * mPixelDuration);
        }
    }
}


void NCMode::arcTo(bool clockwise, float x, float y, float rx, float ry)
{
#define circle_tolerances_threshold 0.5
#define circle_side_length 0.4
    float x0 = coordToMM(COORD_X);
    float y0 = coordToMM(COORD_Y);
    double tolerances = sqrt(pow(x - rx, 2) + pow(y - ry, 2)) - sqrt(pow(x0 - rx, 2) + pow(y0 - ry, 2));
    if(tolerances > circle_tolerances_threshold || tolerances < 0 - circle_tolerances_threshold)
        throw "begin point and the end point are not in the same circle";
    double r = sqrt(pow(x - rx, 2) + pow(y - ry, 2));
    double beginRad = atan2(y0 - ry, x0 - rx);
    double endRad;
    if(x == x0 && y == y0)
    {
        if(clockwise)
            endRad = beginRad - 2 * pi;
        else
            endRad = beginRad + 2 * pi;
    }else{
        endRad = atan2(y - ry, x - rx);
        if(clockwise && endRad - beginRad > 0)
            endRad -= 2*pi;
        if(!clockwise && endRad - beginRad < 0)
            endRad += 2*pi;
    }
    double stepRad = 2 * asin(circle_side_length / 2 / r);
    double tRad = beginRad;
    while(tRad != endRad)
    {
        if(clockwise)
        {
            tRad -= stepRad;
            if(tRad < endRad)
                tRad = endRad;
        }else{
            tRad += stepRad;
            if(tRad > endRad)
                tRad = endRad;
        }
        if(tRad == endRad)
            lineTo(x, y);
        else
            lineTo(cos(tRad) * r + rx, sin(tRad) * r + ry);
    }
}


void NCMode::parseLine(QString& line)
{
    float x0 = coordToMM(COORD_X);
    float y0 = coordToMM(COORD_Y);
    float x = x0;
    float y = y0;
    float rx = NAN, ry = NAN, r = NAN;
    line  = line.trimmed().toUpper();
    if(line.length() == 0 || line.startsWith("#") || line.startsWith("//") || line.startsWith("%"))
        return;
    QStringList words = line.split(QChar(' '), QString::SkipEmptyParts);
    for(QStringList::iterator i=words.begin(); i<words.end(); i++)
    {
        QString word = *i;
        char key = word[0].toAscii();
        bool ok;
        float num = word.right(word.size() - 1).toFloat(&ok);
        if(!ok)
            throw "format error";
        if('G' == key)
        {
            if(num != 0 && num != 1 && num != 2 && num != 3)
            {
                throw "unsupported G code";
            }
            mG = num;
        }else if('M' == key){
            if(0 == num || 30 == num){

            }else if(3 == num || 4 == num){
                laserOn();
            }else if(5 == num){
                laserOff();
            }else{
                LogUtil::warning("unsupported M code (%s), ignore.", word.toAscii().constData());
            }
        }else if('X' == key){
            x = num;
        }else if('Y' == key){
            y = num;
        }else if('F' == key){
            /* full speed 50 , work 4.55 ~ 2.80 */
            mPixelDuration = DOUBLE_TO_INT(1000.0 / num * pixelSize * STEP_SIZE - stepDuration);
            if(mPixelDuration < 0)
                mPixelDuration = 0;
            LogUtil::info("set pixel delay duration to %d.", mPixelDuration);
        }else if('R' == key){
            r = num;
        }else if('I' == key){
            rx = num;
        }else if('J' == key){
            ry = num;
        }else if('N' == key || 'O' == key){

        }else {
            LogUtil::warning("unsupported code (%s), ignore.", word.toAscii().constData());
        }
    }

    if(0 == mG)
    {
        int _x = mmToCoord(x);
        int _y = mmToCoord(y);
        if(COORD_NOT_IN_RANGE(_x, _y))
            throw "destination beyond the range";
        GOTO_COORD(_x, _y);
    }else if(1 == mG){
        lineTo(x, y);
    }else if (2 == mG || 3 == mG) {
        if(!isnan(r) && (isnan(rx) || isnan(ry)))
        {
             if(x0 == x && y0 == y)
                 throw "no end point for the arc";
             double rP2 = pow(r, 2);
             double lP2D4 = (pow(x - x0, 2) + pow(y - y0, 2)) / 4;
             if(rP2 < lP2D4)
                 throw "the arc is unsolvable";
             else if(rP2 == lP2D4){
                 rx = (x + x0) / 2;
                 ry = (y + y0) / 2;
             }else{
                 double rate = sqrt(rP2 - lP2D4) / 2 / sqrt(lP2D4);
                 if((3 == mG && r > 0) || (2 == mG && r < 0))
                 {
                     rx = (x + x0) / 2 - rate * (y - y0);
                     ry = (y + y0) / 2 + rate * (x - x0);
                 }else{
                     rx = (x + x0) / 2 + rate * (y - y0);
                     ry = (y + y0) / 2 - rate * (x - x0);
                 }
             }
        }
        if(!isnan(rx) && !isnan(ry))
            arcTo(2 == mG, x, y, rx, ry);
//        else
//            throw "no enough parameter for the arc";
    }
}

void NCMode::run()
{
    bool _allowLaser = allowLaserWhenMove;
    allowLaserWhenMove = true;
    LogUtil("NCMode begin...");
    for(mIndex=0; mIndex < mLines.size(); mIndex ++)
    {
        LogUtil::debug("#%d;(%d,%d);(%f,%f);%s", mIndex + 1, COORD_X, COORD_Y
                       , coordToMM(COORD_X), coordToMM(COORD_Y), mLines[mIndex].toAscii().constData());
        checkPause();
        try
        {
            parseLine(mLines[mIndex]);
        }
        catch(const char *str)
        {
            LogUtil::error("runtime error(%s) in line %d:\n%s", str, mIndex + 1
                           , mLines[mIndex].toAscii().constData());
            laserOff();
            emit NCError(QString("error: ") + str + ", in line " + (mIndex + 1) + "\n" + mLines[mIndex]);
//            QMessageBox::critical(NULL, "NC code error", QString("error: ") + str
//                                  + ", in line " + (mIndex + 1) + "\n" + mLines[mIndex]);
            allowLaserWhenMove = _allowLaser;
            return;
        }
    }
    LogUtil::info("NCMode end.");
    allowLaserWhenMove = _allowLaser;
}
