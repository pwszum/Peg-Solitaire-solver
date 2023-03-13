#include "solver.h"

#include <QDebug>

#include <QApplication>
#include <QProcess>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <QTextStream>
#include <QFileInfo>

//#include <algorithm>
//#include <random>
//#include <chrono>

Solver::Solver(bool** BOARDS, const int& HOLES, const int& PEGS, QObject* parent) : QObject(parent)
{
    pegs=PEGS;
    isSAT = false;

    if(pegs==0)
        return;

    boards = new bool*[pegs];
    for(int i=0; i<pegs; ++i)
        boards[i] = new bool[HOLES];
    for(int i=0; i<HOLES; ++i)
        boards[0][i] = BOARDS[0][i];

    moves = pegs-1;
    holes = HOLES;
    rules = 38;

    const int POSSIBLE_MOVES[][3] = {{1,2,3}, {1,4,9}, {2,5,10}, {3,6,11},
                                     {4,5,6}, {4,9,16}, {5,10,17}, {6,11,18},
                                     {7,8,9}, {7,14,21}, {8,9,10}, {8,15,22},
                                     {9,10,11}, {9,16,23}, {10,11,12}, {10,17,24},
                                     {11,12,13}, {11,18,25}, {12,19,26},
                                     {13,20,27}, {14,15,16}, {15,16,17},
                                     {16,17,18}, {16,23,28}, {17,18,19}, {17,24,29},
                                     {18,19,20}, {18,25,30}, {21,22,23}, {22,23,24},
                                     {23,24,25}, {23,28,31}, {24,25,26}, {24,29,32},
                                     {25,26,27}, {25,30,33}, {28,29,30}, {31,32,33}};
    possibleMoves = new int*[rules];
    for(int i=0; i<rules; ++i) {
        possibleMoves[i] = new int[3];
        for(int j=0; j<3; ++j) {
            possibleMoves[i][j] = POSSIBLE_MOVES[i][j];
        }
    }

    finishingPattern = new bool[holes]();
    computeFinishingPattern(finishingPattern);

    process = new QProcess();
    variablesToSend = initializePegVariablesCenterHole();
    rulesToSend = initializeRulesGlobal();
}

Solver::~Solver()
{
    if(pegs==0)
        return;

    for(int i=0; i<pegs; ++i) {
        delete[] boards[i];
    }
    delete[] boards;

    for(int i=0; i<rules; ++i) {
        delete[] possibleMoves[i];
    }
    delete[] possibleMoves;

    delete[] finishingPattern;

    delete process;
}

void Solver::run()
{
    if(pegs==0 || finishingHoles==0) {
        qDebug() << "UNSAT";
        return;
    }

    QTemporaryFile cnf_file;
    if(cnf_file.open()) {
        QTextStream stream(&cnf_file);
        stream << variablesToSend << rulesToSend;
        cnf_file.close();
    }

    QStringList resultList;

    QString kissatOutput = "";
    const QStringList args = {"-q", "--sat", cnf_file.fileName()};
    QString kissat = qApp->applicationDirPath() + "/kissat/kissat";
    if(!QFileInfo::exists(kissat))
        kissat = qApp->applicationDirPath() + "/kissat/kissat.exe";

    if(QFileInfo::exists(kissat) && QFileInfo::exists(cnf_file.fileName())) {
        QFile f(kissat);
        if(f.open(QIODevice::ReadOnly)) {
            f.setPermissions(QFile::ExeGroup | QFile::ExeOther | QFile::ExeOther | QFile::ExeUser);
            f.close();
        }
        process->start(kissat, args);
        process->waitForFinished(-1);
        kissatOutput += process->readAllStandardOutput();
        if(kissatOutput.isEmpty()) kissatOutput = "ERROR";
        process->close();
    }
    else {
        kissatOutput = "NO_SOLVER";
    }

    resultList = kissatOutput.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    if(resultList[0] == "NO_SOLVER") {
        qDebug() << "NO_SOLVER";
    }
    else if(resultList[0] == "ERROR") {
        qDebug() << "ERROR";
    }
    else if(resultList[0] == "s" && resultList[1] == "UNSATISFIABLE") {
        qDebug() << "UNSAT";
    }
    else {
        isSAT = true;
        qDebug() << "SAT";

        int counter=3;
        for(int i=0; i<pegs; ++i) {
            for(int j=0; j<holes; ++j) {
                if(resultList[counter] == "v")
                    ++counter;
                if(resultList[counter].toInt() < 0)
                    boards[i][j] = false;
                else
                    boards[i][j] = true;
                ++counter;
            }
        }

        QDebug deb = qDebug();
        deb << "\n    ";
        for(int i=0; i<3; ++i) {
            if(boards[pegs-1][i]) deb << "o";
            else           deb << ".";
        } deb << "\n";
        deb << "   ";
        for(int i=0; i<3; ++i) {
            if(boards[pegs-1][i+3]) deb << "o";
            else           deb << ".";
        } deb << "\n";
        for(int i=0; i<7; ++i) {
            if(boards[pegs-1][i+6]) deb << "o";
            else           deb << ".";
        } deb << "\n";
        for(int i=0; i<7; ++i) {
            if(boards[pegs-1][i+13]) deb << "o";
            else           deb << ".";
        } deb << "\n";
        for(int i=0; i<7; ++i) {
            if(boards[pegs-1][i+20]) deb << "o";
            else           deb << ".";
        } deb << "\n";
        deb << "   ";
        for(int i=0; i<3; ++i) {
            if(boards[pegs-1][i+27]) deb << "o";
            else           deb << ".";
        } deb << "\n";
        deb << "   ";
        for(int i=0; i<3; ++i) {
            if(boards[pegs-1][i+30]) deb << "o";
            else           deb << ".";
        } deb << "\n";
    }
}

