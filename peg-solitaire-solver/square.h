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

protected:
    virtual void paintEvent(QPaintEvent*);
    virtual void mousePressEvent(QMouseEvent*);

private:
    bool isPegPresent;

signals:


};

#endif // SQUARE_H
