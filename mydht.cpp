#include "mydht.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QLabel>
#include <QDebug>

int MyDHT::cnt=0;
QMutex mutex;
MyDHT::MyDHT(QWidget *parent) : QWidget(parent)
{
    thermometer=new QcwThermometer(this);
    thermometer->resize(56, 200);
    thermometer->setMaximumWidth(80);
    thermometer->setMinimumWidth(80);
    thermometer->setValue(10);
    thermometer->setMaxValue(50);
    thermometer->setMinValue(0);

    meter=new QcwMeter(this);
    meter->resize(200, 200);
    meter->setMinimumWidth(160);

    meter->setMaxValue(100);
    meter->setMinValue(0);
    meter->setValue(10);
    tmp = NULL;

    node_info = new QLabel(this);
    node_info->adjustSize();
    node_info->setMaximumHeight(20);
    node_info->setStyleSheet(QString::fromUtf8("font: 10pt \"Sans Serif\";"));

    tmp = NULL;
    blink_flag = false;
    blinkDHT_cnt = 0;
    dht_timer = new QTimer();
    connect(dht_timer,&QTimer::timeout,this,&MyDHT::blinkDHT);



    QSpacerItem* left = new QSpacerItem(75, 40, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSpacerItem* right = new QSpacerItem(75, 40, QSizePolicy::Expanding, QSizePolicy::Minimum);

    QHBoxLayout * humiture_layout = new QHBoxLayout();

    humiture_layout->addSpacerItem(left);
    humiture_layout->addWidget(meter);
    humiture_layout->addWidget(thermometer);
    humiture_layout->addSpacerItem(right);
    humiture_layout->setSpacing(0);
    humiture_layout->setMargin(0);

    //printf("the value of &bird is::0x%0X\n",blink_timer);

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

    //mutex.lock();
    //如果单击了就触发clicked信号
    //dht_timer->stop();
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
        Q_CHECK_PTR(tmp);

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
    //mutex.unlock();
    //将该事件传给父类处理
    QWidget::mousePressEvent(event);

}

QString MyDHT::get_info()
{
    return node_info->text();
}

void MyDHT::mouseReleaseEvent(QMouseEvent *event)
{
    //mutex.lock();
    //int width = this->width();
    //int height = this->height();
    //this->resize(width/2,height/2);
    if(tmp != NULL)
    {
        tmp->close();
        delete tmp;
        tmp = NULL;
    }
    //qDebug()<<"release";
    //mutex.unlock();
    //将该事件传给父类处理
    QWidget::mousePressEvent(event);
    //dht_timer->start(500);

}

void MyDHT::blinkDHT()
{
    qDebug()<<"blink begin";
    if(blinkDHT_cnt % 2 == 0)
    {
        meter->setBackground(Qt::white);
        thermometer->setBackground(Qt::white);
    }
    else if (blinkDHT_cnt % 2 == 1)
    {
        meter->setBackground(Qt::black);
        thermometer->setBackground(Qt::blue);
    }

    blinkDHT_cnt++;
    qDebug()<<"blink end";
}


void MyDHT::setBlink(bool cmd)
{
    if(cmd == true && blink_flag == false)
    {
        dht_timer->start(500);
        qDebug()<<"dht_timer begin";
        blink_flag = true;
    }else if(blink_flag == true && dht_timer != NULL && cmd == false)
    {
        dht_timer->stop();
        blink_flag = false;
    }
}


