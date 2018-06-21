#ifndef ASSETSLISTDELEGATE_H
#define ASSETSLISTDELEGATE_H

#include <QtWidgets/QStyledItemDelegate>

class AssetsListDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	AssetsListDelegate(QObject *parent);
	~AssetsListDelegate();

public:
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const ;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // ASSETSLISTDELEGATE_H
