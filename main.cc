/*******************************************************************************
 
    QTM - Qt-based blog manager
    Copyright (C) 2006, 2007, 2008 Matthew J Smith

    This program is free software; you can redistribute it and/or modify
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

// main.cc - Main window loader for QTM

//#include <QApplication>
//#include <QMessageBox>
#include "Application.h"
#include "catkin.h"

//#include "useSTI.h"
/*#if QT_VERSION >= 0x040200
// #define USE_SYSTRAYICON
#ifdef USE_SYSTRAYICON
#include "SysTrayIcon.h"
#endif
#endif */

#if QT_VERSION < 0x040200
#undef USE_SYSTRAYICON
#endif

#ifdef USE_SYSTRAYICON
#include "SysTrayIcon.h"
#ifdef Q_WS_MAC
#include "SuperMenu.h"
#endif
#endif

int main( int argc, char *argv[] )
{
  Catkin *c;
#ifdef USE_SYSTRAYICON
  SysTrayIcon *sti;
#endif

  Application app( argc, argv );
  QStringList args = app.arguments();

#ifdef USE_SYSTRAYICON
  if( QSystemTrayIcon::isSystemTrayAvailable() ) {
    sti = new SysTrayIcon;
    if( !sti->dontStart() ) {
#ifndef Q_WS_MAC
      sti->show();
#else
      SuperMenu *smenu = new SuperMenu( 0, sti );
#endif
      app.setQuitOnLastWindowClosed( false );
    }
  }
  else {
    c = new Catkin;
    c->setSTI( 0 ); // No STI
    c->setWindowTitle( QObject::tr( "QTM - new entry [*]" ) );
    if( c->handleArguments() )
      c->show();
    else
      c->close();
  }
#else
  c = new Catkin;
  c->setWindowTitle( QObject::tr( "QTM - new entry [*]" ) );
  if( c->handleArguments() ) {
    c->show();
  }
  else {
    qDebug( "closing" );
    c->close();

  }
#endif

#ifdef USE_SYSTRAYICON
  if( QSystemTrayIcon::isSystemTrayAvailable() ) {
    if( !sti->dontStart() )
      return app.exec();
  }
  else
    return app.exec();
#else
  return app.exec();
#endif
}
