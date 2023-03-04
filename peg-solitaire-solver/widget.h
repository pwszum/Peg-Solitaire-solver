#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QResizeEvent>
#include "square.h"
#include "solver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT
public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    virtual void resizeEvent(QResizeEvent*);

private:
    Ui::Widget *ui;
    Solver *solver;
    const int HOLES;
    bool *board;
    Square *squares;

    void setBoardFromPegsState();
    void setPegsStateFromBoard();
    void addSquaresToGrid();
    void lockSquares();
    void unlockSquares();

private slots:
    void solve();

};
#endif // WIDGET_H
