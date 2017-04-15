#include "mydht.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QLabel>
#include <QDebug>

MyDHT::MyDHT(QWidget *parent) : QWidget(parent)
{
    thermometer=new QcwThermometer();
    thermometer->resize(56, 200);
    //thermometer->setMaximumHeight(200);
    thermometer->setMaximumWidth(80);
    thermometer->setMinimumWidth(80);
    //thermometer->setMinimumHeight(200);
    thermometer->setValue(10);
    thermometer->setMaxValue(50);
    thermometer->setMinValue(0);

    meter=new QcwMeter();
    meter->resize(200, 200);
    //meter->setMaximumWidth(200);
    //meter->setMaximumHeight(125);
    meter->setMinimumWidth(170);
    //meter->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    meter->setMaxValue(100);
    meter->setMinValue(0);
    meter->setValue(10);

    QSpacerItem* left = new QSpacerItem(75, 40, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSpacerItem* right = new QSpacerItem(75, 40, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QHBoxLayout * humiture_layout = new QHBoxLayout();

    humiture_layout->addSpacerItem(left);
    humiture_layout->addWidget(meter);
    humiture_layout->addWidget(thermometer);
    humiture_layout->addSpacerItem(right);
    humiture_layout->setSpacing(0);
    humiture_layout->setMargin(0);

    this->setLayout(humiture_layout);
}


