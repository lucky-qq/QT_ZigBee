#include "mythread.h"
#include <QDateTime>
#include <QTimer>
#include <QFileInfo>
#include <QSqlDatabase>

MyThread::MyThread(QObject *parent) : QObject(parent)
{
    //记录读取状态相关变量的初始化
    cnt_read = 0;
    cnt_need = 0;
    data_need = 0;
    cnt = 0;

    isStop = false;//默认是false--不退出子线程处理函数
    serial = new QSerialPort(this);
    timer = NULL;
}

//将QByteArray类型的数据转换为int类型
int bytesToInt(QByteArray bytes) {
    int addr = bytes[0] & 0x000000FF;
    addr |= ((bytes[1] << 8) & 0x0000FF00);
    addr |= ((bytes[2] << 16) & 0x00FF0000);
    addr |= ((bytes[3] << 24) & 0xFF000000);
    return addr;
}
unsigned int calc_crc16 (unsigned char *snd, unsigned char num)
{
    unsigned char i, j;
    unsigned int c,crc=0xFFFF;
    for(i = 0; i < num; i ++)
    {
        c = snd[i] & 0x00FF;
        crc ^= c;
        for(j = 0;j < 8; j ++)
        {
            if (crc & 0x0001)
            {
                crc>>=1;
                crc^=0xA001;
            }
            else
            {
                crc>>=1;
            }
        }
    }
    return(crc);
}
void MyThread::sleep(int msec)
{
    QDateTime last = QDateTime::currentDateTime();
    QDateTime now;
    while (1)
    {
        now = QDateTime::currentDateTime();
        if (last.msecsTo(now) >= msec)
        {
            break;
        }
    }
}

void MyThread::initSerial(QSerialPortInfo info)
{


    photo_state = BEGIN;//默认状态是开始读取图片（暂时没用到）
    serial_state = READ_HDR;//默认状态是读取包头的状态

    node_id = 0;
    node_type = 0;

    seqnb = 0;
    FAIL_FLAG = false;//默认没有失败
    pkt_cnt = 0;



    if(serial->isOpen())//先关闭
        serial->close();
    serial->setPort(info);//设置串口号--就是从下拉框选择的串口号
    serial->open(QIODevice::ReadWrite);         //读写打开
    serial->setBaudRate(QSerialPort::Baud115200);  //波特率
    serial->setDataBits(QSerialPort::Data8); //数据位
    serial->setParity(QSerialPort::NoParity);    //无奇偶校验
    serial->setStopBits(QSerialPort::OneStop);   //停止位
    serial->setFlowControl(QSerialPort::NoFlowControl);  //无控制
    timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,&MyThread::readSerial);
    timer->start(10);

    //readSerial();//实际意义的子线程处理函数
    qDebug() << "child thread 1:========================"<< QThread::currentThread() ;

}

void MyThread::initUart485(QSerialPortInfo info)
{

    if(serial->isOpen())//先关闭
        serial->close();
    serial->setPort(info);//设置串口号--就是从下拉框选择的串口号
    serial->open(QIODevice::ReadWrite);         //读写打开
    serial->setBaudRate(QSerialPort::Baud9600);  //波特率
    serial->setDataBits(QSerialPort::Data8); //数据位
    serial->setParity(QSerialPort::NoParity);    //无奇偶校验
    serial->setStopBits(QSerialPort::OneStop);   //停止位
    serial->setFlowControl(QSerialPort::NoFlowControl);  //无控制
    serial_state = READ_PH;
    this->ec = 0;
    this->light = 0;
    this->ph_val = 0.0f;

    today = QDateTime::currentDateTime();
    current_date = today;

    database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName("husky02.db");
    database.setUserName("root");
    database.setPassword("123456");

    //sleep(5000);
    updateShowPH("ph");
    timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,&MyThread::beginRead);
    timer->start(1000*10);//子线程开启5s以后再去读传感器数据，否则串口资源会吃紧，导致不能正确读取数据
    //updateTablePH("ph");
    //test("ph");
    qDebug() << "child thread 2:========================"<< QThread::currentThread() ;

}

