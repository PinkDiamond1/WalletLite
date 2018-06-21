#include "quotationlistdelegate.h"

#include <QPainter>

#include "quotationlistmodel.h"
#include "misc.h"
#include "datamgr.h"
#include "thirddatamgr.h"

QFont list_font("Microsoft Yahei", 10);

QuotationListDelegate::QuotationListDelegate(QObject *parent)
	: QStyledItemDelegate(parent)
{
}

QuotationListDelegate::~QuotationListDelegate()
{
	qDebug() << "Deconstruction QuotationListDelegate.";
}

QSize QuotationListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	return QSize(822, 32);
}

QString quotationDoubleToStr(double number, int prec=6)
{
	QString num = QString::number(number, 'f', prec);
	int pos = num.indexOf('.') + prec;
	QString str = num.mid(0, pos);

	for (int i = str.length() - 1; i > 0; i--)
	{
		if (str[i] == QChar('0'))
		{
			str = str.left(str.length() - 1);
		}
		else
		{
			if (str[i] == QChar('.'))
			{
				str = str.left(str.length() - 1);
			}
			break;
		}
	}

	if (str == "0" && prec <= 8)
	{
		str = quotationDoubleToStr(number, ++prec);
	}

	return str;
}

void QuotationListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QuotationItemData* itemData = index.data(Qt::UserRole).value<QuotationItemData*>();
	QString currencySymbol =  ThirdDataMgr::getInstance()->getCurrentCurrencySymbol();

	painter->setPen(Qt::black);
	painter->setFont(list_font);
	painter->setRenderHint(QPainter::Antialiasing);

	// Fill background.
	painter->fillRect(QRect(54, option.rect.y(), 686, option.rect.height()), Qt::white);

	//sort num
	painter->drawText(QRect(0, option.rect.y(), 100, option.rect.height()), Qt::AlignLeft | Qt::AlignVCenter, QString::number(index.row() + 1));

	//asset name
	painter->drawText(QRect(100, option.rect.y(), 160, option.rect.height()), Qt::AlignLeft | Qt::AlignVCenter, itemData->asset_name);

	//price
	painter->drawText(QRect(277, option.rect.y(), 160, option.rect.height()), Qt::AlignLeft | Qt::AlignVCenter, doubleToStr(itemData->price) + currencySymbol);

	//percent change
    QString percentChangeStr;
    QString priceChangeStr;

    if (itemData->percent_change == 0)
    {
        painter->setPen(QColor(60, 60, 60));
        percentChangeStr = "0%";
        priceChangeStr = "0" + currencySymbol;
    }
    else if (itemData->percent_change > 0)
    {
        painter->setPen(QColor(0, 153, 51));
        percentChangeStr = "+" + doubleToStr(itemData->percent_change) + "%";
        priceChangeStr = "+" + quotationDoubleToStr(itemData->price_change) + currencySymbol;
    }
	else
    {
        painter->setPen(QColor(221, 112, 54));
        percentChangeStr = doubleToStr(itemData->percent_change) + "%";
        priceChangeStr = quotationDoubleToStr(itemData->price_change) + currencySymbol;
    }

    painter->drawText(QRect(463, option.rect.y(), 160, option.rect.height()), Qt::AlignLeft | Qt::AlignVCenter, percentChangeStr);
    painter->drawText(QRect(626, option.rect.y(), 160, option.rect.height()), Qt::AlignLeft | Qt::AlignVCenter, priceChangeStr);

	painter->setPen(QColor(235, 235, 235));
	painter->drawLine(QLineF(QPointF(-13, option.rect.y() + 31), QPointF(770, option.rect.y() + 31)));
}