bool Solver::getResult(bool **output_boards)
{
    if(pegs==0 || finishingHoles==0 || !isSAT) {
        qDebug() << "UNSAT";
        for(int i=0; i<holes; ++i) {
            output_boards[1][i] = false;
        }
        return false;
    }

    for(int i=1; i<=moves; ++i) {
        for(int j=0; j<holes; ++j) {
            output_boards[i][j] = boards[i][j];
        }
    }
    return true;
}

void Solver::computeFinishingPattern(bool* pattern)
{
    const int diagonalLabels[][holes] =
    {
             {2, 0, 1,
              0, 1, 2,
        2, 0, 1, 2, 0, 1, 2,
        0, 1, 2, 0, 1, 2, 0,
        1, 2, 0, 1, 2, 0, 1,
              1, 2, 0,
              2, 0, 1},

             {4, 3, 5,
              5, 4, 3,
        5, 4, 3, 5, 4, 3, 5,
        3, 5, 4, 3, 5, 4, 3,
        4, 3, 5, 4, 3, 5, 4,
              3, 5, 4,
              4, 3, 5}
    };

    int n0=0, n1=0, n2=0, n3=0, n4=0, n5=0;
    for(int i=0; i<holes; ++i) {
        if(boards[0][i]) {
            switch(diagonalLabels[0][i]) {
                case 0: ++n0; break;
                case 1: ++n1; break;
                case 2: ++n2; break;
            }
            switch(diagonalLabels[1][i]) {
                case 3: ++n3; break;
                case 4: ++n4; break;
                case 5: ++n5; break;
            }
        }
    }
    const int vecN = (n1+n2)%2*32 + (n0+n2)%2*16 + (n0+n1)%2*8 + (n4+n5)%2*4 + (n3+n5)%2*2 + (n3+n4)%2;

    switch(vecN) {
//        case 0b110101:
//            pattern[0] = true; pattern[15] = true; pattern[18] = true; pattern[30] = true;
//            finishingHoles = 4;
//            break;
        case 0b011011:
            pattern[1] = true; pattern[13] = true; pattern[16] = true; pattern[19] = true; pattern[31] = true;
            finishingHoles = 5;
            break;
//        case 0b101110:
//            pattern[2] = true; pattern[14] = true; pattern[17] = true; pattern[32] = true;
//            finishingHoles = 4;
//            break;
//        case 0b011110:
//            pattern[3] = true; pattern[22] = true; pattern[25] = true;
//            finishingHoles = 3;
//            break;
//        case 0b101101:
//            pattern[4] = true; pattern[20] = true; pattern[23] = true; pattern[26] = true;
//            finishingHoles = 4;
//            break;
//        case 0b110011:
//            pattern[5] = true; pattern[21] = true; pattern[24] = true;
//            finishingHoles = 3;
//            break;
//        case 0b110110:
//            pattern[6] = true; pattern[9]  = true; pattern[12] = true; pattern[28] = true;
//            finishingHoles = 4;
//            break;
//        case 0b011101:
//            pattern[7] = true; pattern[10] = true; pattern[29] = true;
//            finishingHoles = 3;
//            break;
//        case 0b101011:
//            pattern[8] = true; pattern[11] = true; pattern[27] = true;
//            finishingHoles = 3;
//            break;
        default:
            finishingHoles = 0;
            break;
    }

    QDebug deb = qDebug();
    deb << "    ";
    for(int i=0; i<3; ++i) {
        if(pattern[i]) deb << "o";
        else           deb << ".";
    } deb << "\n";
    deb << "   ";
    for(int i=0; i<3; ++i) {
        if(pattern[i+3]) deb << "o";
        else           deb << ".";
    } deb << "\n";
    for(int i=0; i<7; ++i) {
        if(pattern[i+6]) deb << "o";
        else           deb << ".";
    } deb << "\n";
    for(int i=0; i<7; ++i) {
        if(pattern[i+13]) deb << "o";
        else           deb << ".";
    } deb << "\n";
    for(int i=0; i<7; ++i) {
        if(pattern[i+20]) deb << "o";
        else           deb << ".";
    } deb << "\n";
    deb << "   ";
    for(int i=0; i<3; ++i) {
        if(pattern[i+27]) deb << "o";
        else           deb << ".";
    } deb << "\n";
    deb << "   ";
    for(int i=0; i<3; ++i) {
        if(pattern[i+30]) deb << "o";
        else           deb << ".";
    } deb << "\n";
    deb << "   (" << Qt::bin << vecN << ")\n";
}