void MyThread::resolveDateChangePH(int date_index)
{
    current_date = today.addDays(-30+date_index);
    //if(current_date != today)
    {
        QString str = current_date.toString("yyyy-MM-dd");

        QString select_all_PH = QString("select * from %1 where date(date)>=date('%2') and date(date)<=date('%3') limit 0,96").arg(QString("ph")).arg(str).arg(str);
        QString create_PH = QString("create table IF NOT EXISTS %1 (id int primary key, date timestamp not null default (datetime('now','localtime')),ph_var real)").arg(QString("ph"));


        if(!database.open())
        {
            qDebug()<<database.lastError();
            qFatal("failed to connect.") ;
        }
        else
        {
            QSqlQuery sql_query;
            qDebug()<<"opend-----";
            sql_query.prepare(create_PH);
            if(!sql_query.exec())
            {
                qDebug()<<sql_query.lastError()<<"err 01 ";
            }
            else
            {
                qDebug()<<"table created!";

            }

            //查询所有数据
            sql_query.prepare(select_all_PH);
            if(!sql_query.exec())
            {
                qDebug()<<sql_query.lastError()<<"err 03 ";;
            }
            else
            {
                qDebug()<<"table select!";

                 QMap<QDateTime,qreal> tmp_map;
                if(tmp_map.count() > 0)
                    tmp_map.clear();
                QDateTime date;
                QString str;
                qreal var;
                int id;
                while(sql_query.next())
                {
                    id = sql_query.value(0).toInt();
                    date = sql_query.value(1).toDateTime();
                    str = date.toLocalTime().toString();
                    var = sql_query.value(2).toDouble();
                    tmp_map.insert(date,var);
                    //qDebug()<<QString("id:%1    date:%2    var:%3").arg(id).arg(str).arg(var);
                    //qDebug()<<QString("date:%1    var:%2").arg(str).arg(var);
                }

                emit DynamicShow(tmp_map);
                qDebug()<<"emit ..............................";
            }
        }
        database.close();
    }
}

void MyThread::beginRead()
{
    //timer->stop();//关闭该定时器

    get_PH_val(PH_DEV_ADDR);
    serial_state = READ_PH;
    sleep(2000);//每读取一个传感器后就延时2s，以便保持串口的同步操作

    get_light_val(LIGHT_ADDR);
    serial_state = READ_LIGHT;
    sleep(2000);

    get_conduct_val(CONDUCT_ADDR);
    serial_state = READ_CONDUCT;
    sleep(2000);

    QDateTime  tmpDate = QDateTime::currentDateTime();

    ph_map.insert(tmpDate,ph_val);
    updateTablePH("ph");
    if(ph_map.count() == PLOT_NUM)
    {
        ph_map.clear();
        updateOneDay("ph");
        return ;
    }
    if(current_date == today)
        emit DynamicShow(this->ph_map);
}

void MyThread::updateOneDay(QString str)
{
    QDateTime current_date = QDateTime::currentDateTime();
    QString firstday = current_date.addDays(-30).toString("yyyy-MM-dd");

    QString create_PH = QString("create table IF NOT EXISTS %1 (id int primary key, date timestamp not null default (datetime('now','localtime')),ph_var real)").arg(str);

    QString delete_all_PH = QString("delete from %1 where date(date)>=date('%2') and date(date)<=date('%3')").arg(str).arg(firstday).arg(firstday);

    qDebug()<<delete_all_PH;

    qDebug()<<"test ....................................................................................................begin";
    if(!database.open())
    {
        qDebug()<<database.lastError();
        qFatal("failed to connect.") ;
    }
    else
    {
        QSqlQuery sql_query;
        qDebug()<<"opend-----";
        sql_query.prepare(create_PH);
        if(!sql_query.exec())
        {
            qDebug()<<sql_query.lastError()<<"err 01 ";
        }
        else
        {
            qDebug()<<"table created!";

        }

        //查询所有数据
        sql_query.prepare(delete_all_PH);
        if(!sql_query.exec())
        {
            qDebug()<<sql_query.lastError()<<"err 03 ";;
        }
        else
        {
            qDebug()<<"table delet for one day!";
        }
    }
    database.close();
}

void MyThread::set_conduct_addr(unsigned char dev_addr,unsigned char addr)
{
    unsigned char cmd[8]={0};
    cmd[0] = dev_addr;
    cmd[1] = 0x06;

    cmd[2] = 0x02;//0x0200
    cmd[3] = 0x00;

    cmd[4] = 0x00;
    cmd[5] = addr;

    unsigned int res = calc_crc16(cmd,6);
    qDebug()<<QString("%1").arg(res,4,16,QLatin1Char('0'));

    cmd[7] = res>>8;//0x1800
    cmd[6] = (res & 0xFF);
    serial->write((char *)cmd,8);

}


