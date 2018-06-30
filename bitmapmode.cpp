#include "bitmapmode.hh"
#include <QVector>
#include <QColor>

BitmapMode::BitmapMode(const char* path, Qt::ImageConversionFlags flag)
{
    mImage = QImage(path);
    transImage(flag);
}

void BitmapMode::transImage(Qt::ImageConversionFlags flag)
{
    int width = mImage.width();
    int height = mImage.height();
    LogUtil::info("image loaded, width:%d,height:%d.", width, height);
    if(width > maxX || height > maxY)
    {
        if(width/maxX > height/maxY)
            mImage = mImage.scaledToWidth(maxX, Qt::SmoothTransformation);
        else
            mImage = mImage.scaledToHeight(maxY, Qt::SmoothTransformation);
        LogUtil::info("resize image to width:%d,height:%d.", mImage.width(), mImage.height());
    }
    mWidth = mImage.width();
    mHeight = mImage.height();
    mImage = mImage.convertToFormat(QImage::Format_Mono, flag);
    mImage = mImage.convertToFormat(QImage::Format_Indexed8);
    mImage.save("/home/echo/Desktop/test_transformed.bmp");
    mBlackIndex = 0xfe;
    QVector<QRgb> colorTable = mImage.colorTable();
    for(int i=0; i<colorTable.count(); i++)
    {
        if(0 == QColor(colorTable.at(i)).red())
        {
            mBlackIndex = i;
            break;
        }
    }
    if(0xfe == mBlackIndex)
    {
        LogUtil::warning("Can't find the index of color black.");
    }
    mSolvedIndex = colorTable.count();
    mImage.setColor(mBlackIndex, 0xffcccccc);
    mImage.setColor(mSolvedIndex, 0xff000000);
    //LogUtil::info("black index:%d, solved color index:%d.", mBlackIndex, mSolvedIndex);
}
