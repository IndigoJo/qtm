/*******************************************************************************
 
    XmlRpcHandler.cc - SAX parser for XML-RPC returns in QTM
    Copyright (C) 2006 Matthew J Smith

    This file is part of QTM.

    QTM is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License (version 2), as 
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*******************************************************************************/

// XmlRpcHandler.cc - XML-RPC handling routines for QTM

#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QMap>
#include <QList>
#include <QtXml>

#include "XmlRpcHandler.h"

XmlRpcHandler::XmlRpcHandler( int x )
{
  reqType = x;
  _fault = false;
  methodResponseFinished = false;
  receivingArgumentName = false;
  insideStruct = false;
  receivingFaultString = false;

  if( reqType == 5 )
    returnElement = doc.createElement( "blogs" );
  if( reqType == 13 )
    returnElement = doc.createElement( "categories" );
}

XmlRpcHandler::XmlRpcHandler()
{
  methodResponseFinished = false;
  receivingArgumentName = false;
  insideStruct = false;
  receivingFaultString = false;
}

void XmlRpcHandler::setProtocol( int x )
{
  reqType = x;
}

bool XmlRpcHandler::startElement( const QString &, const QString &,
				  const QString &qName,
				  const QXmlAttributes &attr )
{
  QString superElementName;

  /*if( qName == "struct" )
    currentString = "";
  if( (qName == "string" ) && (currentString.isNull() ) )
  currentString = "";*/

  if( (qName == "struct") ) {
    insideStruct = true;

    if( ((reqType == 5) || (reqType == 13)) ) {
      switch( reqType ) {
      case 5: superElementName = QString( "blog" ); break;
      case 13: superElementName = QString( "category" ); break;
      }
      // qDebug() << "Creating super element: " << superElementName.toAscii().data();
      currentSuperElement = doc.createElement( superElementName );
    }    
  }

  if( qName == "base64" || qName == "boolean" || qName == "dateTime.iso8601" ||
      qName == "double" || qName == "int" || qName == "string" )
    receivingData = true;

  if( qName == "member" && insideStruct )
    currentElement = doc.createElement( "currentElement" );

  if( qName == "name" ) {
    receivingArgumentName = true;
    currentRpcArgumentName = "";
  }

  if( qName == "fault" )
    _fault = true;

  if( qName == "faultString" )
    receivingFaultString = true;
  return true;
}

bool XmlRpcHandler::characters( const QString &str )
{
  QString stringToAdd;

  if( str.startsWith( "http://", Qt::CaseInsensitive ) && reqType != 11 )
    currentString += str.section( "//", 1 );
  else
    currentString += str;

  if( receivingArgumentName ) {
    currentRpcArgumentName = str;
    if( (reqType == 5) || (reqType == 13) ) {
      // qDebug() << "Naming current element: " << str.toAscii().data();
      currentElement.setTagName( currentRpcArgumentName );
    }
  }
  else {
    if( receivingData ) {
      if( str.startsWith( "http://", Qt::CaseInsensitive ) && reqType != 11 )
	stringToAdd = str.section( "//", 1 );
      // returnDataList[currentRpcArgumentName].append( str.section( "//", 1 ) );
      else
	stringToAdd = str;
      returnDataList[currentRpcArgumentName].append( stringToAdd );
      //returnDataList[currentRpcArgumentName].append( str );
      
      if( (reqType == 5) || (reqType == 13) )
	if( !currentElement.isNull() ) {
	  // qDebug() << "Appending text: " << str.toAscii().data();
	  currentElement.appendChild( QDomText( doc.createTextNode( stringToAdd ) ) );
	}
      
    }
  }
  if( receivingFaultString ) {
    // qDebug() << "Fault: " << str.toAscii().data();
    _faultString = str;
  }

  return true;
}

bool XmlRpcHandler::endElement( const QString &, const QString &,
				const QString &qName )
{
  if( qName == "name" ) {
    //currentString += ":";
    if( !returnDataList.contains( qName ) )
      returnDataList.insert( qName, QStringList() );
    receivingArgumentName = false;
  }
  if( qName == "faultString" )
    receivingFaultString = false;
  
  if( qName == "methodResponse" )
    methodResponseFinished = true;

  if( qName == "member" && insideStruct )
    if( !currentSuperElement.isNull() && !currentElement.isNull() ) {
      // qDebug() << "Appending current element: " << qName.toAscii().data();
      currentSuperElement.appendChild( currentElement );
    }

  if( qName == "struct" ) {
    if( !returnElement.isNull() && !currentSuperElement.isNull() ) {
      // qDebug() << "Appending current super-element: " << qName.toAscii().data();
      returnElement.appendChild( currentSuperElement );
    }
    /*    else
      if( returnElement.isNull() )
	// qDebug() << "Return element is null";
      else
      // qDebug() << "Super element is null"; */
    insideStruct = false;
  }

  if( qName == "base64" || qName == "boolean" || qName == "dateTime.iso8601" ||
      qName == "double" || qName == "int" || qName == "string" )
    receivingData = false;

  if( qName == "array" )
    if( !returnElement.isNull() )
      doc.appendChild( returnElement );

  /*  else {
    if( (qName == "string") || (qName == "int") || (qName == "boolean")
      || (qName == "dateTime.iso8601" ) )**
    if( qName == "value" ) {
      //currentString += ";";
      //returnDataList[currentRpcArgumentName].append( 
    }
    else {
      if( qName == "struct" )
	{
	returnData.append( currentString );
	}
      else {
	if( qName == "methodResponse" )
	  methodResponseFinished = true;
      }
    }
  } */
  return true;
}

bool XmlRpcHandler::fatalError( const QXmlParseException &exc )
{
  qWarning( "Line %d, column %d: %s", exc.lineNumber(),
	    exc.columnNumber(),
	    exc.message().toAscii().data() );
  _faultString.append( QString( "Line %1, column %2: %3" )
		     .arg( exc.lineNumber() )
		     .arg( exc.columnNumber() )
		     .arg( exc.message() ) );
  methodResponseFinished = true;
  return false;
}
/* 
QList<QString> XmlRpcHandler::returnList()
{
  return returnData;
  }*/
 
QStringList XmlRpcHandler::returnList( const QString &title )
{
  if( returnDataList.contains( title ) )
    return returnDataList.value( title );
  else
    return QStringList();
}

QDomDocumentFragment XmlRpcHandler::returnXml()
{
  QDomDocumentFragment returnFragment = doc.createDocumentFragment();
  returnFragment.appendChild( returnElement );
  return returnFragment;
}

bool XmlRpcHandler::fault()
{
  return _fault;
}

QString XmlRpcHandler::faultString()
{
  return _faultString;
}

/*QString XmlRpcHandler::returnFirstEntry()
{
  return returnData[0];
  }*/

bool XmlRpcHandler::isMethodResponseFinished()
{
  return methodResponseFinished;
}
