#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent): QWidget(parent), ui(new Ui::Widget), HOLES(33)
{
    ui->setupUi(this);
    connect(ui->pushButton, &QPushButton::released, this, &Widget::solve);

    board = new bool[HOLES];
    squares = new Square[HOLES];
    squares[HOLES/2].changePegState();

    paintSquares();
}

Widget::~Widget()
{
    delete ui;
    delete[] board;
    delete[] squares;
}

void Widget::setBoardFromPegsState()
{
    for(int i=0; i<HOLES; ++i)
        board[i] = squares[i].getPegState();
}

void Widget::setPegsStateFromBoard()
{
    for(int i=0; i<HOLES; ++i)
        squares[i].setPegState(board[i]);
}

void Widget::paintSquares()
{
    for(int i=0; i<3; ++i) {
        ui->gridLayout->addWidget(&squares[i], 0, i+2);
        ui->gridLayout->addWidget(&squares[3+i], 1, i+2);

        ui->gridLayout->addWidget(&squares[27+i], 5, i+2);
        ui->gridLayout->addWidget(&squares[30+i], 6, i+2);
    }
    for(int i=0; i<7; ++i) {
        ui->gridLayout->addWidget(&squares[6+i], 2, i);
        ui->gridLayout->addWidget(&squares[13+i], 3, i);
        ui->gridLayout->addWidget(&squares[20+i], 4, i);
    }
}

void Widget::lockSquares()
{
    for(int i=0; i<HOLES; ++i)
        squares[i].setEnabled(false);
}

void Widget::unlockSquares()
{
    for(int i=0; i<HOLES; ++i)
        squares[i].setEnabled(true);
}

void Widget::solve()
{
    ui->pushButton->setEnabled(false);
    lockSquares();
    setBoardFromPegsState();
}

void Widget::resizeEvent(QResizeEvent*)
{
    int margin;

    if(QWidget::width() >= QWidget::height()-50) {
        margin = (QWidget::width()+50 - QWidget::height()) / 2;
        ui->gridLayout->setContentsMargins(margin, 0, margin, 0);

        ui->gridLayout->setSpacing((QWidget::height()) / QWidget::minimumHeight());
    }
    else {
        margin = (QWidget::height()-50 - QWidget::width()) / 2;
        ui->gridLayout->setContentsMargins(0, margin, 0, margin);

        ui->gridLayout->setSpacing((QWidget::width()) / QWidget::minimumWidth());
    }
}
