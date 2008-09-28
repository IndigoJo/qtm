/*********************************************************************************

    SysTrayIcon.cc - System tray icon for QTM
    Copyright (C) 2006, 2007, 2008 Matthew J Smih

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

// SysTrayIcon.cc - System tray icon for QTM

#include <QtGlobal>
//#include "useSTI.h"

#if QT_VERSION >= 0x040200
#ifdef USE_SYSTRAYICON

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QFileDialog>
#include <QMessageBox>
#include <QCoreApplication>
#include <QClipboard>
#include <QUrl>
#include <QHttpRequestHeader>
#include <QList>
#include <QFile>
#include <QTextStream>
#include <QRegExp>

#include <QtXml>
#include <cstdio>
#include <QtDebug>

#ifndef DONT_USE_DBUS
#include <QtDBus>
#include "DBusAdaptor.h"
#endif

#include "Application.h"
#include "SafeHttp.h"
#include "SysTrayIcon.h"
#include "catkin.h"
#include "QuickpostTemplate.h"
#include "QuickpostTemplateDialog.h"

//#include "ui_QuickpostTemplateForm.h"

#ifdef Q_WS_X11
#include "susesystray.xpm"
#else
#ifdef Q_WS_WIN
#include "winsystray.xpm"
#include "winsystray_busy.xpm"
#endif
#endif

#include "qtm_version.h"

SysTrayIcon::SysTrayIcon( bool noWindow, QObject *parent )
  : QSystemTrayIcon( parent )
{
  bool newWindow;
  bool noNewWindow = false;
  _dontStart = false;

  QCoreApplication::setOrganizationName( "Catkin Project" );
  QCoreApplication::setOrganizationDomain( "catkin.blogistan.co.uk" );
  QCoreApplication::setApplicationName( "QTM" );
  userAgentString = QString( "QTM/%1" ).arg( QTM_VERSION );

  QSettings settings;
  settings.beginGroup( "sysTrayIcon" );
  newWindow = settings.value( "newWindowAtStartup", true ).toBool();
  doubleClickFunction = settings.value( "doubleClickFunction", 0 ).toInt();
  settings.endGroup();
  settings.beginGroup( "account" );
  _copyTitle = settings.value( "copyTitle", true ).toBool();
  settings.endGroup();

#ifndef DONT_USE_DBUS
  /*  dbus = new QDbusConnection( "qtm" );
  dbus = connectToBus( QDBusConnection::ActivationBus, "qtm" );*/
  new DBusAdaptor( this );
  QDBusConnection::sessionBus().registerObject( "/MainApplication", this );
  QDBusConnection::sessionBus().registerService( "uk.co.blogistan.catkin" );
#endif

  qtm = qobject_cast<Application *>( qApp );

#ifndef Q_WS_MAC
  switch( doubleClickFunction ) {
  case 0:
    setToolTip( tr( "QTM - Double click for \nnew entry" ) ); break;
  case 1:
    setToolTip( tr( "QTM - Double click for \nQuick Post" ) ); break;
  }

  setIcon( QIcon( QPixmap( sysTrayIcon_xpm ) ) );
#endif

  newWindowAtStartup = new QAction( tr( "New entry at startup" ), this );
  newWindowAtStartup->setCheckable( true );
  newWindowAtStartup->setChecked( newWindow );
  connect( newWindowAtStartup, SIGNAL( toggled( bool ) ),
	   this, SLOT( setNewWindowAtStartup( bool ) ) );

  menu = new QMenu;
  menu->addAction( tr( "New entry" ), this, SLOT( newDoc() ) );
  menu->addAction( tr( "Open ..." ), this, SLOT( choose() ) );
  openRecent = menu->addAction( tr( "Open recent" ) );
  menu->addAction( tr( "Quick post" ), this, SLOT( quickpost() ) );
  abortAction = menu->addAction( tr( "Abort quick post" ), this, SLOT( abortQP() ) );
  abortAction->setEnabled( false );
  templateMenu = menu->addMenu( tr( "Quickpost templates" ) );
  templateMenu->setObjectName( "templateMenu" );
  configureTemplates = new QAction( tr( "&Configure ..." ), 0 );
  connect( configureTemplates, SIGNAL( triggered( bool ) ),
	   this, SLOT( configureQuickpostTemplates() ) );
  // templateMenu->addAction( configureTemplates );
  // configureTemplates->setMenu( templateMenu );
  configureTemplates->setObjectName( "QTM Configure Templates" );
  setupQuickpostTemplates();
  menu->addAction( newWindowAtStartup );
#ifndef Q_WS_MAC
  menu->addSeparator();
#endif
#ifndef SUPERMENU
  quitAction = menu->addAction( tr( "Quit" ), this, SLOT( doQuit() ) );
#endif
  // quitAction->setMenuRole( QAction::QuitRole );
  menu->setObjectName( "iconMenu" );
  setContextMenu( menu );
