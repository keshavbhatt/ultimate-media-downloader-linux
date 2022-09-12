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
#ifndef NETWORK_ACCESS_H
#define NETWORK_ACCESS_H

#include "context.hpp"

#include "engines.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include <QFile>
#include <QStringList>

#include "context.hpp"
#include "settings.h"
#include "engines.h"

class basicdownloader ;

class networkAccess
{
public:
	networkAccess( const Context& ) ;
	void download( const engines::Iterator& ) ;
	static bool hasNetworkSupport()
	{
		#if QT_VERSION >= QT_VERSION_CHECK( 5,6,0 )
			return true ;
		#else
			return false ;
		#endif
	}

	template< typename Function >
	void getResource( const QString& url,Function function )
	{
		auto networkReply = m_accessManager.get( this->networkRequest( url ) ) ;

		QObject::connect( networkReply,&QNetworkReply::finished,[ networkReply,function = std::move( function ) ](){

			function( networkReply->readAll() ) ;
		} ) ;
	}
private:
	QNetworkRequest networkRequest( const QString& url ) ;

	struct metadata
	{
		qint64 size ;
		QString url ;
		QString sha256 ;
	};
	void download( const networkAccess::metadata&,const engines::Iterator&,const QString& path ) ;
	void post( const engines::engine&,const QString& ) ;
	const Context& m_ctx ;
	QNetworkAccessManager m_accessManager ;
	QFile m_file ;
	basicdownloader& m_basicdownloader ;
	tabManager& m_tabManager ;
};

#endif

