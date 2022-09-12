

#ifndef ABOUT_H
#define ABOUT_H

#include <QListWidget>
#include <QWidget>

#include "context.hpp"
#include "supportedsites.h"

class tabManager ;

class about
{
public:
	about( const Context& ) ;
	void init_done() ;
	void enableAll() ;
	void disableAll() ;
	void resetMenu() ;
	void retranslateUi() ;
	void tabEntered() ;
    void tabExited() ;
private slots:
    void filterSupportedSites(QString search_str);
    void hideAllSupportedSites(QListWidget *listWidget);
private:
	const Context& m_ctx ;
    SupportedSites *supportedSites = nullptr;
};

#endif

