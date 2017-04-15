#ifndef MYDHT_H
#define MYDHT_H

#include <QWidget>
#include "qcwmeter.h"
#include "qcwthermometer.h"
#include <QLabel>

class MyDHT: public QWidget
{
public:
    explicit MyDHT(QWidget *parent = 0);
    //MyDHT(QString info="",QWidget *parent = 0);
    void set_info(QString str);
    int dht_count();

public:
    QcwThermometer* thermometer;
    QcwMeter *meter;
    QLabel * node_info;
    static int cnt;

};

#endif // MYDHT_H