/*这里需要增加一个参数接收获取的设备地址*/
void MyThread::get_conduct_addr(unsigned char dev_addr)
{
    unsigned char cmd[8]={0};
    cmd[0] = dev_addr;
    cmd[1] = 0x03;

    cmd[2] = 0x02;//0x0200
    cmd[3] = 0x00;

    cmd[4] = 0x00;
    cmd[5] = 0x01;

    unsigned int res = calc_crc16(cmd,6);

    qDebug()<<QString("%1").arg(res,4,16,QLatin1Char('0'));

    cmd[7] = res>>8;//0x1800
    cmd[6] = (res & 0xFF);

    serial->write((char *)cmd,8);

    unsigned char tmp_buf[16]={0};
    unsigned char result_buf[16]={0};

    if(this->read_needed_data(7,(char*)tmp_buf,(char*)result_buf)==7)
    {

    }

}

int MyThread::read_needed_data(int length,char *tmp_buf,char *result_buf)
{
    int cnt_read = 0;//已经读取到的总字节数
    int cnt_need = length;//还需要的字节数（只针对完整读取指定长度的数据包）
    int cnt_tmp = 0;//存放本次读取到的临时长度

    while(1)
    {

        if(serial->bytesAvailable() >= 1 || serial->waitForReadyRead(1000))//有可读数据再去读
        {
            cnt_need = length - cnt_read;//更新当前还需要读取的字节数
            cnt_tmp = serial->read(tmp_buf,cnt_need);
            if(cnt_tmp > 0)//读取是否成功
            {

                memcpy(result_buf+cnt_read,tmp_buf,cnt_tmp);//每次都把数据累加到cmd_buf
                cnt_read +=  cnt_tmp;
                if(cnt_read == length)//是否读取了指定长度
                {
                    return cnt_read;
                    //qDebug()<<"cnt_read == length";
                    break;
                }
            }
            else if(cnt_tmp == -1)//读串口失败
            {
                qDebug()<<"read err";
                return -1;
            }
            continue;
        }

        qDebug()<<"no data";
        break;
    }
    return 0;
}

void MyThread::get_conduct_val(unsigned char dev_addr)
{
    unsigned char cmd[8]={0};
    cmd[0] = dev_addr;
    cmd[1] = 0x04;

    cmd[2] = 0x00;
    cmd[3] = 0x00;

    cmd[4] = 0x00;
    cmd[5] = 0x05;

    unsigned int crc = calc_crc16(cmd,6);

    qDebug()<<"conduct"<<QString("%1").arg(crc,4,16,QLatin1Char('0'));

    cmd[7] = crc>>8;
    cmd[6] = (crc & 0xFF);

    serial->write((char *)cmd,8);
    unsigned char tmp_buf[16]={0};
    unsigned char result_buf[16]={0};

    if(this->read_needed_data(CONDUCT_LENGTH,(char*)tmp_buf,(char*)result_buf)==CONDUCT_LENGTH)
    {
        this->ec = ((result_buf[7] << 8) + result_buf[8]);
        qDebug()<<"conduct: "<<ec;

    }

}

/*命令是{0x02,0x03,0,0,0,0x01,0x84,0x39}*/
void MyThread::get_PH_val(unsigned char dev_addr)
{
    unsigned char cmd[8]={0};
    cmd[0] = 0x02;
    cmd[1] = 0x03;

    cmd[2] = 0x00;
    cmd[3] = 0x00;

    cmd[4] = 0x00;
    cmd[5] = 0x01;

    unsigned int crc = calc_crc16(cmd,6);

    qDebug()<<"PH:"<<QString("%1").arg(crc,4,16,QLatin1Char('0'));

    cmd[7] = crc>>8;
    cmd[6] = (crc & 0xFF);

    serial->write((char *)cmd,8);
    unsigned char tmp_buf[16]={0};
    unsigned char result_buf[16]={0};

    if(this->read_needed_data(PH_DEV_LENGTH,(char*)tmp_buf,(char*)result_buf)==PH_DEV_LENGTH)
    {
        this->ph_val = ((result_buf[3] << 8) + result_buf[4])/100.0;

        qDebug()<<"ph_val: "<<ph_val;

    }
}


