#ifndef SOLVER_H
#define SOLVER_H

#include <QObject>
#include <QProcess>

class Solver : public QObject
{
public:
    Solver(bool** BOARDS, const int& HOLES, const int& PEGS, QObject* parent = nullptr);
    ~Solver();

    void run();
    bool getResult(bool** output_boards);

private:
    QProcess *process;
    QString variablesToSend, rulesToSend;

    bool** boards;
    int pegs, moves, holes, rules;
    int** possibleMoves;
    bool* finishingPattern;
    int finishingHoles;

    bool isSAT;

    void computeFinishingPattern(bool* pattern);
    QString initializePegVariables();
    QString initializeRulesGlobal();
};

#endif // SOLVER_H
