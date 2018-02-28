#ifndef CONTINUATIONMODE_HH
#define CONTINUATIONMODE_HH
#include "bitmapmode.hh"

class ContinuationMode : public BitmapMode
{
public:
    void run();
    ~ContinuationMode();
    static ContinuationMode* createObj(const char *path, Qt::ImageConversionFlags flag=Qt::DiffuseDither);
protected:
    ContinuationMode(const char *path, Qt::ImageConversionFlags flag=Qt::DiffuseDither);
private:
    int vagueSize;
    uchar** mLines;
    bool nextEntry();
    bool pointInRegion(int distance);
    void touch();
    bool checkPixel(int x, int y);
    bool nextPixel();
//    bool paintBlock();
//    void judgeTrend();
//    enum TrendType{
//        TREND_LEFT,
//        TREND_UP,
//        TREND_RIGHT,
//        TREND_DOWN,
//        TREND_LEFT_UP,
//        TREND_RIGHT_UP,
//        TREND_LEFT_DOWN,
//        TREND_RIGHT_DOWN
//    };

//    enum TrendType trend;
//    int trendEndX;
//    int trendEndY;
};

#endif // CONTINUATIONMODE_HH