#ifdef Q_WS_MAC
  qt_mac_set_dock_menu( menu );
#endif
  //show();

  recentFilesMenu = new QMenu;
  noRecentFilesAction = new QAction( tr( "No recent files" ), this );
  noRecentFilesAction->setVisible( true );
  noRecentFilesAction->setEnabled( false );
  recentFilesMenu->addAction( noRecentFilesAction );
  openRecent->setMenu( recentFilesMenu );
  for( int i = 0; i < 10; ++i ) {
    recentFileActions[i] = new QAction( this );
    recentFilesMenu->addAction( recentFileActions[i] );
    connect( recentFileActions[i], SIGNAL( triggered() ),
	     this, SLOT( openRecentFile() ) );
  }
  recentFiles = qtm->recentFiles();
  //  qDebug() << recentFiles.count() << " recent files";
  updateRecentFileMenu();
  connect( qtm, SIGNAL( recentFilesUpdated( QList<Application::recentFile> ) ),
	   this, SLOT( setRecentFiles( QList<Application::recentFile> ) ) );

#ifndef Q_WS_MAC
  connect( this, SIGNAL( activated( QSystemTrayIcon::ActivationReason ) ),
	   this, SLOT( iconActivated( QSystemTrayIcon::ActivationReason ) ) );
#endif
  /*connect( http, SIGNAL( done( bool ) ),
    this, SLOT( handleDone( bool ) ) ); */

  if( handleArguments() )
    noNewWindow = true;

  if( newWindow && !noNewWindow ) {
    Catkin *c = new Catkin;
    c->setSTI( this );
    c->setWindowTitle( tr( "QTM - new entry [*]" ) );
    c->show();
    c->activateWindow();
  }
  //cbtextIsURL = Untested;
  http = new SafeHttp;
  httpBusy = false;
  templateQPActive = false;
  activeTemplate = -1;
}

SysTrayIcon::~SysTrayIcon()
{
}

bool SysTrayIcon::handleArguments()
{
  int i;
  bool rv = false;
  Catkin *c;
  QStringList failedFiles;
  QStringList args = QApplication::arguments();

  for( i = 1; i < args.size(); i++ ) {
    c = new Catkin( true, 0 );
    if( c->load( args.at( i ), true ) ) {
      c->setSTI( this );
      c->show();
      rv = true;
    }
    else {
      /*QMessageBox::showMessage( 0, QObject::tr( "Error" ),
				QObject::tr( "Could not load %1." ).arg( args.at( i ) ),
				QMessageBox::Cancel, QMessageBox::NoButton ); */
				failedFiles.append( args.at( i ) );
    }
  }
  if( failedFiles.size() ) {
    if( failedFiles.size() == args.size()-1 ) {
      if( QMessageBox::question( 0, tr( "Error" ),
				 tr( "Could not load the following:\n\n%1" )
				 .arg( failedFiles.join( "\n" ) ),
				 tr( "&Continue" ), tr( "E&xit" ) ) ) {
	_dontStart = true;
      }
      else
	rv = false;
    }
    else {
      QMessageBox::information( 0, tr( "Error" ),
				tr( "Could not load the following:\n\n%1 " )
				.arg( failedFiles.join( "\n" ) ),
				QMessageBox::Ok );
      rv = true;
    }
  }
  return rv;
}

void SysTrayIcon::updateRecentFileMenu()
{
  QString text, t;
  int j;
  QMutableListIterator<Application::recentFile> i( recentFiles );

  while( i.hasNext() ) {
    if( !QFile::exists( i.next().filename ) )
      i.remove();
  }

  for( j = 0; j < 10; ++j ) {
    if( j < recentFiles.count() ) {
      t = recentFiles.value( j ).title.section( ' ', 0, 5 );
      if( t != recentFiles.value( j ).title )
	t.append( tr( " ..." ) );
      if( j == 9 )
         text = tr( "1&0 %1" )
	.arg( recentFiles.value( j ).title.isEmpty() ?
	      recentFiles.value( j ).filename.section( "/", -1, -1 ) : t );
      else
        text = tr( "&%1 %2" )
	  .arg( j + 1 )
	  .arg( recentFiles.value( j ).title.isEmpty() ?
	        recentFiles.value( j ).filename.section( "/", -1, -1 ) : t );
      recentFileActions[j]->setText( text );
      recentFileActions[j]->setData( recentFiles.value( j ).filename );
      recentFileActions[j]->setVisible( true );
      noRecentFilesAction->setVisible( false );
    }
    else {
      recentFileActions[j]->setVisible( false );
      if( !recentFiles.count() )
	noRecentFilesAction->setVisible( true );
    }
  }
}

void SysTrayIcon::setRecentFiles( const QList<Application::recentFile> &rfs )
{
  recentFiles = rfs;
  updateRecentFileMenu();
}

