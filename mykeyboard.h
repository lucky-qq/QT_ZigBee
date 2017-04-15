#ifndef MYKEYBOARD_H
#define MYKEYBOARD_H

#include <QWidget>
#include "frmmain.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>


class MyKeyboard : public QWidget
{
    Q_OBJECT
public:
    explicit MyKeyboard(QWidget *parent = 0);

signals:

public slots:
private:

    QHBoxLayout *h_layout;
    QVBoxLayout *main_layout;

public:
    QPushButton * key_board;
};

#endif // MYKEYBOARD_H
