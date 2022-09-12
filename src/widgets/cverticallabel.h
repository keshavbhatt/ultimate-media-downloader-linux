#ifndef CVERTICALLABEL_H
#define CVERTICALLABEL_H


#include <QLabel>


/*!
 \brief

 \class cVerticalLabel cverticallabel.h "cverticallabel.h"
*/
class cVerticalLabel : public QLabel
{
	Q_OBJECT
public:
	/*!
	 \brief

	 \fn cVerticalLabel
	 \param parent
	*/
	explicit	cVerticalLabel(QWidget *parent = 0);
	/*!
	 \brief

	 \fn cVerticalLabel
	 \param text
	 \param parent
	*/
	explicit	cVerticalLabel(const QString &text, QWidget *parent=0);

signals:

protected:
	/*!
	 \brief

	 \fn paintEvent
	 \param
	*/
	void		paintEvent(QPaintEvent*);
	/*!
	 \brief

	 \fn sizeHint
	 \return QSize
	*/
	QSize		sizeHint() const ;
	/*!
	 \brief

	 \fn minimumSizeHint
	 \return QSize
	*/
	QSize		minimumSizeHint() const;

public slots:

private:
};

#endif // CVERTICALLABEL_H