void SysTrayIcon::setDoubleClickFunction( int func )
{
  doubleClickFunction = func;

#ifndef Q_WS_MAC
  switch( doubleClickFunction ) {
  case 0:
    setToolTip( tr( "QTM - Double click for \nnew entry" ) ); break;
  case 1:
    setToolTip( tr( "QTM - Double click for \nQuick Post" ) ); break;
  }
#endif
}

QStringList SysTrayIcon::templateTitles()
{
  QStringList rv;
  int i;

  for( i = 0; i < templateTitleList.count(); i++ ) {
    rv.append( QString( "%1.%2" )
	       .arg( i )
	       .arg( templateTitleList.value( i ) ) );
  }
  return rv;
}

QStringList SysTrayIcon::templates()
{
  QStringList rv;
  QString currentTemplate;
  int i;

  for( i = 0; i< templateTitleList.count(); i++ ) {
    currentTemplate = templateList.value( i )
      .replace( "\n", "\\n" )
      .replace( "]", "\\]" );
    rv.append( QString( "%1.[%2].[%3]" )
               .arg( i )
               .arg( templateTitleList.value( i ) )
               .arg( currentTemplate ) );
  }
  return rv;
}

void SysTrayIcon::setCopyTitle( bool status )
{
    _copyTitle = status;
}

void SysTrayIcon::newDoc()
{
    Catkin *c = new Catkin;
    c->setSTI( this );
    c->show();
    c->activateWindow();
#if QT_VERSION >= 0x040300
    QApplication::alert( c );
#endif
}

void SysTrayIcon::openRecentFile()
{
  QAction *action = qobject_cast<QAction *>( sender() );
  if( action )
    choose( action->data().toString() );
}

void SysTrayIcon::choose( QString fname )
{
  QString fn;
  QSettings settings;
  QString localStorageFileExtn, localStorageDirectory;
  QStringList filesSelected;

  settings.beginGroup( "account" );
  localStorageFileExtn = settings.value( "localStorageFileExtn", "cqt" ).toString();
  localStorageDirectory = settings.value( "localStorageDirectory", "" ).toString();
  settings.endGroup();

  QString extn( QString( "%1 (*.%2)" ).arg( tr( "Blog entries" ) )
		.arg( localStorageFileExtn ) );
  /*  QFileDialog *fd = new QFileDialog;
  fd->setFileMode( QFileDialog::AnyFile );
  fd->setAcceptMode( QFileDialog::AcceptOpen );
  fd->setWindowTitle( tr( "Choose a file to open" ) );
  fd->setFilter( extn );
  if( fd->exec() ) {
    filesSelected = fd->selectedFiles();
    fn = filesSelected[0];*/
  
  if( fname.isEmpty() )
    fn = QFileDialog::getOpenFileName( 0, tr( "Choose a file to open" ),
				       localStorageDirectory, extn );
  else
    fn = fname;

  if( !fn.isEmpty() ) {
    Catkin *e = new Catkin( true );
    if( !e->load( fn, true ) ) {
      /*      e->show();
	      else { */
#ifdef Q_WS_MAC
      QMessageBox::warning( 0, "QTM",
			    tr( "Could not load the file you specified." ),
			    QMessageBox::Cancel, QMessageBox::NoButton );
#else
      showMessage( tr( "Error" ), 
		   tr( "Could not load the file you specified." ),
		   QSystemTrayIcon::Warning );
#endif
      e->deleteLater();
    } else {
      e->setSTI( this );
      qtm->addRecentFile( e->postTitle(), fn );
      e->show();
      e->activateWindow();
#if QT_VERSION >= 0x040300
      QApplication::alert( e );
#endif
    }
  }
}

void SysTrayIcon::iconActivated( QSystemTrayIcon::ActivationReason ar )
{
#ifndef Q_WS_MAC
  if( ar == QSystemTrayIcon::DoubleClick )
    switch( doubleClickFunction ) {
    case 0: newDoc(); break;
    case 1: quickpost(); break;
    }

#ifdef Q_WS_X11
  if( ar == QSystemTrayIcon::MiddleClick )
    quickpost( QClipboard::Selection );
#endif
#endif
}

