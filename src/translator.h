/*
 *  Copyright (c) 2021 Keshav Bhatt
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QWidget>
#include <QTranslator>
#include <QMenu>
#include <QAction>
#include <QApplication>

#include <vector>

#include "settings.h"

class translator
{
public:
	translator( settings&,QApplication& ) ;
	void setLanguage( const QString& e ) ;
	void setDefaultLanguage() ;
	~translator() ;
	const QString& UIName( const QString& name ) ;
	const QString& name( const QString& UIName ) ;
	const QString& untranslated( const QString& string ) ;
	QString translate( const QString& internalName ) ;
	const char * UINameUnTranslated( const QString& name ) ;
	struct entry{
		entry( const QString&,const char *,const QString& ) ;
		QString UINameTranslated ;
		const char * UINameUnTranslated ;
		QString internalName ;
	} ;
	QAction * addAction( QMenu * m,translator::entry,bool permanentEntry = false ) ;
	QMenu * addMenu( QMenu * m,translator::entry,bool permanentEntry = false ) ;
private:
	void addString( const QString& translatedString,
			const char * untranslatedString,
			const QString& internalName ) ;
	void clear( void ) ;
	QApplication& m_qapp ;
	QTranslator * m_translator = nullptr ;
	std::vector< entry > m_languages ;
	std::vector< std::pair< QAction *,entry > > m_actions ;
	std::vector< std::pair< QMenu *,entry > > m_menus ;

	settings& m_settings ;
private:
};

#endif

