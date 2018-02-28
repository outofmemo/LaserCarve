#ifndef LOGUTIL_HH
#define LOGUTIL_HH

#include <QObject>
#include <stdio.h>
#include <stdarg.h>
#include <QString>

#define LOG_LEVEL_INFO 1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_ERROR 3
#define LOG_LEVEL_DEBUG 4

class LogUtil : public QObject
{
    Q_OBJECT
public:
    ~LogUtil();
    explicit LogUtil(QObject *parent = 0);
    explicit LogUtil(const char* file, QObject *parent = 0);

    static void info(const char* format, ...);
    static void error(const char* format, ...);
    static void warning(const char* format, ...);
    static void debug(const char* format, ...);

#define LOG_INFO LogUtil::getDefaultLog()->i_f
#define LOG_WARNING LogUtil::getDefaultLog()->w_f
#define LOG_ERROR LogUtil::getDefaultLog()->e_f
#define LOG_DEBUG LogUtil::getDefaultLog()->d_f

    static LogUtil* getDefaultLog(){return sDefaultLog;}
    static void setDefaultLog(LogUtil* log){sDefaultLog = log;}

    void i_f(const char* format, ...);
    void e_f(const char* format, ...);
    void w_f(const char* format, ...);
    void d_f(const char* format, ...);

    void i(const char* format, va_list ap);
    void e(const char* format, va_list ap);
    void w(const char* format, va_list ap);
    void d(const char* format, va_list ap);

    void i(const char* msg);
    void e(const char* msg);
    void w(const char* msg);
    void d(const char* msg);

private:
    static LogUtil* sDefaultLog;
    FILE* mLogFile;
    bool isDebug;

signals:
    void handleLog(int log_level, const char* msg);

public slots:

};

#endif // LOGUTIL_HH
