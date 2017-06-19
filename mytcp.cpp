#include "mytcp.h"
#include <QDebug>
#include <QHostAddress>
#include <QDataStream>
#include "mythread.h"

#define IP ("192.168.1.125")

MyTCP::MyTCP(QObject *parent) : QObject(parent)
{
    tcpSocket = new QTcpSocket(this);
    loadSize = 4*1024;
    totalBytes = 0;
    bytesWritten = 0;
    bytesToWrite = 0;
    pic_flag = false;
    qDebug()<<"MyTCP";
    connect(tcpSocket, &QTcpSocket::connected,
                [=]()
                {
                    qDebug()<<"chenggongconnect*********************";
                }
                );

    connect(tcpSocket,SIGNAL(bytesWritten(qint64)),this,
           SLOT(updateClientProgress(qint64)));
    connect(tcpSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,
           SLOT(displayError(QAbstractSocket::SocketError)));


}

void  MyTCP::connectToGateway()
{

    qDebug()<<"connectToGateway*******************************************";
    tcpSocket->connectToHost(QHostAddress("192.168.1.125"), 9999);

    qDebug()<<"connectToGateway*******************************************";
}

void MyTCP::setFlag(bool flag)
{
    if(flag == true)
    {
        tcpSocket->disconnectFromHost();
        tcpSocket->close();
    }

}


void MyTCP::tcp_send(char * str,int len)
{
    tcpSocket->write(str,len);
    qDebug()<<"tcp_send*******************************************************************************";
}

void MyTCP::startTransfer(QString pic_path)  //实现文件大小等信息的发送
{
#if 1
    fileName = pic_path;
    localFile = new QFile(fileName);
    if(!localFile->open(QFile::ReadOnly))
    {
       qDebug() << "open file error!";
       return;
    }

    //文件总大小
    totalBytes = localFile->size();

    outBlock.resize(0);
    QDataStream sendOut(&outBlock,QIODevice::WriteOnly);
    //sendOut.setVersion(QDataStream::Qt_4_6);
    QString currentFileName = fileName.right(fileName.size()
    - fileName.lastIndexOf('/')-1);

    qDebug()<<currentFileName;
    qDebug()<<currentFileName.size();
    char*  ch;

    QByteArray ba = currentFileName.toLatin1();

    ch=ba.data();
    qDebug()<<ch;

    //依次写入总大小信息空间，文件名大小信息空间，文件名
    sendOut <<qint8(0)<<qint8(0)<<qint8(0)<<qint8(0)<< qint64(0) << qint64(0) << ch;

    //这里的总大小是文件名大小等信息和实际文件大小的总和
    totalBytes += outBlock.size();
    qDebug()<<outBlock.size();

    sendOut.device()->seek(0);
    //返回outBolock的开始，用实际的大小信息代替两个qint64(0)空间
    sendOut<<qint8(BASE)<<qint8(PHOTO_TYPE)<<qint8(1)<<qint8(outBlock.size()-sizeof(qint8)*4)<<totalBytes<<qint64((outBlock.size() - sizeof(qint64)*2-sizeof(qint8)*4));

    //发送完头数据后剩余数据的大小
    bytesToWrite = totalBytes - tcpSocket->write(outBlock);

    //ui->clientStatusLabel->setText(tr("已连接"));
    outBlock.resize(0);
    pic_flag = true;
#endif
}

//更新进度条，实现文件的传送
void MyTCP::updateClientProgress(qint64 numBytes)
{
    if(pic_flag == false)
        return ;
    //已经发送数据的大小
    bytesWritten += (int)numBytes;

    if(bytesToWrite > 0) //如果已经发送了数据
    {
   //每次发送loadSize大小的数据，这里设置为4KB，如果剩余的数据不足4KB，
   //就发送剩余数据的大小
       outBlock = localFile->read(qMin(bytesToWrite,loadSize));

       //发送完一次数据后还剩余数据的大小
       bytesToWrite -= (int)tcpSocket->write(outBlock);

       //清空发送缓冲区
       outBlock.resize(0);

    } else {
       localFile->close(); //如果没有发送任何数据，则关闭文件
       pic_flag = false;
       bytesWritten = 0;
    }

    //更新进度条
    //ui->clientProgressBar->setMaximum(totalBytes);
    //ui->clientProgressBar->setValue(bytesWritten);
    qDebug() << "send pic "<<bytesWritten<<"now :"<<numBytes;

    if(bytesWritten == totalBytes) //发送完毕
    {
     //ui->clientStatusLabel->setText(tr("传送文件 %1 成功")
//.arg(fileName));
       localFile->close();
       pic_flag = false;
       bytesWritten = 0;
       qDebug() << "send pic OK\r\n\r\n\r\n!";
       //tcpClient->close();
    }

}
void MyTCP::displayError(QAbstractSocket::SocketError) //显示错误
{
    qDebug() << tcpSocket->errorString();
    tcpSocket->close();
    //ui->clientProgressBar->reset();
    //ui->clientStatusLabel->setText(tr("客户端就绪"));
    //ui->sendButton->setEnabled(true);
}
