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

#include <QByteArray>
#include <QJsonArray>

#include "../engines.h"

class youtube_dl : public engines::engine::functions
{
public:
	~youtube_dl() override ;

	class youtube_dlFilter : public engines::engine::functions::filter
	{
	public:
		youtube_dlFilter( const QString&,const engines::engine& ) ;

		const QByteArray& operator()( const Logger::Data& e ) override ;

		~youtube_dlFilter() override ;
	private:
		const QByteArray& youtubedlOutput( const Logger::Data& ) ;
		const QByteArray& ytdlpOutput( const Logger::Data& ) ;
		bool m_likeYtdlp ;
		engines::engine::functions::preProcessing m_preProcessing ;
		engines::engine::functions::postProcessing m_postProcessing ;
		QByteArray m_tmp ;
		QByteArray m_fileName ;
	} ;

	std::vector< QStringList > mediaProperties( const QByteArray& ) override ;

	QStringList dumpJsonArguments() override ;

	bool breakShowListIfContains( const QStringList& ) override ;

	engines::engine::functions::DataFilter Filter( const QString& ) override ;

	void runCommandOnDownloadedFile( const QString&,const QString& ) override ;

	QString updateTextOnCompleteDownlod( const QString& uiText,
					     const QString& bkText,
					     const QString& downloadingOptions,
					     const engines::engine::functions::finishedState& ) override ;

	void updateDownLoadCmdOptions( const engines::engine::functions::updateOpts& ) override ;

	static QJsonObject init( const QString& name,
				 const QString& configFileName,
				 Logger& logger,
				 const engines::enginePaths& enginePath ) ;

	youtube_dl( const engines&,
		    const engines::engine&,
		    QJsonObject&,
		    Logger& logger,
		    const engines::enginePaths& ) ;
private:
	const engines::engine& m_engine ;
	QJsonArray m_objs ;
};
