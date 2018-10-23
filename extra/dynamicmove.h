#ifndef DYNAMICMOVE_H
#define DYNAMICMOVE_H

#include <QWidget>
#include <QTimer>
#include <QTime>

class DynamicMove : public QObject
{
    Q_OBJECT
public:
	DynamicMove(QWidget* widget, QPoint desPos, int intervalSecs, QWidget* parent = 0);
    ~DynamicMove();

    void start();

private slots:
    void step();

signals:
    void moveEnd();

private:
    QWidget* w;
    QTimer timer;
    int moveTime = 0;
	int deltaTime = 20;
    QPoint initialPosition;
    QPoint destination;
    int costTime = 0;
};

#endif // DYNAMICMOVE_H
