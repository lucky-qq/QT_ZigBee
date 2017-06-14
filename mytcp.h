#ifndef MYTCP_H
#define MYTCP_H

#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>

class MyTCP : public QObject
{
    Q_OBJECT
public:
    explicit MyTCP(QObject *parent = 0);
    void setFlag(bool flag);

signals:

public slots:
    void connectToGateway();
    void tcp_send(char * str,int len);


private:
    QTcpSocket *tcpSocket; //通信套接字
};

#endif // MYTCP_H
