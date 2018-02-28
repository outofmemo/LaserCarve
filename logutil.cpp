#include "logutil.hh"
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define GENERAL_BUF_SIZE 300
#define GENERAL_BUF_COUNT 10
int buf_index = 0;
char general_buf[GENERAL_BUF_COUNT * GENERAL_BUF_SIZE];

#define GET_BUF() (general_buf + (buf_index++ % GENERAL_BUF_COUNT) * GENERAL_BUF_SIZE)

LogUtil* LogUtil::sDefaultLog = NULL;

LogUtil::~LogUtil()
{
    if(mLogFile)
        fclose(mLogFile);
}

LogUtil::LogUtil(QObject *parent) :
    QObject(parent)
{
    mLogFile = NULL;
    isDebug = true;
}

LogUtil::LogUtil(const char* file, QObject *parent) :
    QObject(parent)
{
    isDebug = true;
    mLogFile = fopen(file, "w+");
    if(NULL == mLogFile)
        e_f("open log file(%s) fail: %s.", file, strerror(errno));
}

void LogUtil::i(const char *msg)
{
    printf("[INFO] %s\n", msg);
    emit handleLog(LOG_LEVEL_INFO, msg);
    if(mLogFile){
        fwrite("[info] ", 1, 7, mLogFile);
        fwrite(msg, 1, strlen(msg), mLogFile);
        fwrite("\n", 1, 1, mLogFile);
    }
}

void LogUtil::w(const char *msg)
{
    printf("[WARNING] %s\n", msg);
    emit handleLog(LOG_LEVEL_WARNING, msg);
    if(mLogFile){
        fwrite("[WARNING] ", 1, 10, mLogFile);
        fwrite(msg, 1, strlen(msg), mLogFile);
        fwrite("\n", 1, 1, mLogFile);
    }
}

void LogUtil::e(const char *msg)
{
    fprintf(stderr, "[ERROR] %s\n", msg);
    emit handleLog(LOG_LEVEL_ERROR, msg);
    if(mLogFile){
        fwrite("[ERROR] ", 1, 8, mLogFile);
        fwrite(msg, 1, strlen(msg), mLogFile);
        fwrite("\n", 1, 1, mLogFile);
    }
}

void LogUtil::d(const char *msg)
{
    printf("[DEBUG] %s\n", msg);
    if(isDebug){
        emit handleLog(LOG_LEVEL_DEBUG, msg);
        if(mLogFile){
            fwrite("[DEBUG] ", 1, 8, mLogFile);
            fwrite(msg, 1, strlen(msg), mLogFile);
            fwrite("\n", 1, 1, mLogFile);
        }
    }
}


void LogUtil::d(const char *format, va_list ap)
{
    char* buf = GET_BUF();
    vsnprintf(buf, GENERAL_BUF_SIZE, format, ap);
    d(buf);
}


void LogUtil::i(const char *format, va_list ap)
{
    char* buf = GET_BUF();
    vsnprintf(buf, GENERAL_BUF_SIZE, format, ap);
    i(buf);
}

void LogUtil::w(const char *format, va_list ap)
{
    char* buf = GET_BUF();
    vsnprintf(buf, GENERAL_BUF_SIZE, format, ap);
    w(buf);
}

void LogUtil::e(const char *format, va_list ap)
{
    char* buf = GET_BUF();
    vsnprintf(buf, GENERAL_BUF_SIZE, format, ap);
    e(buf);
}

void LogUtil::i_f(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    i(format, ap);
    va_end(ap);
}

void LogUtil::w_f(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    w(format, ap);
    va_end(ap);
}

void LogUtil::e_f(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    e(format, ap);
    va_end(ap);
}


void LogUtil::d_f(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    d(format, ap);
    va_end(ap);
}

void LogUtil::info(const char *format, ...)
{
    if(sDefaultLog){
        va_list ap;
        va_start(ap, format);
        sDefaultLog->i(format, ap);
        va_end(ap);
    }
}

void LogUtil::warning(const char *format, ...)
{
    if(sDefaultLog){
        va_list ap;
        va_start(ap, format);
        sDefaultLog->w(format, ap);
        va_end(ap);
    }
}

void LogUtil::error(const char *format, ...)
{
    if(sDefaultLog){
        va_list ap;
        va_start(ap, format);
        sDefaultLog->e(format, ap);
        va_end(ap);
    }
}


void LogUtil::debug(const char *format, ...)
{
    if(sDefaultLog){
        va_list ap;
        va_start(ap, format);
        sDefaultLog->d(format, ap);
        va_end(ap);
    }
}
