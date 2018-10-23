#include "delegateslistdelegate.h"

#include <QPainter>
#include <QMouseEvent>

#include "qdebug.h"
#include "datamgr.h"
#include "delegateslistmodel.h"

DelegatesListDelegate::DelegatesListDelegate(QObject *parent)
	: QStyledItemDelegate(parent)
{
}

DelegatesListDelegate::~DelegatesListDelegate()
{
	qDebug() << "Deconstruction DelegatesListDelegate.";
}

void DelegatesListDelegate::setCheckboxClickedCallback(const std::function<void(int)>& func)
{
	checkboxClicked = func;
}

QSize DelegatesListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	return QSize(700, 38);
}

void DelegatesListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	DelegateAccount* itemData = index.data(Qt::UserRole).value<DelegateAccount*>();
	int pageNum = index.data(Qt::DisplayPropertyRole).value<int>();

	painter->setFont(QFont("Microsoft Yahei", 12));
	painter->setPen(QColor(16, 16, 16));

	int posY = option.rect.y();
	int rectHeight = option.rect.height();

	QString orderIdx = QString::number((pageNum - 1) * 10 + index.row() + 1);
	painter->drawText(QRect(12, posY, 200, rectHeight), Qt::AlignLeft | Qt::AlignVCenter, orderIdx);

	painter->drawText(QRect(158, posY, 200, rectHeight), Qt::AlignLeft | Qt::AlignVCenter, itemData->name);

	QString votesNumStr = itemData->votes_num;
	if (votesNumStr.size() > 5)
		votesNumStr.insert(votesNumStr.size() - 5, ".");
	else
	{
		int zeroNum = 6 - votesNumStr.size();
		for (int i = 0; i < zeroNum + 1; i++)
		{
			if (i == zeroNum)
				votesNumStr.insert(1, ".");
			else
				votesNumStr.insert(0, "0");
		}
	}
	painter->drawText(QRect(340, posY, 200, rectHeight), Qt::AlignLeft | Qt::AlignVCenter, votesNumStr);

	QString imgPath = DataMgr::getDataMgr()->getWorkPath();
	bool isVoteDelegate = DataMgr::getDataMgr()->isVoteDelegate(itemData->id);
	if (isVoteDelegate)
		imgPath += "pic2/star_checked.png";
	else
		imgPath += "pic2/star_unchecked.png";

	QPixmap img(imgPath);
	itemData->checkboxRect = QRect(590, posY + 11, img.rect().width(), img.rect().height());
	painter->drawPixmap(itemData->checkboxRect, img);
}

bool DelegatesListDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, 
	const QStyleOptionViewItem &option, const QModelIndex &index)
{
	QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

	if (event->type() == QEvent::MouseButtonRelease)
	{
		DelegateAccount* itemData = index.data(Qt::UserRole).value<DelegateAccount*>();

		if (itemData->checkboxRect.contains(mouseEvent->pos()))
		{
			checkboxClicked(itemData->id);
		}
	}

	return QStyledItemDelegate::editorEvent(event, model, option, index);
}
