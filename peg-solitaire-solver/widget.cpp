#include "widget.h"
#include "ui_widget.h"

#include <QDebug>

#include <QRegularExpression>
#include <QTemporaryFile>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTextStream>


QString getRuleForMoveXYZ(const int& ID, const int ARR[3], const int& MOVE, const int& HOLES)
{
    QString result = "";

    result += QString::number(-ID) + " " + QString::number(ARR[1]+MOVE) + " 0\n";
    result += QString::number(-ID) + " " + QString::number(-ARR[1]-MOVE-HOLES) + " 0\n";

    result += QString::number(-ID) + " " + QString::number(ARR[0]+MOVE) + " " + QString::number(ARR[2]+MOVE) + " 0\n";
    result += QString::number(-ID) + " " + QString::number(-ARR[0]-MOVE) + " " + QString::number(-ARR[2]-MOVE) + " 0\n";

    result += QString::number(-ID) + " " + QString::number(ARR[2]+MOVE) + " " + QString::number(ARR[2]+MOVE+HOLES) + " 0\n";
    result += QString::number(-ID) + " " + QString::number(-ARR[2]-MOVE) + " " + QString::number(-ARR[2]-MOVE-HOLES) + " 0\n";
    result += QString::number(-ID) + " " + QString::number(ARR[0]+MOVE) + " " + QString::number(ARR[0]+MOVE+HOLES) + " 0\n";
    result += QString::number(-ID) + " " + QString::number(-ARR[0]-MOVE) + " " + QString::number(-ARR[0]-MOVE-HOLES) + " 0\n";

    return result;
}

std::vector<int> getFirstBoardTransition(const int possibleMoves[][3], const int& HOLES, const int& PEGS, const int& RULES)
{
    std::vector<int> result;

    for(int i=1; i<=HOLES; ++i) {
        std::vector<int> tmp;
        for(int j=0; j<RULES; ++j) {
            if(possibleMoves[j][0] > i) break;
            if(possibleMoves[j][0] == i) {
                tmp.push_back(HOLES*PEGS + j + 1);
                continue;
            }
            if(possibleMoves[j][1] > i) continue;
            if(possibleMoves[j][1] == i) {
                tmp.push_back(HOLES*PEGS + j + 1);
                continue;
            }
            if(possibleMoves[j][2] > i) continue;
            if(possibleMoves[j][2] == i) {
                tmp.push_back(HOLES*PEGS + j + 1);
            }
        }

        result.push_back(-i);
        result.push_back(i+HOLES);
        result.insert(result.end(), tmp.begin(), tmp.end());
        result.push_back(0);

        result.push_back(i);
        result.push_back(-i-HOLES);
        result.insert(result.end(), tmp.begin(), tmp.end());
        result.push_back(0);
    }

    return result;
}

QString initializeRulesGlobal(const int possibleMoves[][3], const int& HOLES, const int& PEGS, const int& MOVES, const int& RULES)
{
    const std::vector<int> firstTransition = getFirstBoardTransition(possibleMoves, HOLES, PEGS, RULES);
    QString result = "";

    for(int m=0; m<MOVES; ++m) {
        QString loopResult = "";

        for(int i=0; i<RULES; ++i) {
            loopResult += QString::number(HOLES*PEGS + m*RULES + i + 1) + " ";
            result += getRuleForMoveXYZ(HOLES*PEGS + m*RULES + i + 1, possibleMoves[i], m*HOLES, HOLES);

            for(int j=0; j<i; ++j) {
                result += QString::number(-(HOLES*PEGS + m*RULES + i + 1)) + " " + QString::number(-(HOLES*PEGS + m*RULES + j + 1)) + " 0\n";
            }
        }
        loopResult += "0\n";

        int size = firstTransition.size();
        int i=0;
        while(i < size) {
            if(firstTransition[i] < 0) {
                result += QString::number(firstTransition[i] - m*HOLES) + " ";
                ++i;
                result += QString::number(firstTransition[i] + m*HOLES) + " ";
            }
            else {
                result += QString::number(firstTransition[i] + m*HOLES) + " ";
                ++i;
                result += QString::number(firstTransition[i] - m*HOLES) + " ";
            }

            while(firstTransition[++i]) {
                result += QString::number(firstTransition[i] + m*RULES) + " ";

            }
            result += "0\n";

            ++i;
        }
        result += loopResult;
    }

    return result;
}