void SysTrayIcon::quickpost( QClipboard::Mode mode )
{
  int i, j;
  //QListIterator i;
  //QStringListIterator j;
  bool qpt = false;
  QRegExp regExp;

  /*#if QT_VERSION >= 0x040300 && !defined DONT_USE_SSL
  QRegExp httpRegExp( "^(http|https):\\/\\/" );
#else
  QRegExp httpRegExp( "^http:\\/\\/" );
#endif */
  cbtext = QApplication::clipboard()->text( mode );

  if( cbtext == "" )
    showMessage( tr( "No selection!" ),
		 tr( "No web location specified to blog about." ),
		 QSystemTrayIcon::Warning );
  else {
    if( !cbtext.startsWith( "http://" )
#if QT_VERSION >= 0x040300 && !defined DONT_USE_SSL
	&& !cbtext.startsWith( "https://" )
#endif
	) {
      //qDebug( "doesn't match" );
      // If it's not obviously an URL.
      if( cbtext.startsWith( "https" ) ) {
	if( supportsMessages() )
	  showMessage( tr( "Error" ), tr( "This version of QTM does not support HTTPS." ) );
	else
	  QMessageBox::information( 0, tr( "Quickpost Error" ),
				    tr( "This version of QTM does not support HTTPS." ) );
      }
      doQP( "" );
    }
    else {
      if( !QUrl( cbtext, QUrl::StrictMode ).isValid() ) {
	if( cbtext.contains( QChar( ' ' ) ) ) {
	  doQP( "" );
	}
	else
	  showMessage( tr( "Invalid web location" ),
		       tr( "Quick Post requires valid web location." ),
		       QSystemTrayIcon::Warning );
      }
      else {
      // Otherwise, it's an URL, and has to be downloaded to extract the title.
	if( !httpBusy ) {
	  if( assocHostLists.count() ) {
	    // i = QListIterator( assocHostLists );
	    //while( i.hasNext() ) {
	    for( i = 0; i < assocHostLists.count(); ++i ) {
	      if( assocHostLists.at( i ).count() ) {
		for( j = 0; j < assocHostLists.at( i ).count(); j++ ) {
		  //qDebug() << "Trying " << assocHostLists.at( i ).at( j )
		  //  .toAscii().data();
		  if( (!assocHostLists.at( i ).at( j ).isEmpty()) &&
		      cbtext.contains( QRegExp( QString( "[/\\.]%1[/\\.]" )
						.arg( assocHostLists.at( i ).at( j ) ) ) ) ) {
		    //qDebug() << "Match: " << cbtext;
		    qpt = true;
		    quickpostFromTemplate( i, 
					   quickpostTemplateActions.value( i )->postTemplate(),
					   cbtext );
		    break;
		  }
		}
	      }
	      if( qpt )
		break;
	    }
	  }

	  if( !qpt ) {
	    //qDebug() << "doing quickpost";
	    QString withoutHttp = cbtext.section( "://", 1 );
	    QString host = withoutHttp.section( "/", 0, 0 );
	    QString loc = withoutHttp.section( '/', 1, -1, QString::SectionIncludeLeadingSep );
	    QHttpRequestHeader header( "GET", loc );
	    header.setValue( "Host", host );
	    header.setValue( "User-Agent", userAgentString );

#if QT_VERSION >= 0x040300 && !defined DONT_USE_SSL
	    http->setHost( host, cbtext.startsWith( "https://" ) ?
			   QHttp::ConnectionModeHttps : QHttp::ConnectionModeHttp );
	    connect( http, SIGNAL( sslErrors( QList<QSslError> ) ),
		     http, SLOT( ignoreSslErrors() ) );
#else
	    http->setHost( host );
#endif
	    http->request( header );
	    abortAction->setEnabled( true );
	    httpBusy = true;
	    templateQPActive = false;
#ifdef Q_WS_WIN
	    setIcon( QIcon( QPixmap( sysTrayIcon_busy_xpm ) ) );
#endif
	    connect( http, SIGNAL( done( bool ) ),
		     this, SLOT( handleDone( bool ) ) );
	    connect( http, SIGNAL( readyRead( const QHttpResponseHeader & ) ),
		     this, SLOT( handleResponseHeader( const QHttpResponseHeader & ) ) );
	    connect( http, SIGNAL( hostLookupFailed() ),
		     this, SLOT( handleHostLookupFailed() ) );
	  }
	}
      }
    }
  }
}


void SysTrayIcon::quickpostFromTemplate( int id, QString templateString, QString t )
{
  activeTemplate = id;

  if( t.isNull() )
    cbtext = QApplication::clipboard()->text( QClipboard::Clipboard );
  else
    cbtext = t;

  if( cbtext.startsWith( "https://" ) ) {
    if( supportsMessages() )
      showMessage( tr( "Error" ),
		   tr( "This version of QTM does not support HTTPS." ) );
    else
      QMessageBox::information( 0, tr( "Quickpost Error" ),
				tr( "This version of QTM does not support HTTPS." ) );
  }

  if( !QUrl( cbtext, QUrl::StrictMode ).isValid() ) {
    showMessage( tr( "No selection!" ),
		 tr( "The selection is not a web location, or is invalid." ),
		 QSystemTrayIcon::Warning );
  }
  else {
    if( !httpBusy ) {
      QString withoutHttp = cbtext.section( "://", 1 );
      QString host = withoutHttp.section( "/", 0, 0 );
      QString loc = withoutHttp.section( '/', 1, -1, QString::SectionIncludeLeadingSep );

#if QT_VERSION >= 0x040300 && !defined DONT_USE_SSL
      http->setHost( host, cbtext.startsWith( "https://" ) ?
		     QHttp::ConnectionModeHttps : QHttp::ConnectionModeHttp );
      connect( http, SIGNAL( sslErrors( QList<QSslError> ) ),
	       http, SLOT( ignoreSslErrors() ) );
#else
      http->setHost( host );
#endif
      http->get( loc );
      abortAction->setEnabled( true );
      httpBusy = true;
      templateQPActive = true;
#ifdef Q_WS_WIN
      setIcon( QIcon( QPixmap( sysTrayIcon_busy_xpm ) ) );
#endif
      connect( http, SIGNAL( done( bool ) ),
	       this, SLOT( handleDone( bool ) ) );
      connect( http, SIGNAL( readyRead( const QHttpResponseHeader & ) ),
	       this, SLOT( handleResponseHeader( const QHttpResponseHeader & ) ) );
      connect( http, SIGNAL( hostLookupFailed() ),
	       this, SLOT( handleHostLookupFailed() ) );
    }
    // cbtext = templateString.replace( "%url%", cbtext );
  }
}