/*命令是{0x01,0x03,0,0,0,0x01,0x84,0x0A}*/
void MyThread::get_light_val(unsigned char dev_addr)
{
    unsigned char cmd[8]={0};
    cmd[0] = dev_addr;
    cmd[1] = 0x03;

    cmd[2] = 0x00;
    cmd[3] = 0x00;

    cmd[4] = 0x00;
    cmd[5] = 0x01;

    unsigned int crc = calc_crc16(cmd,6);

    qDebug()<<"Light:"<<QString("%1").arg(crc,4,16,QLatin1Char('0'));

    cmd[7] = crc>>8;
    cmd[6] = (crc & 0xFF);

    serial->write((char *)cmd,8);
    unsigned char tmp_buf[16]={0};
    unsigned char result_buf[16]={0};

    if(this->read_needed_data(LIGHT_LENGTH,(char*)tmp_buf,(char*)result_buf)==LIGHT_LENGTH)
    {
        this->light = ((result_buf[3] << 8) + result_buf[4])*10;
        qDebug()<<"light: "<<light;
        for(int i = 0;i < 7;i++)
        {
            qDebug()<<result_buf[i];
        }
    }
}

//读取指定长度串口数据（实际不一定能读取指定的长度）
//实际读取到的字节数 小于等于 给定的长度
int MyThread::readFrameData(unsigned int length)
{
    QByteArray tmp_data;//存放本次读取到的临时数据
    int cnt_tmp;//存放本次读取到的临时长度

    cnt_need = length - cnt_read;//更新当前还需要读取的字节数
    tmp_data = serial->read(cnt_need);//本次读取的数据
    if(tmp_data.isEmpty())//如果读取空内容直接返回-1表示读取错误
        return -1;

    cnt_tmp = tmp_data.length();//获取本次读取到的字节数
    cnt_read +=  cnt_tmp;//已经读取的总字节数（只针对读取一个指定长度的数据包）
    read_data += tmp_data;//已经读取的数据（只针对读取一个指定长度的数据包）

    return cnt_tmp;//返回本次读取的实际字节数
}

//处理包头
void MyThread::handleHead()
{
    static unsigned int seq_old = 0;//记录上一次读取到的数据包的序号

    //能够执行这个函数说明已经读完了指定包头长度的数据（表示读完了包头），要进行第二部分指定长度的数据读取了（有效数据的读取）


    //将记录读取状态的相关变量清零
    cnt_read = 0;
    cnt_need = 0;

    //读完头部数据下一个状态应该是读取有效数据--切换状态
    serial_state = READ_DATA;

    //获取包头里的有用信息
    node_type = bytesToInt(read_data.mid(0,1));//节点类型
    node_id = bytesToInt(read_data.mid(1,1));//节点id
    data_need = bytesToInt(read_data.mid(7,1));//有效数据的长度--很重要
    seqnb = bytesToInt(read_data.mid(6,1))*256 + bytesToInt(read_data.mid(5,1));//该数据包的序号--也很重要

    //如果该数据包序号和上一个数据包序号相等--读重复了
    if(seqnb == seq_old)
    {
        FAIL_FLAG = true;//重复--读失败的标志置位--该数据包将不会写入图片文件
        qDebug()<<"seqnb err..********************************";
    }
    else if(seqnb - seq_old > 1 && seqnb !=1)//如果该数据包序号比上一个数据包序号大于等于2，表示中间丢了数据包
    {
        //虽然读取失败了，但是该数据包是需要写入图片文件的，所以失败标志复位--要写入该数据包到图片文件
        FAIL_FLAG = false;

        //计算丢失的数据包个数
        int need_pkt = seqnb - seq_old;
        QByteArray errData(64*(need_pkt-1),0);//构造一个和丢失的所有数据包总和一样大、全0的数据包

        my_stream += errData;//将上述构造的数据包加到缓冲区
        dst.write(my_stream);//将缓冲区所有数据写入图片文件
        my_stream.clear(); //清空缓冲区
        pkt_cnt = 0;//记录缓冲区是否满32*64的变量清零--表示重新计数（此时缓冲区已经没有数据了）

    }
    else {//既没有重复读取也没有丢失数据包
        FAIL_FLAG = false;
    }
    seq_old = seqnb;//更新旧的数据包序号

    //发送信号给主线程--更新UI界面（携带包头数据）
    emit isDone(read_data);

    //清空读取到的数据（只针对读取一个指定长度的数据包）
    read_data.clear();
}


