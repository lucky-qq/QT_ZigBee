#include "mytcp.h"
#include <QDebug>
#include <QHostAddress>

#define IP ("192.168.1.125")

MyTCP::MyTCP(QObject *parent) : QObject(parent)
{
    tcpSocket = new QTcpSocket(this);
    qDebug()<<"MyTCP";
    connect(tcpSocket, &QTcpSocket::connected,
                [=]()
                {
                    qDebug()<<"chenggongconnect*********************";
                }
                );


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
}
