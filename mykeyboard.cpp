#include "mykeyboard.h"

MyKeyboard::MyKeyboard(QWidget *parent) : QWidget(parent)
{
    key_board = new QPushButton[12];
    h_layout = new QHBoxLayout[4];

    main_layout = new QVBoxLayout();

    for(int i = 0;i < 4;i++)
    {
        for(int j = 0; j < 3;j++)
        {
            (h_layout+i)->addWidget(key_board+j+3*i);
            (key_board+j+3*i)->setText(QString("%1").arg(j+3*i+1));
        }
        main_layout->addLayout(h_layout + i);
    }

    (key_board+9)->setText(QString("OK"));
    (key_board+10)->setText(QString("0"));
    (key_board+11)->setText(QString("<--"));

    this->setLayout(main_layout);
}

//MyKeyboard::~MyKeyboard()
//{
//    delete []key_board;
//    delete []h_layout;
//}