//处理图片实际数据
void  MyThread::handlePhoto()
{

    //能够执行这个函数说明已经读完了一个数据包的效数据，要进行下一个数据包的数据读取了（又要从包头开始读取）

    //记录读取状态的变量清零--以便下一个指定长度的读取
    cnt_read = 0;
    cnt_need = 0;

    serial_state = READ_HDR;//更新串口读取状态--立马要读取下一个数据包的包头了

    //if(photo_state == END && strcmp("end\r\n",str_tmp) == 0)
    //if(strcmp("end",str_tmp) == 0)
    if(read_data.contains("end"))//如果读取到的有效数据包含end--表示读取到图片结束
    {

        //debug = 1;

        QFileInfo fi = QFileInfo(dst.fileName());

        QString ImageName = fi.absolutePath() +"/";
        ImageName += dst.fileName();
        qDebug()<<ImageName;


        //QImage *image=new QImage(ImageName);
        emit ImageOK(ImageName);
        //while(1);

        //打印调试信息
        qDebug()<<"end";
        qDebug()<<cnt;

        //清空相关缓冲区和--准备读取下一个数据包了
        //（只针对读取一个指定长度的数据包）已读取的数据清空
        read_data.clear();

        //缓冲区清空
        my_stream.clear();
        photo_state = BEGIN;//更新图片的读取状态（暂时没用到）

        data_need = 0;//图片的有效数据长度清零
        cnt = 0;//读取到的所有有效数据清零
        emit end();

        if(dst.isOpen())//关闭图片文件
            dst.close();
        return ;//直接返回--后面的没必要再执行了
    }

    //if(photo_state == BEGIN && strcmp("begin\r\n",str_tmp) == 0)
    //if(strcmp("begin\r\n",str_tmp) == 0)
    if(read_data.contains("begin"))//如果读取到的有效数据包含begin--表示准备开始读取图片
    {

        //将系统当前时间作为图片的文件名
        QDateTime time = QDateTime::currentDateTime();//获取系统现在的时间

        //设置显示格式,注意QFile的文件名不能有：（冒号）等特殊字符
        QString str = time.toString("yyyy-MM-dd-hh-mm-ss");
        str += ".jpg";

        qDebug()<<str;
        if(dst.isOpen())
            dst.close();

        dst.setFileName(str);//将时间作为文件名
        //QDir::currentPath()
        bool isOK = dst.open(QIODevice::WriteOnly|QIODevice::Append); //打开文件
        if(isOK == false)//打开失败
        {
            qDebug()<<"dst.open err";
//            //this->close();
        }

        //清空相关缓冲区和--准备读取下一个数据包了
        read_data.clear();
        data_need = 0;

        photo_state = READING;//更新图片读取状态（暂时没用到）

        return ;//直接返回--后面的没必要再执行了
    }
    //if(photo_state == READING)//读取图片的实际有效数据
    {
        cnt += read_data.length();//累加图片的有效数据

        //如果重复了--不会写入当前数据包到图片文件
        if(FAIL_FLAG == true)
        {
            //清空已读取到的数据（但不清空缓冲区--实际不重复的有效数据没有满32*64）--准备读取下一个数据包的数据
            read_data.clear();
            data_need = 0;//有效数据长度清零
            return;//直接返回
        }
//                       //qDebug()<<"==========>"<<seqnb;
        pkt_cnt++;//记录不重复的有效数据包的个数
        my_stream += read_data;//不重复的有效数据包累加到缓冲区

        //如果当前数据包的有效长度小于给定的有效数据包的长度的宏定义--表示是最后一个数据包
        if(data_need < REAL_LEN)
        {
            photo_state = END;//更新图片读取状态（暂时没用到）
            dst.write(my_stream);//将当前缓冲区的数据写入图片文件
            my_stream.clear();//清空缓冲区
            pkt_cnt = 0;//记录不重复有效数据包的个数清零
        }

        //还没有读取完毕，但是缓冲区已经满了32*64（这里的64是一个数据包里有效数据的最大长度--对应宏定义REAL_LEN）
        if(pkt_cnt == 32)
        {
            pkt_cnt = 0;//记录不重复有效数据包的个数清零
            dst.write(my_stream);//将当前缓冲区的数据写入图片文件
            my_stream.clear();//清空缓冲区
        }


        //已读取的数据清空（只针对读取一个指定长度的数据包）
        read_data.clear();
        data_need = 0;//图片的有效数据长度清零
    }

}

