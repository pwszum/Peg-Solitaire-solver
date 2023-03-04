#ifndef SQUARE_H
#define SQUARE_H

#include <QWidget>

class Square : public QWidget
{
    Q_OBJECT
public:
    explicit Square(QWidget *parent = nullptr);
    void changePegState();
    void setPegState(const bool&);
    bool getPegState();

private:
    bool isPegPresent;

signals:

protected:
    virtual void paintEvent(QPaintEvent*);
    virtual void mousePressEvent(QMouseEvent*);
};

#endif // SQUARE_H