void SysTrayIcon::quickpostFromDBus( QString &url, QString &content )
{
  int i, j;

  for( i = 0; i < assocHostLists.count(); ++i ) {
    if( assocHostLists.at( i ).count() ) {
      for( j = 0; j < assocHostLists.at( i ).count(); j++ ) {
        //qDebug() << "Trying " << assocHostLists.at( i ).at( j )
        //  .toAscii().data();
        if( (!assocHostLists.at( i ).at( j ).isEmpty()) &&
            url.contains( QRegExp( QString( "[/\\.]%1[/\\.]" )
                                   .arg( assocHostLists.at( i ).at( j ) ) ) ) ) {
          //qDebug() << "Match: " << cbtext;
          activeTemplate = i;
          break;
        }
      }
    }
  }
  cbtext = url;
  doQP( content );
}

void SysTrayIcon::setNewWindowAtStartup( bool nwas )
{
  QSettings settings;

  settings.beginGroup( "sysTrayIcon" );
  settings.setValue( "newWindowAtStartup", nwas );
  settings.endGroup();
}

// HTTP Quickpost handler routines

void SysTrayIcon::handleHostLookupFailed()
{
  disconnect( http );
  abortAction->setEnabled( false );
  httpBusy = false;
  http->abort();
  showMessage( tr( "Cannot find page" ),
	       tr( "QTM cannot find the resource you specified.\n"
		   "Check that it is spelled right and that your "
		   "network is working." ),
	       QSystemTrayIcon::Warning );
}

void SysTrayIcon::handleDone( bool error )
{
  QString ddoc, newTitle, errorString;
  QHttp::Error httpError;

  abortAction->setEnabled( false );
  httpBusy = false;

  if( error ) {
    httpError = http->error();
    switch( httpError ) {
    case QHttp::HostNotFound:
      errorString = tr( "Could not find the host." ); break;
    case QHttp::ConnectionRefused:
      errorString = tr( "Connection was refused." ); break;
    case QHttp::UnexpectedClose:
      errorString = tr( "Connection closed unexpectedly." ); break;
    case QHttp::InvalidResponseHeader:
    case QHttp::WrongContentLength:
    case QHttp::UnknownError:
      errorString = tr( "Document was not received correctly." ); break;
    }
    if( httpError != QHttp::Aborted )
      showMessage( tr( "Quickpost Error" ), errorString, QSystemTrayIcon::Warning );
  }
  else {
    doQP( QString( responseData ) );
    responseData = "";
  }

  // http->close();
  http->disconnect();
#ifndef Q_WS_MAC
  setIcon( QIcon( QPixmap( sysTrayIcon_xpm ) ) );
#endif
}

void SysTrayIcon::handleResponseHeader( const QHttpResponseHeader &header ) // slot
{
  if( header.statusCode() == 404 ) {
    showMessage( tr( "Error 404" ),
		 tr( "The file or entry you specified could not be\n"
		     "found." ),
		 QSystemTrayIcon::Warning );
    http->disconnect();
    http->abort();
    abortAction->setEnabled( false );
    httpBusy = false;
    // http->deleteLater();
    // http = 0;
    // setNetworkActionsEnabled( false );
  }
  else
    responseData.append( http->readAll() );
}

void SysTrayIcon::doQuit()
{
  int catkins = 0;
  int a;

  QWidgetList tlw = qApp->topLevelWidgets();
  for( a = 0; a < tlw.size(); a++ ) {
    if( (QString( tlw[a]->metaObject()->className() ) == "Catkin")
	&& tlw[a]->isVisible() )
      catkins++;
  }

#ifndef NO_DEBUG_OUTPUT
  qDebug() << catkins << " main windows";
#endif
  
  //if( QApplication::topLevelWidgets().size() <= noWidgets )
  if( !catkins )
    QCoreApplication::quit();
  else {
    qApp->setQuitOnLastWindowClosed( true );
    qApp->closeAllWindows();
  }

}
//QCoreApplication::quit(); */

