#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QResizeEvent>
#include "square.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT
public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
    const int HOLES;
    bool *board;
    Square *squares;

    void setBoardFromPegsState();
    void setPegsStateFromBoard();
    void paintSquares();
    void lockSquares();
    void unlockSquares();

private slots:
    void solve();

protected:
    virtual void resizeEvent(QResizeEvent*);
};
#endif // WIDGET_H
