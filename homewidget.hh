#ifndef HOMEWIDGET_HH
#define HOMEWIDGET_HH

#include <QWidget>
#include <QPushButton>
#include <QLabel>

class HomeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HomeWidget(QWidget *parent = 0);
    virtual void timerEvent( QTimerEvent *event);

signals:

public slots:
    void moveUp(bool checked = false);
    void moveDown(bool checked = false);
    void moveLeft(bool checked = false);
    void moveRight(bool checked = false);
    void returnZero(bool checked = false);
    void setZero(bool checked = false);
    void turnLaser(bool checked = false);
    void openFile(bool checked = false);

protected:
    void paintEvent(QPaintEvent* event);

private:
    QPushButton* laserBtn;
    QLabel* coordLable;
    int disableTime;
    void disableDelay();

};

#endif // HOMEWIDGET_HH