/*
#ifdef Q_WS_MAC
  if( QApplication::topLevelWidgets().isEmpty() ) {
    QCoreApplication::quit();
  } else {
    connect( qApp, SIGNAL( lastWindowClosed() ), qApp, SLOT( quit() ) );
    qApp->closeAllWindows();
  }
#else
  connect( qApp, SIGNAL( lastWindowClosed() ), qApp, SLOT( quit() ) );
  qApp->closeAllWindows();
#endif
} */

void SysTrayIcon::doQP( QString receivedText )
{
  QString newPost, newTitle;
  // QRegExp titleRegExp( "<title( (dir=\"(ltr)|(rtl)\")|((xml:)?lang=\"[a-zA-Z-]+\"){0, 2}>", Qt::CaseInsensitive );
  QRegExp titleRegExp( "<title.*</title>", Qt::CaseInsensitive );
  QRegExp openTitleRX( "<title", Qt::CaseInsensitive );
  QRegExp closeTitleRX( "</title", Qt::CaseInsensitive );

  if( activeTemplate >= 0 ) {
    if( receivedText == "" )
      newPost = cbtext;
    else {
      newPost = templateList[activeTemplate];
      newPost.replace( "%url%", cbtext );
      newPost.replace( "%host%", cbtext.section( "//", 1, 1 ).section( "/", 0, 0 ) );
      newPost.replace( "%domain%", cbtext.section( "//", 1, 1 ).section( "/", 0, 0 )
		       .remove( "www." ) );
      if( receivedText.contains( titleRegExp )
	  && receivedText.section( openTitleRX, 1 )
	  .section( ">", 1 ).contains( closeTitleRX ) ) {
	newTitle = receivedText.section( openTitleRX, 1 ).section( ">", 1 )
	  .section( closeTitleRX, 0, 0 ).simplified();
	newPost.replace( "%title%", newTitle );
      }
    }
    newPost.replace( "\\n", "\n" );
  } else {
    if( receivedText == "" )
      newPost = cbtext;
    else {
	//// qDebug( receivedText.left( 300 ).toAscii().constData() );
	// The title check will accept flaky 1990s HTML - this isn't a browser
      if( receivedText.contains( titleRegExp ) ) {
	/*&& receivedText.section( titleRegExp, 1 )
	  .section( '\n', 0 ).contains( "</title>", Qt::CaseInsensitive ) )*/
	newTitle = receivedText.section( openTitleRX, 1 ).section( ">", 1 )
	  .section( closeTitleRX, 0, 0 )
	  .simplified();
	if( !templateQPActive )
	  newPost = QString( "<a title = \"%1\" href=\"%2\">%1</a>\n\n" )
	    .arg( newTitle )
	    .arg( cbtext );
      }
      else // Post has no valid title
	newPost = QString( tr( "<a href=\"%1\">Insert link text here</a>" ) )
	  .arg( cbtext );
    }
  }

  Catkin *c = new Catkin( newPost );
  c->setSTI( this );
  c->setPostClean();

  if( activeTemplate >= 0 ) {
    switch( defaultPublishStatusList[activeTemplate] ) {
    case 0:
    case 1:
      c->setPublishStatus( defaultPublishStatusList[activeTemplate] );
    }
// Copy the title, if a quickpost
    if( copyTitleStatusList[activeTemplate] ) {
      if( !newTitle.isEmpty() )
	c->setWindowTitle( QString( "%1 - QTM" ).arg( newTitle ) );
      c->setPostTitle( newTitle );
    }
  }
  else {
    if( _copyTitle ) {
// Copy the title, if not a quickpost
      c->setPostTitle( newTitle );
      if( !newTitle.isEmpty() )
	c->setWindowTitle( QString( "%1 - QTM" ).arg( newTitle ) );
    }
  }

  c->show();
  c->activateWindow();
#ifdef Q_WS_WIN
  setIcon( QIcon( QPixmap( sysTrayIcon_xpm ) ) );
#endif

#if QT_VERSION >= 0x040300
  QApplication::alert( c );
#endif

  activeTemplate = -1;
  templateQPActive = false;
}

void SysTrayIcon::abortQP()
{
  http->abort();
  http->disconnect();
  abortAction->setEnabled( false );
  httpBusy = false;
}

