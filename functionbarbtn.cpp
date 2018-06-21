#include "functionbarbtn.h"

#include <QPainter>

FunctionBarBtn::FunctionBarBtn(QWidget* parent) :
QToolButton(parent)
{
	m_pixmap = new QPixmap();
}

FunctionBarBtn::~FunctionBarBtn()
{
	if (m_pixmap != nullptr)
	{
		delete m_pixmap;
	}
}

void FunctionBarBtn::setPixmap(const QString& fileName)
{
	m_pixmap->load(fileName);
}

void FunctionBarBtn::setBtnText(const QString& text)
{
	btnText = text;
}

void FunctionBarBtn::setSelected(bool state)
{
	selected = state;
	repaint();
}

void FunctionBarBtn::paintEvent(QPaintEvent* e)
{
	QPainter painter(this);

	if (selected)
		painter.fillRect(QRect(2, 2, 130, 84), QColor(228, 228, 228));
	else
		painter.fillRect(QRect(2, 2, 130, 84), QColor(255, 255, 255));

	painter.drawPixmap(30, 32, 24, 24, *m_pixmap);

	painter.setFont(QFont("Microsoft Yahei", 10));
	painter.setBrush(QColor(16, 16, 16));
	painter.drawText(QRect(60, 36, 100, 20), Qt::AlignLeft, btnText);

}