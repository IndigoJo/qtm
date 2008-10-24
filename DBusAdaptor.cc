/***************************************************************************
 * DBusAdaptor.cc - D-Bus adaptor allowing other applications to use
 * QTM's features.
 *
 * Copyright (C) 2008, Matthew J Smith
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


#include "DBusAdaptor.h"
#include "EditingWindow.h"
#include "qtm_version.h"

#include <QApplication>

DBusAdaptor::DBusAdaptor( SysTrayIcon *sti )
  : QDBusAbstractAdaptor( sti ), _sti( sti )
{
  connect( qApp, SIGNAL( aboutToQuit() ), SIGNAL( aboutToQuit() ) );
  connect( _sti, SIGNAL( quickpostTemplatesUpdated( QStringList ) ),
	   this, SIGNAL( quickpostTemplatesUpdated( QStringList ) ) );
  connect( _sti, SIGNAL( quickpostTemplateTitlesUpdated( QStringList ) ),
	   this, SIGNAL( quickpostTemplateTitlesUpdated( QStringList ) ) );
}

/** applicationVersion: Returns the version number of the application
 */
QString DBusAdaptor::applicationVersion()
{
    return QString( QTM_VERSION );
}

/** getQuickpostTemplateTitles: Returns a string list of the titles of available quickpost templates
 *  in format n.[title]
 */
QStringList DBusAdaptor::getQuickpostTemplateTitles()
{
    return _sti->templateTitles();
}

/** getQuickpostTemplates: Returns a string list of the titles and content of available quickpost templates
 *  in format n.[title].[template] - note that newlines are replaced with \n and closing square brackets
 *  with \]
 */
QStringList DBusAdaptor::getQuickpostTemplates()
{
  return _sti->templates();
}

/** quit - Quits the application
 */
Q_NOREPLY void DBusAdaptor::quit()
{
  _sti->doQuit();
}

/** newDocument - Opens a new blank entry
 */
Q_NOREPLY void DBusAdaptor::newDocument()
{
  EditingWindow *c = new EditingWindow;
  c->setSTI( _sti );
  c->show();
  c->activateWindow();
#if QT_VERSION >= 0x040300
  QApplication::alert( c );
#endif
}

/** newDocumentWithTitleAndText - Opens a new entry
 *  title - The title of the new document
 *  text - The content of the new document
 */
void DBusAdaptor::newDocumentWithTitleAndText( QString title, QString text )
{
  EditingWindow *c = new EditingWindow( text );
  c->setSTI( _sti );
  c->setPostTitle( title );
  c->setPostClean();
  c->show();
  c->activateWindow();
#if QT_VERSION >= 0x040300
  QApplication::alert( c );
#endif
}

/** quickpost - Does a quickpost.  The web location of the quickposted document is the first argument
 *  and the content is the second.
 *  Note that if the web location is associated with a particular template, that template will be
 *  used.
 */
void DBusAdaptor::quickpost( QString url, QString content )
{
  _sti->quickpostFromDBus( url, content );
}

/** open - Opens a saved entry from disk
 *  Returns true if successful and false if unsuccessful.
 */
bool DBusAdaptor::open( QString path )
{
  bool rv = false;

  if( !path.isEmpty() ) {
    EditingWindow *e = new EditingWindow( true );
    rv = e->load( path, true );
    if( rv ) {
      e->setSTI( _sti );
      e->show();
      e->activateWindow();
#if QT_VERSION >= 0x040300
      QApplication::alert( e );
#endif
    }
    else
      e->deleteLater();
  }
  return rv;
}