QString Solver::initializePegVariablesCenterHole()
{
    int i=0;
    int sum=0;
    if(finishingHoles != 5) return "";

    for(int i=1; i<rules; ++i) sum += i;

    QString result = "p cnf "
            + QString::number(holes*pegs + rules*moves) + " "
            + QString::number(holes*2 + (rules*8 + holes*2 + sum + 1)*moves) + "\n";

    i = 1;
    while(i <= holes) {
        if(boards[0][i-1]) result += QString::number(i) + " 0\n";
        else           result += QString::number(-i) + " 0\n";
        ++i;
    }

    i = holes*moves + 1;
    while(i <= holes*moves + holes/2) {
        result += QString::number(-i) + " 0\n";
        ++i;
    }

    result += QString::number(i) + " 0\n";
    ++i;

    while(i <= holes*pegs) {
        result += QString::number(-i) + " 0\n";
        ++i;
    }

    return result;
}

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

QString Solver::initializeRulesGlobal()
{
    if(finishingHoles==0) return "";

    std::vector<int> firstTransition;

    for(int i=1; i<=holes; ++i) {
        std::vector<int> tmp;
        for(int j=0; j<rules; ++j) {
            if(possibleMoves[j][0] > i) break;
            if(possibleMoves[j][0] == i) {
                tmp.push_back(holes*pegs + j + 1);
                continue;
            }
            if(possibleMoves[j][1] > i) continue;
            if(possibleMoves[j][1] == i) {
                tmp.push_back(holes*pegs + j + 1);
                continue;
            }
            if(possibleMoves[j][2] > i) continue;
            if(possibleMoves[j][2] == i) {
                tmp.push_back(holes*pegs + j + 1);
            }
        }

        firstTransition.push_back(-i);
        firstTransition.push_back(i+holes);
        firstTransition.insert(firstTransition.end(), tmp.begin(), tmp.end());
        firstTransition.push_back(0);

        firstTransition.push_back(i);
        firstTransition.push_back(-i-holes);
        firstTransition.insert(firstTransition.end(), tmp.begin(), tmp.end());
        firstTransition.push_back(0);
    }


    QString result = "";

    for(int m=0; m<moves; ++m) {
        QString loopResult = "";

        for(int i=0; i<rules; ++i) {
            loopResult += QString::number(holes*pegs + m*rules + i + 1) + " ";
            result += getRuleForMoveXYZ(holes*pegs + m*rules + i + 1, possibleMoves[i], m*holes, holes);

            for(int j=0; j<i; ++j) {
                result += QString::number(-(holes*pegs + m*rules + i + 1)) + " " + QString::number(-(holes*pegs + m*rules + j + 1)) + " 0\n";
            }
        }
        loopResult += "0\n";

        int size = firstTransition.size();
        int i=0;
        while(i < size) {
            if(firstTransition[i] < 0) {
                result += QString::number(firstTransition[i] - m*holes) + " ";
                ++i;
                result += QString::number(firstTransition[i] + m*holes) + " ";
            }
            else {
                result += QString::number(firstTransition[i] + m*holes) + " ";
                ++i;
                result += QString::number(firstTransition[i] - m*holes) + " ";
            }

            while(firstTransition[++i]) {
                result += QString::number(firstTransition[i] + m*rules) + " ";

            }
            result += "0\n";

            ++i;
        }
        result += loopResult;
    }

    return result;
}
