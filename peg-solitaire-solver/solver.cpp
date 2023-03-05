#include "solver.h"

#include <QDebug>

#include <QApplication>
#include <QProcess>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <QTextStream>
#include <QFileInfo>

Solver::Solver(const bool* BOARD, const int& HOLES, QObject* parent) : QObject(parent)
{
    pegs=0;
    for(int i=0; i<HOLES; ++i)
        if(BOARD[i]) ++pegs;

    boards = new bool*[pegs];
    for(int i=0; i<pegs; ++i)
        boards[i] = new bool[HOLES];
    for(int i=0; i<HOLES; ++i)
        boards[0][i] = BOARD[i];

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

    process = new QProcess();
    variablesToSend = initializePegVariablesCenterHole();
    rulesToSend = initializeRulesGlobal();
}

Solver::~Solver()
{
    for(int i=0; i<pegs; ++i) {
        delete[] boards[i];
    }
    delete[] boards;

    for(int i=0; i<rules; ++i) {
        delete[] possibleMoves[i];
    }
    delete[] possibleMoves;

    delete process;
}

void runSatSolver(QProcess* process, const QString& CNF_PATH, QStringList& result)
{
    QString kissatOutput = "";

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
        process->start(kissat, args);
        process->waitForFinished(-1);
        kissatOutput += process->readAllStandardOutput();
        if(kissatOutput.isEmpty()) kissatOutput = "TIMEOUT";
        process->close();
    }
    else {
        kissatOutput = "NO_SOLVER";
    }

    result = kissatOutput.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
}

void Solver::run()
{
    QTemporaryFile cnf_file;
    if(cnf_file.open()) {
        QTextStream stream(&cnf_file);
        stream << variablesToSend << rulesToSend;
        cnf_file.close();
    }

    QStringList resultList;
    runSatSolver(process, cnf_file.fileName(), resultList);

    if(resultList[0] == "NO_SOLVER") {
        qDebug() << "NO_SOLVER";
    }
    else if(resultList[0] == "TIMEOUT") {
        qDebug() << "TIMEOUT";
    }
    else if(resultList[0] == "s" && resultList[1] == "UNSATISFIABLE") {
        qDebug() << "UNSAT";
    }
    else {
        qDebug() << "SAT";
    }
}

QString Solver::initializePegVariablesCenterHole()
{
    int i=0;
    int sum=0;
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

    result += QString::number(-ID) + " " + QString::number(ARR[0]+MOVE) + " " + QString::number(ARR[2]+MOVE) + " 0\n";
    result += QString::number(-ID) + " " + QString::number(-ARR[0]-MOVE) + " " + QString::number(-ARR[2]-MOVE) + " 0\n";

    result += QString::number(-ID) + " " + QString::number(ARR[1]+MOVE) + " 0\n";
    result += QString::number(-ID) + " " + QString::number(-ARR[1]-MOVE-HOLES) + " 0\n";

    result += QString::number(-ID) + " " + QString::number(ARR[2]+MOVE) + " " + QString::number(ARR[2]+MOVE+HOLES) + " 0\n";
    result += QString::number(-ID) + " " + QString::number(-ARR[2]-MOVE) + " " + QString::number(-ARR[2]-MOVE-HOLES) + " 0\n";
    result += QString::number(-ID) + " " + QString::number(ARR[0]+MOVE) + " " + QString::number(ARR[0]+MOVE+HOLES) + " 0\n";
    result += QString::number(-ID) + " " + QString::number(-ARR[0]-MOVE) + " " + QString::number(-ARR[0]-MOVE-HOLES) + " 0\n";

    return result;
}

QString Solver::initializeRulesGlobal()
{
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