void SysTrayIcon::setupQuickpostTemplates()
{
  QString templateFile;
  QString errorString;
  QString currentTitle, currentTemplate;
  int currentDefaultPublishStatus;
  int errorLine, errorCol;
  QDomNodeList templateNodes, titles, templateStrings, defaultPublishStates;
  QDomNodeList assocHostsInTemplate;
  QDomElement currentAssocHostListsElement, currentCopyStatusElement;
  QString currentCopyStatusElementText;
  int numTemplates, t, j;
  QTextStream ts( stdout );
  bool useDefaultPublishStatus, ok;

  QSettings settings;
  settings.beginGroup( "account" );
  templateFile = QString( "%1/qptemplates.xml" ).
    arg( settings.value( "localStorageDirectory", "" ).toString() );
  settings.endGroup();

  if( QFile::exists( templateFile ) ) {
    QDomDocument domDoc( "quickpostTemplates" );
    QFile file( templateFile );
    if( !domDoc.setContent( &file, true, &errorString, &errorLine, &errorCol ) )
      QMessageBox::warning( 0, tr( "Failed to read templates" ),
			    QString( tr( "Error: %1\n"
					 "Line %2, col %3" ) )
			    .arg( errorString ).arg( errorLine )
			    .arg( errorCol ) );
    else {
      templateNodes = domDoc.elementsByTagName( "template" );
      titles = domDoc.elementsByTagName( "title" );
      templateStrings = domDoc.elementsByTagName( "templateString" );
      defaultPublishStates = domDoc.elementsByTagName( "defaultPublishStatus" );

      int tnc = templateNodes.count();
      if( tnc ) {
	for( int i = 0; i < templateNodes.count(); i++ ) {
	  currentAssocHostListsElement = templateNodes.at( i ).firstChildElement( "associatedHosts" );
	  assocHostLists.append( QStringList() );
	  if( !currentAssocHostListsElement.isNull() ) {
	    assocHostsInTemplate = currentAssocHostListsElement.elementsByTagName( "associatedHost" );
	    for( j = 0; j < assocHostsInTemplate.count(); j++ )
	      assocHostLists[i].append( assocHostsInTemplate.at( j ).toElement().text() );
	  }
	  currentCopyStatusElement = templateNodes.at( i ).firstChildElement( "copyTitleStatus" );
	  if( currentCopyStatusElement.isNull() ) {
	    copyTitleStatusList.append( false );
	  }
	  else {
	    currentCopyStatusElementText = currentCopyStatusElement.text();
	    if( currentCopyStatusElementText.startsWith( "1" ) )
	      copyTitleStatusList.append( true );
	    else
	      copyTitleStatusList.append( false );
	  }
	}
      }

      // qDebug() << "Built list of associated hosts";

      if( titles.size() ) {
	quickpostTemplateActions.clear();
	numTemplates = ( titles.size() >= templateStrings.size() ?
			 titles.size() : templateStrings.size() );
	useDefaultPublishStatus = (defaultPublishStates.size() < numTemplates) ?
	  false : true;

	// qDebug( "Populating template menu" );
	// templateMenu->addSeparator();
	templateTitleList.clear();
	templateList.clear();
	for( t = 0; t < numTemplates; t++ ) {
	  currentTitle = titles.item( t ).toElement().text();
	  currentTemplate = templateStrings.item( t ).toElement().text();
	  currentDefaultPublishStatus = useDefaultPublishStatus ? 
	    defaultPublishStates.item( t ).toElement().text().toInt( &ok ) : 2;
	  templateTitleList.append( currentTitle );
	  templateList.append( currentTemplate );
	  if( ok ) {
	    switch( currentDefaultPublishStatus ) {
	    case 0:
	    case 1:
	      defaultPublishStatusList.append( currentDefaultPublishStatus ); break;
	    default:
	      defaultPublishStatusList.append( 2 );
	    }
	  }
	  else
	    defaultPublishStatusList.append( 2 );
	  
	  ts << titles.item( t ).nodeValue();
	  ts << templateStrings.item( t ).nodeValue();
	  quickpostTemplateActions.append( new QuickpostTemplate( t, currentTitle,
								  currentTemplate, 
								  templateMenu ) );
	  templateMenu->addAction( quickpostTemplateActions[t] );
	  connect( quickpostTemplateActions[t], 
		   SIGNAL( quickpostRequested( int, QString ) ),
		   this, SLOT( quickpostFromTemplate( int, QString ) ) );
        }
	templateMenu->addSeparator();
      }
      /*      else
	      qDebug( "No templates found." );*/
    }
    file.close();
  }
  templateMenu->addAction( configureTemplates );
}

