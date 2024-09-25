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

#include "networkAccess.h"

#include "networkAccess.h"
#include "tabmanager.h"
#include "basicdownloader.h"
#include "engines.h"

#include <QtNetwork/QNetworkReply>
#include <QFile>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>

#if QT_VERSION >= QT_VERSION_CHECK( 5,4,0 )
static QString _sslLibraryVersionString()
{
	return QSslSocket::sslLibraryBuildVersionString() ;
}
#else
static QString _sslLibraryVersionString()
{
	return {} ;
}
#endif

networkAccess::networkAccess( const Context& ctx ) :
	m_ctx( ctx ),
	m_basicdownloader( m_ctx.TabManager().basicDownloader() ),
	m_tabManager( m_ctx.TabManager() )
{
	if( utility::platformIsWindows() && m_ctx.Settings().showVersionInfoWhenStarting() ){

		auto& e = m_ctx.logger() ;
		auto s = QSslSocket::sslLibraryVersionString() ;

		e.add( QObject::tr( "Checking installed version of" ) + " OpenSSL" ) ;

		if( s.isEmpty() ){

			auto q = _sslLibraryVersionString() ;
			auto m = QObject::tr( "Failed to find version information, make sure \"%1\" is installed and works properly" ).arg( q ) ;
			e.add( m ) ;
		}else{
			e.add( QObject::tr( "Found version" ) + ": " + s ) ;
		}
	}
}

QNetworkRequest networkAccess::networkRequest( const QString& url )
{
	QNetworkRequest networkRequest( url ) ;
#if QT_VERSION >= QT_VERSION_CHECK( 5,9,0 )
	networkRequest.setAttribute( QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::NoLessSafeRedirectPolicy ) ;
#else
	#if QT_VERSION >= QT_VERSION_CHECK( 5,6,0 )
		networkRequest.setAttribute( QNetworkRequest::FollowRedirectsAttribute,true ) ;
	#endif
#endif
	return networkRequest ;
}

void networkAccess::download( const engines::Iterator& iter )
{
	const auto& engine = iter.engine() ;

	auto exePath = engine.exePath().realExe() ;

	auto exeFolderPath = [ &exePath ](){

		auto a = QDir::fromNativeSeparators( exePath ) ;

		auto aa = a.lastIndexOf( '/' ) ;

		if( aa != -1 ){

			return a.mid( 0,aa ) ;
		}else{
			return a ;
		}
	}() ;

	auto internalBinPath = QDir::fromNativeSeparators( m_ctx.Engines().engineDirPaths().binPath() ) ;

	if( !exeFolderPath.startsWith( internalBinPath ) ){

		auto m = QDir::fromNativeSeparators( exePath ) ;

		auto a = m.lastIndexOf( '/' ) ;

		if( a == -1 ){

			exePath = internalBinPath + "/" + exePath ;
		}else{
			exePath = internalBinPath + "/" + m.mid(( a + 1 ) ) ;
		}
	}

	QDir dir ;

	auto path = engine.exeFolderPath() ;

	if( !dir.exists( path ) ){

		if( !dir.mkpath( path ) ){

			this->post( engine,QObject::tr( "Failed to download, Following path can not be created: " ) + path ) ;

			return ;
		}
	}

	this->post( engine,QObject::tr( "Start Downloading" ) + " " + engine.name() + " ..." ) ;

	m_tabManager.disableAll() ;

	QString url( engine.downloadUrl() ) ;

	auto networkReply = m_accessManager.get( this->networkRequest( url ) ) ;

	QObject::connect( networkReply,&QNetworkReply::downloadProgress,[ this,&engine ]( qint64 received,qint64 total ){

		Q_UNUSED( received )
		Q_UNUSED( total )

		this->post( engine,"..." ) ;
	} ) ;

	QObject::connect( networkReply,&QNetworkReply::finished,[ this,networkReply,&engine,iter,exePath ](){

		networkReply->deleteLater() ;

		if( networkReply->error() != QNetworkReply::NetworkError::NoError ){

			this->post( engine,QObject::tr( "Download Failed" ) + ": " + networkReply->errorString() ) ;

			m_tabManager.enableAll() ;

			if( iter.hasNext() ){

				m_ctx.versionInfo().check( iter.next() ) ;
			}

			return ;
		}

		networkAccess::metadata metadata ;

		util::Json json( networkReply->readAll() ) ;

		if( !json ){

			this->post( engine,QObject::tr( "Failed to parse json file from github" ) + ": " + json.errorString() ) ;

			m_tabManager.enableAll() ;

			if( iter.hasNext() ){

				m_ctx.versionInfo().check( iter.next() ) ;
			}

			return ;
		}

		auto object = json.doc().object() ;

		auto value = object.value( "assets" ) ;

		const auto array = value.toArray() ;

		for( const auto& it : array ){

			const auto object = it.toObject() ;

			const auto value = object.value( "name" ) ;

			auto entry = value.toString() ;

            if( entry == "yt-dlp" ){ // hardcoded engine name since we renamed it internally

				auto value = object.value( "browser_download_url" ) ;

				metadata.url = value.toString() ;

				value = object.value( "size" ) ;

				metadata.size = value.toInt() ;

			}else if( entry == "SHA2-256SUMS" ){

				auto value = object.value( "browser_download_url" ) ;

				metadata.sha256 = value.toString() ;
			}
		}

		this->download( metadata,iter,exePath ) ;
	} ) ;
}

