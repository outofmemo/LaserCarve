#ifndef LOGTEXT_HH
#define LOGTEXT_HH

#include <QTextEdit>
#include "logutil.hh"

class LogText : public QTextEdit
{
    Q_OBJECT
public:
    explicit LogText(QWidget *parent = 0);

signals:

public slots:
    void handleLog(int logLevel, const char* msg);

};

#endif // LOGTEXT_HH
