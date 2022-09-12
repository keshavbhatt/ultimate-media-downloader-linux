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

#ifndef BASIC_DOWNLOADER_H
#define BASIC_DOWNLOADER_H

#include <QString>
#include <QStringList>
#include <QMenu>

#include "settings.h"
#include "utility.h"
#include "context.hpp"
#include "tableWidget.h"
#include "networkAccess.h"
#include "customformatselector.h"

class basicdownloader : public QObject
{
	Q_OBJECT
public:
	basicdownloader( const Context& ) ;
	void init_done() ;
	void tabEntered() ;
	void tabExited() ;
	void enableAll() ;
	void disableAll() ;
	void appQuit() ;
	void gotEvent( const QByteArray& e ) ;
	void resetMenu( const QStringList& = {} ) ;
	void retranslateUi() ;
	basicdownloader& setAsActive() ;
    basicdownloader& hideTableList() ;
private slots:
    void resetView();
    void showCustomFormatSelector();
private:
	QString defaultEngineName() ;
	const engines::engine& defaultEngine() ;

	struct opts
	{
		const engines::engine& engine ;
		tableWidget& table ;

		const Context& ctx ;
		QString debug ;
		bool listRequested ;
		int index ;
	} ;

	template< typename Functions >
	auto make_options( basicdownloader::opts opts,Functions functions )
	{
		return utility::options< basicdownloader::opts,Functions >( std::move( opts ),std::move( functions ) ) ;
	}

	const Context& m_ctx ;
	settings& m_settings ;
	QString m_debug ;
	Ui::MainWindow& m_ui ;
	tabManager& m_tabManager ;
	tableMiniWidget< int > m_tableList ;
	QStringList m_optionsList ;
	QTableWidget m_bogusTableOriginal ;
	tableWidget m_bogusTable ;
	utility::Terminator m_terminator ;

	void run( const engines::engine& engine,
		  const QStringList& args,
		  const QString& quality,
		  bool list_requested ) ;

	void listRequested( const QByteArray& ) ;
	void list() ;
	void download( const engines::engine&,
		       const utility::args&,
		       const QString& urls,
		       bool = true ) ;
	void download( const engines::engine&,
		       const utility::args&,
		       const QStringList& urls,
		       bool = true ) ;
	void download( const QString& ) ;
	void exit() ;
} ;

#endif
