#ifndef BITMAPMODE_HH
#define BITMAPMODE_HH
#include "basicmode.hh"
#include <QImage>

class BitmapMode : public BasicMode
{
public:
    inline int getWidth(){return mWidth;}
    inline int getHeight(){return mHeight;}
    inline QImage* getImageP(){return &mImage;}
protected:
    int mWidth;
    int mHeight;
    QImage mImage;
    BitmapMode(const char *path, Qt::ImageConversionFlags flag=Qt::DiffuseDither);
    void transImage(Qt::ImageConversionFlags flag=Qt::DiffuseDither);
    QRgb mBlackIndex;
    QRgb mSolvedIndex;

};

#endif // BITMAPMODE_HH
