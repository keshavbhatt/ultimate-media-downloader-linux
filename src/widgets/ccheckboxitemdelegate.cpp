/* */
#include "ccheckboxitemdelegate.h"

#include <QPainter>
//#include <QTextDocument>
//#include <QAbstractTextDocumentLayout>
#include <QFont>


void cCheckBoxItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
	QStyleOptionViewItem options = option;
	initStyleOption(&options, index);

	if(index.data(Qt::CheckStateRole) == Qt::Checked)
		options.font.setBold(true);
	QStyledItemDelegate::paint(painter, options, index);
}

QSize cCheckBoxItemDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
	QStyleOptionViewItem options = option;
	initStyleOption(&options, index);

	//if(index.data(Qt::CheckStateRole) == Qt::Checked)
		options.font.setBold(true);

	return(QStyledItemDelegate::sizeHint(options, index));
}
