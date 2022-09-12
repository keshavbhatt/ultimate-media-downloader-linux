#include "controlbutton.h"
#include <QToolTip>
#include <QEvent>
#include <QMouseEvent>

controlButton::controlButton(QWidget *parent)
    : QPushButton(parent)
{
    setMouseTracking(true);
}

bool controlButton::eventFilter(QObject *obj, QEvent *event){
    Q_UNUSED(obj);
    if(event->type() == QEvent::ToolTip && this->isEnabled()){
        return true;
    }
    return controlButton::eventFilter(obj,event);
}

void controlButton::mouseMoveEvent(QMouseEvent *e){
    if(this->isEnabled()){
        QToolTip::showText(this->mapToGlobal(e->localPos().toPoint()),this->toolTip());
    }
    QPushButton::mouseMoveEvent(e);
}