void networkAccess::download( const networkAccess::metadata& metadata,
			      const engines::Iterator& iter,
			      const QString& path )
{
	const auto& engine = iter.engine() ;

	QString filePath = path + ".tmp" ;

	m_file.setFileName( filePath ) ;

	m_file.remove() ;

	m_file.open( QIODevice::WriteOnly ) ;

//	this->post( engine,QObject::tr( "Downloading" ) + ": " + metadata.url ) ;

//	this->post( engine,QObject::tr( "Destination" ) + ": " + filePath ) ;

	auto networkReply = m_accessManager.get( this->networkRequest( metadata.url ) ) ;

	QObject::connect( networkReply,&QNetworkReply::finished,[ this,networkReply,&engine,iter,path ](){

		networkReply->deleteLater() ;

		if( networkReply->error() != QNetworkReply::NetworkError::NoError ){

			this->post( engine,QObject::tr( "Download Failed" ) + ": " + networkReply->errorString() ) ;

			m_tabManager.enableAll() ;

			if( iter.hasNext() ){

				m_ctx.versionInfo().check( iter.next() ) ;
			}
		}else{
			m_file.close() ;

			this->post( engine,QObject::tr( "Download complete" ) ) ;

            //this->post( engine,QObject::tr( "Renaming file to: " ) + path ) ;

			QFile::remove( path ) ;

			m_file.rename( path ) ;

			m_file.setPermissions( m_file.permissions() | QFileDevice::ExeOwner ) ;

			m_ctx.versionInfo().check( iter ) ;            
		}
	} ) ;

	QObject::connect( networkReply,&QNetworkReply::downloadProgress,
			  [ this,metadata,networkReply,&engine ]( qint64 received,qint64 total ){

		Q_UNUSED( total )

		m_file.write( networkReply->readAll() ) ;

		utility::locale locale ;

		auto perc = double( received )  * 100 / double( metadata.size ) ;
		auto totalSize = locale.formattedDataSize( metadata.size ) ;
		auto current   = locale.formattedDataSize( received ) ;
		auto percentage = QString::number( perc,'f',2 ) ;

		auto m = QString( "%1 / %2 (%3%)" ).arg( current,totalSize,percentage ) ;

		this->post( engine,QObject::tr( "Downloading" ) + " " + engine.name() + ": " + m ) ;
	} ) ;
}

void networkAccess::post( const engines::engine& engine,const QString& m )
{
	m_ctx.logger().add( [ &engine,&m ]( Logger::Data& s,int id,bool ){

		auto e = m.toUtf8() ;

		Q_UNUSED( id )

		auto p = QObject::tr( "Downloading" ) + " " + engine.name() ;

		auto prefix = p.toUtf8() ;

		if( s.isEmpty() ){

            s.add( "[UMD4] " + e ) ;

		}else if( e == "..." ){

			s.replaceLast( s.lastText() + " ..." ) ;

		}else if( e.startsWith( prefix ) ){

            if( s.lastText().startsWith( "[UMD4] " + prefix ) ){

				s.removeLast() ;
			}

            s.add( "[UMD4] " + e ) ;
		}else{
            s.add( "[UMD4] " + e ) ;
		}
	},-1 ) ;
}
