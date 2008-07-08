/***************************************************************************
 * SafeHttp.h - Header for QTM's safe HTTP subclass
 *
 * Copyright (C) 2006, 2008, Matthew J Smith
 *
 * This file is part of QTM.
 * QTM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License (version 2), as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *****************************************************************************/


#include <QtGlobal>
#include <QHttp>

class QString;
class QIODevice;
class QByteArray;
class QHostInfo;

class SafeHttp : public QHttp
{
Q_OBJECT

public:
  SafeHttp( QObject *parent = 0 );
  SafeHttp( const QString &, quint16, QObject *parent = 0 ); 
  ~SafeHttp();
  int currentHttpBusiness();
  void setCurrentHttpBusiness( int );

public slots:
  void safePost( const QString &, const QString &, 
		 const QByteArray &, 
	    	 QIODevice *dataTo = 0 );
  void handleLookupResult( const QHostInfo & );

private:
  QString location;
  QString httpHost;
  QByteArray httpData;
  QIODevice *httpDataTo;
  int _currentHttpBusiness;

signals:
  void hostLookupFailed();
};
