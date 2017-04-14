#ifndef MYDHT_H
#define MYDHT_H

#include <QWidget>
#include "qcwmeter.h"
#include "qcwthermometer.h"

class MyDHT: public QWidget
{
public:
    explicit MyDHT(QWidget *parent = 0);
    //MyDHT(int cnt,QWidget *parent = 0);

private:
    QcwThermometer* thermometer;
    QcwMeter *meter;
};

#endif // MYDHT_H
