
#include "homepagescrollarea.h"

#include <QWheelEvent>

#include <qdebug.h>

HomePageScrollArea::HomePageScrollArea(QWidget *parent)
	: QScrollArea(parent)
{
}

HomePageScrollArea::~HomePageScrollArea()
{
}

bool HomePageScrollArea::eventFilter(QObject *watched, QEvent *e)
{
	if (e->type() == QEvent::Move)
	{
		int widgetY = -widget()->y();
		int widgetMaxY = widget()->height() - height();

		if (lastWidgetY == 0 && widgetY > 0)
		{
			emit scrollMoveEvent(MoveEvent::LEAVE_TOP);
			lastMoveEvent = MoveEvent::LEAVE_TOP;

			if (widgetY == widgetMaxY)
			{
				emit scrollMoveEvent(MoveEvent::REACH_BOTTOM);
				lastMoveEvent = MoveEvent::REACH_BOTTOM;
			}
		}
		else if (lastWidgetY < widgetMaxY && widgetY == widgetMaxY)
		{
			emit scrollMoveEvent(MoveEvent::REACH_BOTTOM);
			lastMoveEvent = MoveEvent::REACH_BOTTOM;
		}
		else if (lastWidgetY == widgetMaxY && widgetY < widgetMaxY)
		{
			emit scrollMoveEvent(MoveEvent::LEAVE_BOTTOM);
			lastMoveEvent = MoveEvent::LEAVE_BOTTOM;

			if (widgetY == 0)
			{
				emit scrollMoveEvent(MoveEvent::REACH_TOP);
				lastMoveEvent = MoveEvent::REACH_TOP;
			}
		}
		else if (lastWidgetY > 0 && widgetY == 0)
		{
			emit scrollMoveEvent(MoveEvent::REACH_TOP);
			lastMoveEvent = MoveEvent::REACH_TOP;
		}

		lastWidgetY = widgetY;
		lastWidgetMaxY = widgetMaxY;
	}
	else if (e->type() == QEvent::Resize)
	{
		int widgetY = -widget()->y();
		int widgetMaxY = widget()->height() - height();

		if (lastMoveEvent == MoveEvent::REACH_BOTTOM)
		{
			if (widgetY < widgetMaxY)
			{
				emit scrollMoveEvent(MoveEvent::LEAVE_BOTTOM);
				lastMoveEvent = MoveEvent::LEAVE_BOTTOM;
			}
		}
		else
		{
			if (lastWidgetY >= widgetMaxY)
			{
				emit scrollMoveEvent(MoveEvent::REACH_BOTTOM);
				lastMoveEvent = MoveEvent::REACH_BOTTOM;
			}
		}

		lastWidgetY = widgetY;
		lastWidgetMaxY = widgetMaxY;
	}

	return QScrollArea::eventFilter(watched, e);
}
