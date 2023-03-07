#include "widget.h"
#include "ui_widget.h"

#include "solver.h"

#include <QDebug>

Widget::Widget(QWidget* parent): QWidget(parent), ui(new Ui::Widget), HOLES(33)
{
    ui->setupUi(this);
    connect(ui->solveButton, &QPushButton::released, this, &Widget::solve);
    connect(ui->leftButton, &QPushButton::released, this, &Widget::changeSquaresToPrevBoard);
    connect(ui->rightButton, &QPushButton::released, this, &Widget::changeSquaresToNextBoard);

    boards = new bool*[HOLES-1];
    for(int i=0; i<HOLES-1; ++i)
        boards[i] = new bool[HOLES];

    selectedBoard = 0;

    squares = new Square[HOLES];
    squares[HOLES/2].changePegState();

    addSquaresToGrid();
}

Widget::~Widget()
{
    delete ui;
    for(int i=0; i<HOLES-1; ++i)
        delete[] boards[i];
    delete[] boards;
    delete[] squares;
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

void Widget::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_A || e->key() == Qt::Key_S || e->key() == Qt::Key_Backspace || e->key() == Qt::Key_PageUp) {
        emit backPressed();
    }
    else if(e->key() == Qt::Key_D || e->key() == Qt::Key_W || e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter || e->key() == Qt::Key_PageDown) {
        emit forwardPressed();
    }
}

void Widget::setCurrentBoardFromSquaresState(const bool& updatePegsN)
{
    if(updatePegsN) pegs=0;
    for(int i=0; i<HOLES; ++i) {
        boards[selectedBoard][i] = squares[i].getPegState();
        if(updatePegsN && boards[selectedBoard][i]) ++pegs;
    }
}

void Widget::setSquaresStateFromCurrentBoard(const bool& updatePegsN)
{
    if(updatePegsN) pegs=0;
    for(int i=0; i<HOLES; ++i) {
        if(boards[selectedBoard][i] ^ squares[i].getPegState()) {
            squares[i].setPegState(boards[selectedBoard][i]);
            squares[i].repaint();
        }
        if(updatePegsN && boards[selectedBoard][i]) ++pegs;
    }
}

void Widget::addSquaresToGrid()
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

void Widget::changeSquaresToNextBoard()
{
    ++selectedBoard;
    if(pegs) selectedBoard %= pegs;

    setSquaresStateFromCurrentBoard(false);
}

void Widget::changeSquaresToPrevBoard()
{
    --selectedBoard;
    if(selectedBoard < 0)
        selectedBoard += pegs;

    setSquaresStateFromCurrentBoard(false);
}

void Widget::solve()
{
    lockSquares();
    setCurrentBoardFromSquaresState();

    Solver* solver = new Solver(boards, HOLES, pegs);
    solver->run();
    if(!solver->getResult(boards)) {
        changeSquaresToNextBoard();
        setSquaresStateFromCurrentBoard();
    }
    else {
        changeSquaresToPrevBoard();
        ui->leftButton->setEnabled(true);
        ui->rightButton->setEnabled(true);
        connect(this, &Widget::backPressed, this, &Widget::changeSquaresToPrevBoard);
        connect(this, &Widget::forwardPressed, this, &Widget::changeSquaresToNextBoard);
    }
    disconnect(ui->solveButton, &QPushButton::released, this, &Widget::solve);
    ui->solveButton->setText("Reset");
    connect(ui->solveButton, &QPushButton::released, this, &Widget::reset);
    delete solver;
}

void Widget::reset()
{
    unlockSquares();
    selectedBoard = 0;
    setSquaresStateFromCurrentBoard();
    ui->leftButton->setEnabled(false);
    ui->rightButton->setEnabled(false);
    disconnect(this, &Widget::backPressed, this, &Widget::changeSquaresToPrevBoard);
    disconnect(this, &Widget::forwardPressed, this, &Widget::changeSquaresToNextBoard);

    disconnect(ui->solveButton, &QPushButton::released, this, &Widget::reset);
    ui->solveButton->setText("Solve");
    connect(ui->solveButton, &QPushButton::released, this, &Widget::solve);
}