void SysTrayIcon::configureQuickpostTemplates( QWidget *parent )
{
  QDomDocument templateDocument;
  QDomElement quickpostTemplates;
  QString templateFileName;
  QString templateFile;
  QSettings settings;

  /*  QList<QString> titles, templateStrings;
      QStringListModel *titlesModel;*/

  QuickpostTemplateDialog templateDialog( templateTitleList, templateList,
					  defaultPublishStatusList, copyTitleStatusList,
					  assocHostLists, _copyTitle, parent );

  configureTemplates->setEnabled( false );
  configureTemplates->setMenu( 0 );
  if( templateDialog.exec() ) {
    // Set templates menu back up
    templateMenu->clear();
    templateTitleList = templateDialog.templateTitles();
    templateList = templateDialog.templateStrings();
    defaultPublishStatusList = templateDialog.defaultPublishStates();
    copyTitleStatusList = templateDialog.copyTitleStates();
    assocHostLists = templateDialog.assocHostLists();
    quickpostTemplates = templateDocument.createElement( "QuickpostTemplates" );
    int numTemplates = (templateTitleList.size() <= templateList.size()) ?
      templateTitleList.size() : templateList.size();
    quickpostTemplateActions.clear();
    for( int i = 0; i < numTemplates; i++ ) {
      quickpostTemplates.appendChild( templateElement( templateDocument,
						       templateTitleList[i],
						       templateList[i],
						       defaultPublishStatusList[i],
						       copyTitleStatusList[i],
						       assocHostLists[i] ) );

      quickpostTemplateActions.append( new QuickpostTemplate( i,
							      templateTitleList[i],
							      templateList[i],
							      templateMenu ) );
      templateMenu->addAction( quickpostTemplateActions[i] );
      connect( quickpostTemplateActions[i],
	       SIGNAL( quickpostRequested( int, QString ) ),
	       this, SLOT( quickpostFromTemplate( int, QString ) ) );
    }
    templateDocument.appendChild( quickpostTemplates );
    templateMenu->addSeparator();
    configureTemplates->setEnabled( true );
    templateMenu->addAction( configureTemplates );

    // Save template XML file
    templateDocument.insertBefore( templateDocument.createProcessingInstruction( "xml", "version=\"1.0\"" ),
				   templateDocument.firstChild() );
    settings.beginGroup( "account" );
#if defined Q_WS_WIN
#if QT_VERSION < 0x040200
    QString templateFileDir = settings.value( "localStorageDirectory",
					      QString( "%1/QTM blog" ).arg( QDir::homePath() ) ).toString();
      .replace( "/", "\\" );
#else
    QString templateFileDir = QDir::toNativeSeparators( settings.value( "localStorageDirectory",
									QString( "%1/QTM blog" ).arg( QDir::homePath() ) ).toString() );
#endif
#else
    QString templateFileDir = settings.value( "localStorageDirectory",
					      QString( "%1/qtm-blog" ).arg( QDir::homePath() ) ).toString();
#endif
    QDir tfd( templateFileDir );
    if( !tfd.exists() )
      tfd.mkpath( templateFileDir );
    templateFileName = QString( "%1/qptemplates.xml" ).arg( templateFileDir );
    settings.endGroup();
    QFile *templateFile = new QFile( templateFileName );
    if( templateFile->open( QIODevice::WriteOnly ) ) {
      QTextStream templateFileStream( templateFile );
      templateDocument.save( templateFileStream, 4 );
      templateFile->close();
    }
    else
      showMessage( tr( "Error" ),
		   tr( "Could not write to templates file" ), Warning );
    emit quickpostTemplateTitlesUpdated( templateTitles() );
    emit quickpostTemplatesUpdated( templates() );
    templateFile->deleteLater();
  }
  else {
    configureTemplates->setEnabled( true );
  }
}

QDomElement SysTrayIcon::templateElement( QDomDocument &doc,
					  QString &title,
					  QString &templateString,
					  int &defaultPublishStatus,
					  bool &copyTitleStatus,
					  QStringList &assocHosts )
{
  QDomElement returnElement = doc.createElement( "template" );
  QDomElement titleElement = doc.createElement( "title" );
  titleElement.appendChild( QDomText( doc.createTextNode( title ) ) );
  QDomElement templateStringElement = doc.createElement( "templateString" );
  templateStringElement.appendChild( QDomText( doc.createTextNode( templateString ) ) );
  QDomElement defaultPublishStatusElement( doc.createElement( "defaultPublishStatus" ) );
  defaultPublishStatusElement.appendChild( QDomText( doc.createTextNode( QString::number( defaultPublishStatus ) ) ) );
  QDomElement copyTitleElement( doc.createElement( "copyTitleStatus" ) );
  copyTitleElement.appendChild( QDomText( doc.createTextNode( QString::number( copyTitleStatus ? 1 : 0 ) ) ) );
  returnElement.appendChild( titleElement );
  returnElement.appendChild( templateStringElement );
  returnElement.appendChild( defaultPublishStatusElement );
  returnElement.appendChild( copyTitleElement );

  if( assocHosts.count() ) {
    QDomElement associatedHost;
    QDomElement assocHostListsElement = doc.createElement( "associatedHosts" );
    for( int i = 0; i < assocHosts.count(); i++ ) {
      associatedHost = doc.createElement( "associatedHost" );
      associatedHost.appendChild( QDomText( doc.createTextNode( assocHosts[i] ) ) );
      assocHostListsElement.appendChild( associatedHost );
    }
    returnElement.appendChild( assocHostListsElement );
  }

  return returnElement;
}
#endif
#endif
