#ifndef NCMODE_HH
#define NCMODE_HH
#include "basicmode.hh"
#include <QString>
#include <QStringList>

#define COORD_X coordY
#define COORD_Y coordX
#define DOUBLE_TO_INT(d) ((int)((d) + 0.5))
class NCMode : public BasicMode
{

Q_OBJECT

public:
    //explicit NCMode(QObject *parent = 0);
    static NCMode* createObj(QString codes);
    static NCMode* createObj(const char* file);
    void run();
    inline int getIndex(){return mIndex;}
    inline QStringList getLines(){return mLines;}
    static inline int mmToCoord(double a){return DOUBLE_TO_INT(a / STEP_SIZE / pixelSize);}
    static inline float coordToMM(int a){return a * STEP_SIZE * pixelSize;}

signals:
    void NCError(QString error);

protected:
    NCMode(QString codes);
    NCMode(const char* file);
    void lineTo(float x, float y);
    void arcTo(bool clockwise, float x, float y, float rx, float ry);
    int mPixelDuration;
    void parseLine(QString& line);
    void initNC(QString codes);
    QStringList mLines;
    int mIndex;
    int mG;

};

#endif // NCMODE_HH
