#include "logtext.hh"

LogText::LogText(QWidget *parent) :
    QTextEdit(parent)
{
    setReadOnly(true);
    document()->setMaximumBlockCount(1000);
    if(NULL == LogUtil::getDefaultLog())
        LogUtil::setDefaultLog(new LogUtil("/root/laser_carve.log"));
    connect(LogUtil::getDefaultLog(), SIGNAL(handleLog(int,const char*)), this, SLOT(handleLog(int,const char*)));
}

void LogText::handleLog(int logLevel, const char *msg)
{

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
    QString color, tip;
    if(LOG_LEVEL_INFO == logLevel){
        color = QString("gray");
    }else if(LOG_LEVEL_WARNING == logLevel){
        color = QString("yellow");
    }else if(LOG_LEVEL_ERROR == logLevel){
        color = QString("red");
    }else if(LOG_LEVEL_DEBUG ==logLevel){
        color = QString("green");
    }else{
        color = QString("black");
    }
    if(LOG_LEVEL_INFO == logLevel){
        tip = QString("INFO");
    }else if(LOG_LEVEL_WARNING == logLevel){
        tip = QString("WARNING");
    }else if(LOG_LEVEL_ERROR == logLevel){
        tip = QString("ERROR");
    }else if(LOG_LEVEL_DEBUG == logLevel){
        tip = QString("DEBUG");
    }else{
        tip = QString("NOTE");
    }
    insertHtml(QString("<font color='") + color + "'>[" + tip + "] " + msg + "<br/></font>");
    ensureCursorVisible();
}
