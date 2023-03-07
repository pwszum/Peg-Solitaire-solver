#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QKeyEvent>
#include "square.h"
#include "solver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT
public:
    Widget(QWidget* parent = nullptr);
    ~Widget();

protected:
    virtual void resizeEvent(QResizeEvent*);
    virtual void keyPressEvent(QKeyEvent*);

private:
    Ui::Widget* ui;
    Solver* solver;
    const int HOLES;
    bool** boards;
    int selectedBoard;
    Square* squares;
    int pegs;

    void setCurrentBoardFromSquaresState(const bool& updatePegsN = true);
    void setSquaresStateFromCurrentBoard(const bool& updatePegsN = true);
    void addSquaresToGrid();
    void lockSquares();
    void unlockSquares();
    void changeSquaresToNextBoard();
    void changeSquaresToPrevBoard();

private slots:
    void solve();
    void reset();

signals:
    void backPressed();
    void forwardPressed();

};
#endif // WIDGET_H