//读取串口数据--真正子线程处理函数
void MyThread::readSerial()
{
    int ret = 0;//记录每次实际读取的字节数

   // while(!isStop)
    {

#if 1

        //一定要调用这个函数，否则串口不会发出readyRead信号（或者说即使发了也没有去捕获），也就什么都不能读取了
       //while(this->serial->waitForReadyRead(8) == false);
        //if(serial->bytesAvailable() < 1 && debug == 0){
            //qDebug() << "子线程号：========================"<< QThread::currentThread() ;
        //}

        if(serial->bytesAvailable() >= 1 || serial->waitForReadyRead(8))//有可读数据再去读
        {
            switch(serial_state)
            {

            case READ_HDR://读包头
                ret = readFrameData(HDR_LEN);//目标是读取HDR_LEN个字节的数据
                if(ret == -1)//读取错误
                {
                    return;
                }

                if(HDR_LEN == cnt_read)//已读取到的字节数和目标长度相等
                    handleHead();//处理头部数据
                break;

            case READ_DATA://读有效数据
                if(node_type == 1)//如果是图像节点的数据（不再判断节点--目前只有一个节点的数据）
                {
                    ret = readFrameData(data_need);//读取有效长度（从包头获取）的数据
                    if(ret == -1)//读取错误
                    {
                        return;
                    }

                    if(data_need == cnt_read)//已读取到的字节数和目标长度相等
                        handlePhoto();//处理有效数据
                }

                break;
            default:
                break;

            }
        }
#else

        //if(this->serial->waitForReadyRead(10) == false)
            //qDebug() << "子线程号：========================"<< QThread::currentThread() ;
        if(serial->bytesAvailable() >= 1)//有可读数据再去读
        {
            tmp += serial->readAll();
            if(tmp.length()>0)
            {
                if(tmp.contains("\r\n"))
                {
                    emit isDone(tmp);
                    tmp.clear();
                }
            }

        }
#endif

    }
}

//读取串口数据--真正子线程处理函数
void MyThread::readUart485()
{
    int ret = 0;//记录每次实际读取的字节数

    {
        if(serial->bytesAvailable() < 1 )
             qDebug() << "子线程号：========================"<< QThread::currentThread() ;
        if(serial->bytesAvailable() >= 1)//有可读数据再去读
        {
            switch(serial_state)
            {

            case READ_PH:
                qDebug()<<"in";
                ret = readFrameData(PH_DEV_LENGTH);//目标是读取PH_DEV_LENGTH个字节的数据
                if(ret == -1)//读取错误
                {
                    return;
                }

                if(PH_DEV_LENGTH == cnt_read)//已读取到的字节数和目标长度相等
                {
                    //将记录读取状态的相关变量清零
                    cnt_read = 0;
                    cnt_need = 0;

                    //读完头部数据下一个状态应该是读取有效数据--切换状态

                    int high = bytesToInt(read_data.mid(3,1));
                    int low = bytesToInt(read_data.mid(4,1));
                    this->ph_val = ((high << 8) + low)/100.0;
                    qDebug()<<"ph_val: "<<ph_val;
                    read_data.clear();
                }

                break;

            case READ_LIGHT:
                ret = readFrameData(LIGHT_LENGTH);//目标是读取LIGHT_LENGTH个字节的数据
                if(ret == -1)//读取错误
                {
                    return;
                }

                if(LIGHT_LENGTH == cnt_read)
                {
                    //将记录读取状态的相关变量清零
                    cnt_read = 0;
                    cnt_need = 0;

                    //读完头部数据下一个状态应该是读取有效数据--切换状态

                    int high = bytesToInt(read_data.mid(3,1));
                    int low = bytesToInt(read_data.mid(4,1));
                    this->light = ((high << 8) + low)*10;
                    qDebug()<<"light: "<<light;

                    //get_conduct_val(CONDUCT_ADDR);
                    //serial_state = READ_CONDUCT;
                    read_data.clear();
                }
                break;
            case READ_CONDUCT://读有效数据
                ret = readFrameData(CONDUCT_LENGTH);//目标是读取CONDUCT_LENGTH个字节的数据
                if(ret == -1)//读取错误
                {
                    return;
                }

                if(CONDUCT_LENGTH == cnt_read)
                {
                    //将记录读取状态的相关变量清零
                    cnt_read = 0;
                    cnt_need = 0;

                    //读完头部数据下一个状态应该是读取有效数据--切换状态

                    int high = bytesToInt(read_data.mid(7,1));
                    int low = bytesToInt(read_data.mid(8,1));
                    this->ec = ((high << 8) + low);
                    qDebug()<<"ec: "<<ec;

//                    get_light_val(LIGHT_ADDR);
//                    serial_state = READ_LIGHT;

                    read_data.clear();
                }
                break;

            default:
                break;

            }
        }
    }
}

