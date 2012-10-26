#include "DTIDERegisters.h"

DTIDERegisters::DTIDERegisters(QWidget* parent): QWidget(parent)
{
    setupUi(this);
    connect(btn_step, SIGNAL(clicked()), this, SIGNAL(step()));
    connect(btn_run, SIGNAL(clicked()), this, SIGNAL(start()));
    connect(btn_stop, SIGNAL(clicked()), this, SIGNAL(pause()));
}

QSize DTIDERegisters::sizeHint()
{
    return QSize(400, 400);
}

void DTIDERegisters::setRegisters(StatusMessage m)
{
    QStringList registers;
    for (int i = 0; i < 8; i++)
    {
        QString r = QString("0x%1").arg(m.registers[i], 4, 16, QChar('0'));
        registers.append(r);
    }

    // TODO: Fix this mess.

    reg_A->setText(registers[0]);
    reg_B->setText(registers[1]);
    reg_C->setText(registers[2]);
    reg_X->setText(registers[3]);
    reg_Y->setText(registers[4]);
    reg_Z->setText(registers[5]);
    reg_I->setText(registers[6]);
    reg_J->setText(registers[7]);

    reg_PC->setText(QString("0x%1").arg(m.pc, 4, 16, QChar('0')));
    reg_SP->setText(QString("0x%1").arg(m.sp, 4, 16, QChar('0')));
    reg_EX->setText(QString("0x%1").arg(m.ex, 4, 16, QChar('0')));
    reg_IA->setText(QString("0x%1").arg(m.ia, 4, 16, QChar('0')));
}
