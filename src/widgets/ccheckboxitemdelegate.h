#ifndef CCHECKBOXITEMDELEGATE_H
#define CCHECKBOXITEMDELEGATE_H


#include <QStyledItemDelegate>


/*!
 \brief

 \class cCheckBoxItemDelegate ccheckboxitemdelegate.h "ccheckboxitemdelegate.h"
*/
class cCheckBoxItemDelegate : public QStyledItemDelegate
{
public:
protected:
	/*!
	 \brief

	 \fn paint
	 \param painter
	 \param option
	 \param index
	*/
	void			paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	/*!
	 \brief

	 \fn sizeHint
	 \param option
	 \param index
	 \return QSize
	*/
	QSize			sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // CCHECKBOXITEMDELEGATE_H
