/***************************************************************************
 *   Copyright (C) 2006 by Diego R. Brogna                                 *
 *   dierbro@gmail.com                                               	   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#ifndef PHP_HANDLER_H
#define PHP_HANDLER_H
		
#include <QMap>
#include <kurl.h>
#include <QByteArray>
#include <QProcess>


namespace kt
{
	class PhpCodeGenerator;
	
	class PhpHandler : public QProcess 
	{
		Q_OBJECT
	public:
		PhpHandler(const QString & php_exe,PhpCodeGenerator* gen);
		virtual ~PhpHandler();
			
		bool executeScript(const QString & path,const QMap<QString,QString> & args);
		const QByteArray & getOutput() const {return output;};
		
	public slots:
		void onFinished(int exitCode,QProcess::ExitStatus exitStatus);
		void onReadyReadStdout();
		
	private:
		bool containsDelimiters(const QString & str);
		
	signals:
		void finished();
		
	private:
		QByteArray output;
		QString php_exe;
		PhpCodeGenerator* gen;

		
		static QMap<QString,QByteArray> scripts;
	};
}

#endif
