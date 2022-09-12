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

#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QCloseEvent>

class settings ;
class Logger ;

namespace Ui {
class logWindow;
}

class logWindow : public QWidget
{
	Q_OBJECT
public:
	logWindow( QWidget * parent,settings&,Logger& ) ;
	~logWindow() override ;
	void setText( const QByteArray& ) ;
	template< typename LoggerData >
	void update( const LoggerData& e )
	{
		this->update( e.toString() ) ;
	}
	void update( const QByteArray& e ) ;
	void Hide() ;
	void Show() ;
	void clear() ;
private:
	void closeEvent( QCloseEvent * ) override ;
	Ui::logWindow * m_ui ;
	settings& m_settings ;
};

#endif // LOGWINDOW_H
