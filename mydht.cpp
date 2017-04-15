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
    thermometer->setMaximumWidth(80);
    thermometer->setMinimumWidth(80);
    thermometer->setValue(10);
    thermometer->setMaxValue(50);
    thermometer->setMinValue(0);

    meter=new QcwMeter();
    meter->resize(200, 200);
    meter->setMinimumWidth(160);

    meter->setMaxValue(100);
    meter->setMinValue(0);
    meter->setValue(10);

    node_info = new QLabel();
    node_info->adjustSize();
    node_info->setMaximumHeight(20);
    node_info->setStyleSheet(QString::fromUtf8("font: 10pt \"Sans Serif\";"));

    QSpacerItem* left = new QSpacerItem(75, 40, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSpacerItem* right = new QSpacerItem(75, 40, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSpacerItem* mid = new QSpacerItem(5, 40, QSizePolicy::Fixed, QSizePolicy::Minimum);
    QHBoxLayout * humiture_layout = new QHBoxLayout();

    humiture_layout->addSpacerItem(left);
    humiture_layout->addWidget(meter);
    humiture_layout->addSpacerItem(mid);
    humiture_layout->addWidget(thermometer);
    humiture_layout->addSpacerItem(right);
    humiture_layout->setSpacing(0);
    humiture_layout->setMargin(0);


    QSpacerItem* left1 = new QSpacerItem(75, 40, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSpacerItem* right2 = new QSpacerItem(75, 40, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QHBoxLayout * label_layout = new QHBoxLayout();

    label_layout->addSpacerItem(left1);
    label_layout->addWidget(node_info);
    label_layout->addSpacerItem(right2);
    label_layout->setSpacing(0);
    label_layout->setMargin(0);

    QVBoxLayout *main_layout = new QVBoxLayout();
    main_layout->addLayout(humiture_layout);
    main_layout->addLayout(label_layout);
    main_layout->setSpacing(0);
    main_layout->setMargin(0);

    this->setLayout(main_layout);
}


void MyDHT::set_info(QString str)
{
    node_info->setText(str);
}

