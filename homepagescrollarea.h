#ifndef HOMEPAGESCROLLAREA_H
#define HOMEPAGESCROLLAREA_H

#include <QScrollArea>

class HomePageScrollArea : public QScrollArea
{
	Q_OBJECT

public:
	enum MoveEvent
	{
		NONE = 0,
		LEAVE_TOP = 1,
		REACH_TOP = 2,
		LEAVE_BOTTOM = 3,
		REACH_BOTTOM = 4
	};

public:
	HomePageScrollArea(QWidget *parent);
	~HomePageScrollArea();

protected:
	bool eventFilter(QObject *watched, QEvent *e);

signals:
	void scrollMoveEvent(MoveEvent);

private:
	int lastWidgetY = 0;
	int lastWidgetMaxY = 0;
	MoveEvent lastMoveEvent = MoveEvent::NONE;
};

#endif // HOMEPAGESCROLLAREA_H
