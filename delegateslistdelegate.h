#ifndef DELEGATESLISTDELEGATE_H
#define DELEGATESLISTDELEGATE_H

#include <QtWidgets/QStyledItemDelegate>

#include <functional>

class DelegatesListDelegate : public QStyledItemDelegate
{
public:
	DelegatesListDelegate(QObject *parent);
	~DelegatesListDelegate();

public:
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

	void setCheckboxClickedCallback(const std::function<void(int)>& func);

private:
	std::function<void(int)> checkboxClicked = nullptr;
};

#endif // DELEGATESLISTDELEGATE_H