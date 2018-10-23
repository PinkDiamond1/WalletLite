#include "dynamicmove.h"
#include <QDebug>

DynamicMove::DynamicMove( QWidget *widget, QPoint desPos, int mSecs, QWidget *parent) :
    w(widget),
	destination(desPos),
	moveTime(mSecs),
    QObject(parent)
{
    initialPosition = w->pos();
}

DynamicMove::~DynamicMove()
{
	qDebug() << "DynamicMove::~DynamicMove()";
}

void DynamicMove::start()
{
	timer.setInterval(deltaTime);
	connect(&timer, SIGNAL(timeout()), this, SLOT(step()));
	timer.start();
}

void DynamicMove::step()
{
	costTime += deltaTime;

	if (costTime >= moveTime)
	{
		w->move(destination);
		timer.stop();
		emit moveEnd();
		deleteLater();
	}
	else
	{
		int x = initialPosition.x() + (destination.x() - initialPosition.x()) * 1.0f * float(costTime) / float(moveTime);
		int y = initialPosition.y() + (destination.y() - initialPosition.y()) * 1.0f * float(costTime) / float(moveTime);
		w->move(x, y);
	}
}



