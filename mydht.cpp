#include "mydht.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QLabel>
#include <QDebug>

MyDHT::MyDHT(QWidget *parent) : QWidget(parent)
{
    thermometer=new QcwThermometer();
    thermometer->resize(100, 300);
    thermometer->setValue(10);
    thermometer->setMaxValue(80);
    thermometer->setMinValue(0);

    meter=new QcwMeter();
    meter->resize(20, 20);
    meter->setMaxValue(50);
    meter->setMinValue(0);
    meter->setValue(10);

    QHBoxLayout * humiture_layout = new QHBoxLayout();
    humiture_layout->addWidget(meter);
    humiture_layout->addWidget(thermometer);
    humiture_layout->setSpacing(0);
    humiture_layout->setMargin(0);

    this->setLayout(humiture_layout);

}


