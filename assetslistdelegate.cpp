#include "assetslistdelegate.h"

#include <QPainter>

#include "assetslistmodel.h"
#include "misc.h"
#include "datamgr.h"
#include "thirddatamgr.h"

const QColor color_font(16, 16, 16);
const QColor color_gray_normal(240, 240, 240);
const QColor color_gray_head(210, 210, 210);

QFont first_level_font("Microsoft Yahei", 12);
QFont second_level_font("Microsoft Yahei", 11);

AssetsListDelegate::AssetsListDelegate(QObject *parent)
	: QStyledItemDelegate(parent)
{
}

AssetsListDelegate::~AssetsListDelegate()
{
	qDebug() << "Deconstruction AssetsListDelegate.";
}

QSize AssetsListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	AssetItemData* itemData = index.data(Qt::UserRole).value<AssetItemData*>();
	if (itemData->level == 1)
		return QSize(822, 50);
	else if (itemData->level == 2)
		return QSize(822, 32);
}

void AssetsListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	AssetItemData* itemData = index.data(Qt::UserRole).value<AssetItemData*>();
	double totalPrice = index.data(Qt::DisplayPropertyRole).value<double>();

	painter->setRenderHint(QPainter::Antialiasing);

	int posY = option.rect.y();
	int rectHeight = option.rect.height();
	if (itemData->level == 1)
	{
		posY += 5;
		rectHeight -= 5;
	}

	// Fill background.
	if (itemData->level == 1)
	{
		if (itemData->collapse)
			painter->fillRect(option.rect, Qt::white);
		else
			painter->fillRect(QRect(54, posY, 716, rectHeight), color_gray_normal);
	}
	else if (itemData->level == 2)
	{
		painter->fillRect(QRect(54, posY, 716, rectHeight), color_gray_normal);
		painter->fillRect(QRect(110, posY, 2, rectHeight), color_gray_head);
	}

	if (itemData->level == 1)
	{
		painter->setFont(first_level_font);
		painter->setBrush(color_font);
		painter->drawText(QRect(110, posY, 200, rectHeight), Qt::AlignLeft | Qt::AlignVCenter, itemData->asset_name);
	}
	else if(itemData->level == 2)
	{
		painter->setFont(second_level_font);
		painter->setBrush(color_font);
		painter->drawText(QRect(120, posY, 200, rectHeight), Qt::AlignLeft | Qt::AlignVCenter, itemData->account_name);
	}


	painter->drawText(QRect(232, posY, 200, rectHeight), Qt::AlignLeft | Qt::AlignVCenter, doubleToStr(itemData->balance));


	if (itemData->price >= 0)
	{
		auto priceStr = QString::number(itemData->price, 'f', 8);
		painter->drawText(QRect(395, posY, 200, rectHeight), Qt::AlignLeft | Qt::AlignVCenter, priceStr);
	}
	else
	{
		painter->drawText(QRect(395, posY, 200, rectHeight), Qt::AlignLeft | Qt::AlignVCenter, "--");
	}

	if (itemData->level == 1)
	{
		painter->setFont(first_level_font);
		painter->setBrush(color_font);

		if (itemData->price > 0)
		{
			if (itemData->price / totalPrice >= 0.000001)
			{
				const QString& rateStr = QString::number((itemData->price / totalPrice)*100.f, 'f', 4);
				painter->drawText(QRect(592, option.rect.y() + 5, 150, rectHeight), Qt::AlignLeft | Qt::AlignVCenter, rateStr + "%");
			}
			else
			{
				painter->drawText(QRect(592, option.rect.y(), 150, rectHeight), Qt::AlignLeft | Qt::AlignVCenter, tr("less than 0.0001%"));
			}
		}
		else
		{
			painter->drawText(QRect(592, option.rect.y(), 100, rectHeight), Qt::AlignLeft | Qt::AlignVCenter, "--");
		}
	}
	
	if (itemData->level == 1 && itemData->children.size() != 0)
	{
		QString imgPath;
		if (itemData->collapse)
			imgPath = DataMgr::getDataMgr()->getWorkPath() + "pic2/expand_normal.png";
		else
			imgPath = DataMgr::getDataMgr()->getWorkPath() + "pic2/unexpand_normal.png";

		QPixmap img(imgPath);
		int posY = option.rect.y() + (rectHeight - img.rect().height()) / 2 + 7;
		painter->drawPixmap(QRect(698 + 34, posY, img.rect().width(), img.rect().height()), img);
	}
}
