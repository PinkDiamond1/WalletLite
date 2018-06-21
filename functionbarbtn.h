#ifndef FUNCTIONBARBTN_H
#define FUNCTIONBARBTN_H

#include <QToolButton>
#include <QLabel>


class FunctionBarBtn : public QToolButton
{
public:
	explicit FunctionBarBtn(QWidget* parent = nullptr);
	virtual ~FunctionBarBtn();

	void setPixmap(const QString& fileName);
	void setBtnText(const QString& text);
	void setSelected(bool state);

protected:
	virtual void paintEvent(QPaintEvent* e) override;

private:
	QPixmap* m_pixmap = nullptr;
	QLabel* label = nullptr;
	QString btnText;
	bool selected = false;
};

#endif // FUNCTIONBARBTN_H