QString initializePegVariablesCenterHole(const bool* BOARD, const int& HOLES, const int& PEGS, const int& MOVES, const int& RULES)
{
    int i=0;
    int sum=0;
    for(int i=1; i<RULES; ++i) sum += i;

    QString result = "p cnf "
            + QString::number(HOLES*PEGS + RULES*MOVES) + " "
            + QString::number(HOLES*2 + (RULES*8 + HOLES*2 + sum + 1)*MOVES) + "\n";

    i = 1;
    while(i <= HOLES) {
        if(BOARD[i-1]) result += QString::number(i) + " 0\n";
        else           result += QString::number(-i) + " 0\n";
        ++i;
    }

    i = HOLES*MOVES + 1;
    while(i <= HOLES*MOVES + HOLES/2) {
        result += QString::number(-i) + " 0\n";
        ++i;
    }

    result += QString::number(i) + " 0\n";
    ++i;

    while(i <= HOLES*PEGS) {
        result += QString::number(-i) + " 0\n";
        ++i;
    }

    return result;
}

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
    pegs = 0;
    for(int i=0; i<HOLES; ++i) {
        board[i] = squares[i].getPegState();
        if(board[i]) ++pegs;
    }
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

QString solveSAT(const QString& CNF_PATH)
{
    QProcess p;
    QString result = "";

    QString kissat = qApp->applicationDirPath() + "/kissat/kissat";
    if(!QFileInfo::exists(kissat))
        kissat = qApp->applicationDirPath() + "/kissat/kissat.exe";
    const QStringList args = {"-q", "--sat", "--conflicts=1400000", CNF_PATH};

    if(QFileInfo::exists(kissat) && QFileInfo::exists(CNF_PATH)) {
        QFile f(kissat);
        if(f.open(QIODevice::ReadOnly)) {
            f.setPermissions(QFile::ExeGroup | QFile::ExeOther | QFile::ExeOther | QFile::ExeUser);
            f.close();
        }
        p.start(kissat, args);
        p.waitForFinished(-1);
        result += p.readAllStandardOutput();
        if(result.isEmpty()) result = "TIMEOUT";
        p.close();
    }
    else {
        result = "NO_SOLVER";
    }

    return result;
}

void Widget::solve()
{
    ui->pushButton->setEnabled(false);
    lockSquares();
    setBoardFromPegsState();

    const int PEGS = pegs;
    const int MOVES = PEGS-1;
    const int RULES = 38;
    const int possibleMoves[RULES][3] = {{1,2,3}, {1,4,9}, {2,5,10}, {3,6,11},
                                         {4,5,6}, {4,9,16}, {5,10,17}, {6,11,18},
                                         {7,8,9}, {7,14,21}, {8,9,10}, {8,15,22},
                                         {9,10,11}, {9,16,23}, {10,11,12}, {10,17,24},
                                         {11,12,13}, {11,18,25}, {12,19,26},
                                         {13,20,27}, {14,15,16}, {15,16,17},
                                         {16,17,18}, {16,23,28}, {17,18,19}, {17,24,29},
                                         {18,19,20}, {18,25,30}, {21,22,23}, {22,23,24},
                                         {23,24,25}, {23,28,31}, {24,25,26}, {24,29,32},
                                         {25,26,27}, {25,30,33}, {28,29,30}, {31,32,33}};
    QString variablesToSend = initializePegVariablesCenterHole(board, HOLES, PEGS, MOVES, RULES);
    QString rulesToSend = initializeRulesGlobal(possibleMoves, HOLES, PEGS, MOVES, RULES);

    QTemporaryFile cnf_file;
    if(cnf_file.open()) {
        QTextStream stream(&cnf_file);
        stream << variablesToSend << rulesToSend;
        cnf_file.close();
    }

    QRegularExpression rx("\\s+");
    QStringList list = solveSAT( cnf_file.fileName() ).split(rx, Qt::SkipEmptyParts);

    if(list[0] == "NO_SOLVER") {
        qDebug() << "NO_SOLVER";
    }
    else if(list[0] == "PERMISSION_DENIED") {
        qDebug() << "PERM_DEN";
    }
    else if(list[0] == "s" && list[1] == "UNSATISFIABLE") {
        qDebug() << "UNSAT";
    }
    else {
        qDebug() << "SAT";
    }

    ui->pushButton->setEnabled(true);
    unlockSquares();
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
