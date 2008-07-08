/***************************************************************************
 * SafeHttp.cc - Safe wrapper for QHttp which checks validity of given URIs
 *
 * Copyright (C) 2006, 2008 Matthew J Smith
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


#include "SafeHttp.h"

#include <QtNetwork>

SafeHttp::SafeHttp( QObject *parent )
  : QHttp( parent )
{

}

SafeHttp::SafeHttp( const QString &hostName,
		    quint16 port, QObject *parent )
  : QHttp( parent )
{
  httpHost = hostName;
}

SafeHttp::~SafeHttp()
{
}

void SafeHttp::safePost( const QString &host, 
			 const QString &path,
			 const QByteArray &data,
			 QIODevice *dataTo )
{
  httpHost = host;
  location = path;
  httpData = data;
  httpDataTo = dataTo;

  QHostInfo::lookupHost( host, this,
			 SLOT( handleLookupResult( QHostInfo ) ) );
}

void SafeHttp::handleLookupResult( const QHostInfo &hostInfo )
{
  if( hostInfo.error() != QHostInfo::NoError )
    emit hostLookupFailed();
  else {
    setHost( httpHost );
    post( location, httpData, httpDataTo );
  }
}

void SafeHttp::setCurrentHttpBusiness( int chb )
{
  _currentHttpBusiness = chb;
}

int SafeHttp::currentHttpBusiness()
{
  return _currentHttpBusiness;
}