void MyThread::setFlag(bool flag)
{
    isStop = flag;

    //5. 子线程中声明、初始化的对象在子线程中析构，
    //利用deleteLater该函数可很好解决多线程释放对象
    serial->deleteLater();
    if(timer != NULL)
        timer->deleteLater();
    qDebug() << "stop";
}

void MyThread::test(QString str)
{
    QDateTime current_date = QDateTime::currentDateTime();
    QString yesterday = current_date.addDays(-30).toString("yyyy-MM-dd");

    QString create_PH = QString("create table IF NOT EXISTS %1 (id int primary key, date timestamp not null default (datetime('now','localtime')),ph_var real)").arg(str);
    //QString select_all_PH = QString("select * from %1 where time(date)>=time('02:42:38') and time(date)<=time('02:42:38')").arg(str);
    //QString insert_PH = QString("insert into %1(id,ph_var) values(?,?)").arg(str);--ok
    //select * from MyDate where datetime( datatime)>=datetime('2016-01-02') and datetime(datatime)<=datetime('2016-01-05')--ok
    //QString create_PH = QString("create table IF NOT EXISTS %1 (id int primary key, date timestamp not null default (datetime('now','localtime')),ph_var real)").arg(str);--ok
    //QString select_all_PH = QString("select * from %1 where time(date)>=time('02:42:38') and time(date)<=time('02:42:38')").arg(str);--ok
    //QString select_all_PH = QString("select * from %1 where date(date)>=date('%2') and date(date)<=date('%3')").arg(str).arg(yesterday).arg(yesterday);--ok
    //QString select_all_PH = QString("delete from %1 where date(date)>=date('%2') and date(date)<=date('%3')").arg(str).arg(yesterday).arg(yesterday);--ok
    QString query_cnt =  QString("select count(*) from %1").arg(str);

    qDebug()<<query_cnt;

    qDebug()<<"test ....................................................................................................begin";
    if(!database.open())
    {
        qDebug()<<database.lastError();
        qFatal("failed to connect.") ;
    }
    else
    {
        QSqlQuery sql_query;
        qDebug()<<"opend-----";
        sql_query.prepare(create_PH);
        if(!sql_query.exec())
        {
            qDebug()<<sql_query.lastError()<<"err 01 ";
        }
        else
        {
            qDebug()<<"table created!";

        }

        //查询所有数据
        QString query_cnt =  QString("select count(*) from %1").arg(str);
        sql_query.prepare(query_cnt);
        if(!sql_query.exec())
        {
            qDebug()<<sql_query.lastError()<<"err 03 ";;
        }
        else
        {
            qDebug()<<"table select!";


            QDateTime date;
            QString str;
            qreal var;
            int id;
            while(sql_query.next())
            {
                id = sql_query.value(0).toInt();
                date = sql_query.value(1).toDateTime();
                str = date.toLocalTime().toString();
                var = sql_query.value(2).toDouble();

                qDebug()<<QString("id:%1    date:%2    var:%3").arg(id).arg(str).arg(var);
                //qDebug()<<QString("date:%1    var:%2").arg(str).arg(var);
            }

            qDebug()<<"test ....................................................................................................";
        }
    }
    database.close();
}

