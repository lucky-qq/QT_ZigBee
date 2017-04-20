#ifndef MYDHT_H
#define MYDHT_H

#include <QWidget>
#include "qcwmeter.h"
#include "qcwthermometer.h"
#include <QLabel>
#include <QMouseEvent>

class MyDHT: public QWidget
{
public:
    explicit MyDHT(QWidget *parent = 0);
    //MyDHT(QString info="",QWidget *parent = 0);
    void set_info(QString str);
    QString get_info();
    int dht_count();
    //重写mousePressEvent事件
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

public slots:


public:
    QcwThermometer* thermometer;
    QcwMeter *meter;
    QLabel * node_info;
    static int cnt;
    MyDHT *tmp;

};

#endif // MYDHT_H
