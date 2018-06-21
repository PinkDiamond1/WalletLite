#ifndef QUOTATIONLISTDELEGATE_H
#define QUOTATIONLISTDELEGATE_H

#include <QtWidgets/QStyledItemDelegate>

class QuotationListDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	QuotationListDelegate(QObject *parent);
	~QuotationListDelegate();

public:
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const ;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // QUOTATIONLISTDELEGATE_H