void MyThread::updateShow(QString str)
{

    QDateTime current_date = QDateTime::currentDateTime();
    //QString yesterday = current_date.addDays(0).toString("HH:MM:SS");

    QString create_PH = QString("create table IF NOT EXISTS %1 (id int primary key, date timestamp not null default (datetime('now','localtime')),ph_var real)").arg(str);
    QString select_all_PH = QString("select * from %1 order by date desc limit 0,96").arg(str);
    //QString select_all_PH = QString("select * from %1").arg(str);
    //QString insert_PH = QString("insert into %1(id,ph_var) values(?,?)").arg(str);

    if(!database.open())
    {
        qDebug()<<database.lastError();
        qFatal("failed to connect.") ;
    }
    else
    {
        QSqlQuery sql_query;
        qDebug()<<"opend-----";
        sql_query.prepare(create_PH);
        if(!sql_query.exec())
        {
            qDebug()<<sql_query.lastError()<<"err 01 ";
        }
        else
        {
            qDebug()<<"table created!";

        }

        //查询所有数据
        sql_query.prepare(select_all_PH);
        if(!sql_query.exec())
        {
            qDebug()<<sql_query.lastError()<<"err 03 ";;
        }
        else
        {
            qDebug()<<"table select!";
             QMap<QDateTime,qreal> tmp_map;

            if(tmp_map.count() > 0)
                tmp_map.clear();
            QDateTime date;
            QString str;
            qreal var;
            int id;
            while(sql_query.next())
            {
                id = sql_query.value(0).toInt();
                date = sql_query.value(1).toDateTime();
                str = date.toLocalTime().toString();
                var = sql_query.value(2).toDouble();
                tmp_map.insert(date,var);
                //qDebug()<<QString("id:%1    date:%2    var:%3").arg(id).arg(str).arg(var);
                //qDebug()<<QString("date:%1    var:%2").arg(str).arg(var);
            }

            emit DynamicShow(tmp_map);

            qDebug()<<"emit ..............................";
        }
    }
    database.close();
}

void MyThread::updateTablePH(QString str)
{

    static int primary_key = 0;

    QString create_PH = QString("create table IF NOT EXISTS %1 (id int primary key, date timestamp not null default (datetime('now','localtime')),ph_var real)").arg(str);
    //QString select_all_PH = QString("select * from %1").arg(str);
    //QString insert_PH = QString("insert into %1(id,ph_var) values(?,?)").arg(str);
    QString insert_PH = QString("insert into %1(ph_var) values(?)").arg(str);
    //DELETE FROM CACHEYX WHERE id IN(SELECT id FROM CACHEYX ORDER BY TIME DESC LIMIT 3);
    QString delete_PH = QString("delete from %1 where id in(select id from %2 order by date limit 96)").arg(str).arg(str);


    if(!database.open())
    {
        qDebug()<<database.lastError();
        qFatal("failed to connect.") ;
    }
    else
    {
        //QSqlQuery类提供执行和操作的SQL语句的方法。
        //可以用来执行DML（数据操作语言）语句，如SELECT、INSERT、UPDATE、DELETE，
        //以及DDL（数据定义语言）语句，例如CREATE TABLE。
        //也可以用来执行那些不是标准的SQL的数据库特定的命令。
        QSqlQuery sql_query;
        qDebug()<<"opend-----";
        sql_query.prepare(create_PH);
        if(!sql_query.exec())
        {
            qDebug()<<sql_query.lastError()<<"err 01 ";
        }
        else
        {
            qDebug()<<"table created!";

        }




        QVariantList  vars;

        QVariantList  ids;

        QString query_cnt =  QString("select count(*) from %1").arg(str);
        sql_query.prepare(query_cnt);
        int cnt;
        if(!sql_query.exec())
        {
            qDebug()<<sql_query.lastError()<<"err 03 ";;
        }
        else
        {



            while(sql_query.next())
            {
                cnt = sql_query.value(0).toInt();
                qDebug()<<QString("count:%1").arg(cnt);

            }

        }

        if(cnt >= 2000)
        {
            sql_query.prepare(delete_PH);
            if(!sql_query.exec())
            {
                qDebug()<<sql_query.lastError()<<"err 03 ";;
            }
            else
            {
                qDebug()<<"delete ..................................................................................";
            }
        }

        sql_query.prepare(insert_PH);
#if 0
        for(int i = cnt;i < 96*3;i++)
        {
            ids.append(i+1);

        }
        for(int i = cnt;i < 96*3;i++)
        {
            vars.append(7.5);

        }
#else


        primary_key = cnt;
        if(primary_key >= 96*31)
            primary_key =0;
         primary_key++;

         ids.append(primary_key);
         vars.append(ph_val);

#endif


        //sql_query.addBindValue(ids);
        sql_query.addBindValue(vars);
        if(!sql_query.execBatch())
        {
            qDebug()<<sql_query.lastError();
        }
        else
        {
            qDebug()<<"table insert!";
        }


    }
    database.close();
}



void MyThread::sendToUart(QByteArray tmp)
{
    unsigned char len = (unsigned char)tmp.length();

    QByteArray res;
    res.clear();

    res.append(len);
    res += tmp;

    qDebug()<<(int)(len);

    qint64 ret = serial->write(res);

    qDebug()<<ret<<"send";
}
