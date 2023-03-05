#ifndef SOLVER_H
#define SOLVER_H

#include <QObject>
#include <QProcess>

class Solver : public QObject
{
public:
    Solver(const bool* BOARD, const int& HOLES, const int& PEGS, QObject* parent = nullptr);
    ~Solver();

    void run();
    void getResultOrSomething();

private:
    QProcess *process;
    QString variablesToSend, rulesToSend;

    bool** boards;
    int pegs, moves, holes, rules;
    int** possibleMoves;

    QString initializePegVariablesCenterHole();
    QString initializeRulesGlobal();
};

#endif // SOLVER_H
