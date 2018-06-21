
#include "assetslistview.h"

AssetsListView::AssetsListView(QWidget *parent)
	: QListView(parent)
{
	setStyleSheet(QString(
		"QListView{background-color:rgb(255, 255, 255);"
		"border:0px solid rgb(255, 255, 255);"
		"border-right-width:1px;}"));
}

AssetsListView::~AssetsListView()
{
}
