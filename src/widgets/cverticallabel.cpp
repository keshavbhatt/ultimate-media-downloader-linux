#include "cverticallabel.h"

#include <QDebug>
#include <QPainter>


cVerticalLabel::cVerticalLabel(QWidget *parent)
	: QLabel(parent)
{
}


cVerticalLabel::cVerticalLabel(const QString &text, QWidget *parent)
	: QLabel(text, parent)
{
}

void cVerticalLabel::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.setBrush(Qt::Dense1Pattern);
    painter.translate(sizeHint().width(), sizeHint().height());
    painter.rotate(270);
    painter.drawText(-5,0,text());
}

QSize cVerticalLabel::minimumSizeHint() const
{
	QSize s = QLabel::minimumSizeHint();
    QRect b_rect = fontMetrics().boundingRect(text());
    return QSize(s.height()+(b_rect.height()/6), s.width());
}

QSize cVerticalLabel::sizeHint() const
{
	QSize s = QLabel::sizeHint();
    return QSize(s.height(), s.width());
}
