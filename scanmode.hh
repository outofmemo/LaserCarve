#ifndef SCANMODE_HH
#define SCANMODE_HH
#include "bitmapmode.hh"

class ScanMode : public BitmapMode
{
public:
    ~ScanMode();
    void run();
    static ScanMode* createObj(const char *path, Qt::ImageConversionFlags flag=Qt::DiffuseDither);
protected:
    ScanMode(const char *path, Qt::ImageConversionFlags flag=Qt::DiffuseDither);
private:
    void getBoundary();
    void scanLine(int line);
    unsigned short* mBoundaryTable;
    uchar** mLines;


};


#endif // SCANMODE_HH
