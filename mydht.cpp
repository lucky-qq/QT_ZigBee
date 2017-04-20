#include "mydht.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QLabel>
#include <QDebug>

int MyDHT::cnt=0;

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

    QHBoxLayout * humiture_layout = new QHBoxLayout();

    humiture_layout->addSpacerItem(left);
    humiture_layout->addWidget(meter);
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
    cnt++;
}



void MyDHT::set_info(QString str)
{
    node_info->setText(str);
}

int MyDHT::dht_count()
{
    return cnt;
}

//重写mousePressEvent事件,检测事件类型是不是点击了鼠标左键
void MyDHT::mousePressEvent(QMouseEvent *event) {
    //如果单击了就触发clicked信号
    if (event->button() == Qt::LeftButton) {
        //触发clicked信号
        //emit clicked();
        int humiture_w = this->meter->width();
        int humiture_h = this->meter->height();
        int temprature_w = this->thermometer->width();
        int temprature_h = this->thermometer->height();
        double humiture_val = this->meter->value();
        double temprature_val = this->thermometer->value();

        tmp = new MyDHT();

        tmp->meter->resize(humiture_w*2,humiture_h*2);
        tmp->meter->setValue(humiture_val);
        tmp->meter->setMinimumWidth(160*2);
        tmp->meter->setMaxValue(100);
        tmp->meter->setMinValue(0);

        tmp->thermometer->setMaximumWidth(80*2);
        tmp->thermometer->setMinimumWidth(80*2);
        tmp->thermometer->setValue(temprature_val);
        tmp->thermometer->setMaxValue(50);
        tmp->thermometer->setMinValue(0);
        tmp->thermometer->resize(temprature_w*2,temprature_h*2);

        //tmp->node_info->adjustSize();
        tmp->node_info->setMaximumHeight(20*2);
        tmp->node_info->setStyleSheet(QString::fromUtf8("font: 20pt \"Sans Serif\";"));
        tmp->set_info(this->get_info());

        tmp->setGeometry(312,100,400,400);
        tmp->show();

        //tmp->resize(width*2,height*2);
        //this->resize(width*2,height*2);
        //qDebug()<<"press";

    }
    //将该事件传给父类处理
    QWidget::mousePressEvent(event);
}

QString MyDHT::get_info()
{
    return node_info->text();
}

void MyDHT::mouseReleaseEvent(QMouseEvent *event)
{

    //int width = this->width();
    //int height = this->height();
    //this->resize(width/2,height/2);
    tmp->close();
    //qDebug()<<"release";

    //将该事件传给父类处理
    QWidget::mousePressEvent(event);
}


