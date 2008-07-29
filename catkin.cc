/*******************************************************************************

    QTM - Qt-based blog manager
    Copyright (C) 2006 Matthew J Smith

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

// catkin.cc - Main window class for QTM applicaiton

// #define QTM_DEBUG

//#include <QApplication>
//#include <QCoreApplication>
#include <QDesktopWidget>
#include <QRect>
#include <QCursor>
#include <QSettings>
#include <QList>
#include <QMainWindow>
#include <QComboBox>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QSize>
#include <QPoint>
#include <QStackedWidget>
// #include <QSplitter>
#include <QTextBrowser>
#include <QPushButton>
#include <QVBoxLayout>
#include <QAction>
#include <QIcon>
#include <QPixmap>
#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QClipboard>
#include <QDir>
#include <QTemporaryFile>
#include <QTextStream>
#include <QDataStream>
#include <QChar>
#include <QRegExp>
#include <QRegExpValidator>
#include <QDateTime>
#include <QKeySequence>
#include <QFileDialog>
#include <QIODevice>
#include <QMap>
#include <QHash>
#include <QHashIterator>
#include <QWhatsThis>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QFont>
#include <QFontDialog>
#include <QHostInfo>
#include <QProgressDialog>
#include <QTextEdit>
#if QT_VERSION >= 0x040400
#include <QPlainTextEdit>
#endif

#include "XmlRpcHandler.h"
#include "SafeHttp.h"
#include "Application.h"
#include "catkin.h"
#ifdef USE_SYSTRAYICON
#include "SysTrayIcon.h"
#endif

#include "qtm_version.h"

#include <QtAlgorithms>
#include <QtNetwork>
#include <QtXml>

#include "addimg.xpm"
#include "addlink.xpm"
#include "blog-this.xpm"
#include "fileopen.xpm"
#include "filesave.xpm"
#include "more.xpm"
#include "paragraph.xpm"
#include "preview.xpm"
#include "textbold.xpm"
#include "textital.xpm"
#include "textul.xpm"
#include "bquote.xpm"
#include "addtag.xpm"
#include "addctag.xpm"
#include "remtag.xpm"

#define EDITOR ed

#ifdef Q_WS_WIN
#define PROPERSEPS( x ) x.replace( "/", "\\" )
#else
#define PROPERSEPS( x ) x
#endif

Catkin::Catkin( QDomDocument &dd, QString &cid, QWidget *parent )
  : QMainWindow( parent )
{
  QFont f, g, h;

  setAttribute( Qt::WA_QuitOnClose );
  setAttribute( Qt::WA_DeleteOnClose );

  initialChangeBlog = false;
  doUiSetup();

  accountsDom = dd;

  readSettings();
  checkForEmptySettings();

  if( editorFontString != "" ) {
      f.fromString( editorFontString );
      EDITOR->setFont( f );
  } else {
      f = EDITOR->font();
      editorFontString = f.toString();
  }
  if( previewFontString != "" ) {
      g.fromString( previewFontString );
      previewWindow->setFont( g );
  } else {
      g = previewWindow->font();
      previewFontString = g.toString();
  }
  if( consoleFontString != "" ) {
    h.fromString( consoleFontString );
    console->setFont( h );
  } else {
    h = console->font();
    consoleFontString = h.toString();
  }

  /*  if( isBloggerBlog ) {
    cw.chComments->setEnabled( false );
    cw.chTB->setEnabled( false );
    } else { */
  cw.chComments->setEnabled( true );
  cw.chComments->setEnabled( true );
  cw.chComments->setCheckState( allowComments ? Qt::Checked :
				Qt::Unchecked );
  cw.chTB->setCheckState( allowTB ? Qt::Checked : Qt::Unchecked );
    //  }

  QDomNodeList accts = accountsDom.documentElement().elementsByTagName( "account" );
  int a = accts.count();
  for( int i = 0; i < a; i++ ) {
    if( accts.at( i ).toElement().attribute( "id" ) == cid ) {
      currentAccountElement = accts.at( i ).toElement();
      break;
    }

    // If it doesn't find the ID
    if( i == a-1 ) {
      if( a )
	currentAccountElement = accountsDom.firstChildElement( "QTMAccounts" )
	  .firstChildElement( "account" );
    }
  }

  if( a ) {
    populateAccountList();
    populateBlogList();
  }

  mainStack->setCurrentIndex( edID );
}

Catkin::Catkin( bool noRefreshBlogs, QWidget *parent )
  : QMainWindow( parent )
{
  //qDebug() << "Starting window";
  QFont f, g, h;

  QDomElement detailElem, nameElem, serverElem, locElem, loginElem, pwdElem, attribElem;
  QSettings settings;

  setAttribute( Qt::WA_QuitOnClose );
  setAttribute( Qt::WA_DeleteOnClose );
  doUiSetup();

  readSettings();
  checkForEmptySettings();

  if( editorFontString != "" ) {
      f.fromString( editorFontString );
      EDITOR->setFont( f );
  } else {
      f = EDITOR->font();
      editorFontString = f.toString();
  }
  if( previewFontString != "" ) {
      g.fromString( previewFontString );
      previewWindow->setFont( g );
  } else {
      g = previewWindow->font();
      previewFontString = g.toString();
  }
  if( consoleFontString != "" ) {
    h.fromString( consoleFontString );
    console->setFont( h );
  } else {
    h = console->font();
    consoleFontString = h.toString();
  }

  cw.chComments->setEnabled( true );
  cw.chComments->setCheckState( allowComments ? Qt::Checked :
				Qt::Unchecked );
  cw.chTB->setCheckState( allowTB ? Qt::Checked : Qt::Unchecked );

  handleEnableCategories();

  if( !location.isEmpty() && !noRefreshBlogs ) {
    //qDebug() << "Reading in XML";
    QFile accountsXmlFile( PROPERSEPS( QString( "%1/qtmaccounts.xml" ).arg( localStorageDirectory ) ) );
    if( !accountsDom.setContent( &accountsXmlFile ) ) {
#ifndef NO_DEBUG_OUTPUT
      qDebug() << "Can't read the XML";
#endif
      accountsXmlFile.close();
      accountsElement = accountsDom.createElement( "QTMAccounts" );
      currentAccountElement = accountsDom.createElement( "account" );
      currentAccountElement.setAttribute( "id", "default" );

      if( !server.isEmpty() ) {
	qDebug() << "copying details to new default element";
	detailElem = accountsDom.createElement( "details" );
	nameElem = accountsDom.createElement( "title" );
	nameElem.appendChild( accountsDom.createTextNode( tr( "Default account" ) ) );
	serverElem = accountsDom.createElement( "server" );
	serverElem.appendChild( accountsDom.createTextNode( server ) );
	locElem = accountsDom.createElement( "location" );
	locElem.appendChild( accountsDom.createTextNode( location ) );
	loginElem = accountsDom.createElement( "login" );
	loginElem.appendChild( accountsDom.createTextNode( login ) );
	pwdElem = accountsDom.createElement( "password" );
	pwdElem.appendChild( accountsDom.createTextNode( password ) );
	detailElem.appendChild( nameElem );
	detailElem.appendChild( serverElem );
	detailElem.appendChild( locElem );
	detailElem.appendChild( loginElem );
	detailElem.appendChild( pwdElem );
	currentAccountElement.appendChild( detailElem );

	// Delete the old account from the settings
	settings.beginGroup( "account" );
	settings.remove( "server" );
	settings.remove( "location" );
	settings.remove( "login" );
	settings.remove( "password" );
	settings.endGroup();

	// Now transfer the attributes to the default accounts
	QStringList attribs( accountAttributes.keys() );
	Q_FOREACH( QString s, attribs ) {
	  if( *(accountAttributes[s]) ) {
	    attribElem = accountsDom.createElement( "attribute" );
	    attribElem.setAttribute( "name", s );
	    detailElem.appendChild( attribElem );
	  }
	}
      }

      accountsElement.appendChild( currentAccountElement );
      accountsDom.appendChild( accountsElement );
      accountsDom.insertBefore( accountsDom.createProcessingInstruction( "xml", "version=\"1.0\"" ),
				accountsDom.firstChild() );
      QHostInfo::lookupHost( server, this, SLOT( handleInitialLookup( QHostInfo ) ) );
    }
    else {
      int i;

      qDebug() << "server is empty";
      QSettings settings;
      lastAccountID = settings.value( "account/lastAccountID", "" ).toString();
      QDomNodeList accountsList = accountsDom.documentElement()
	.elementsByTagName( "account" );
      QDomElement thisTitleElem;
      cw.cbAccountSelector->clear();

      for( i = 0; i < accountsList.count(); i++ ) {
	thisTitleElem = accountsList.at( i ).toElement().firstChildElement( "details" )
	  .firstChildElement( "title" );
	if( !thisTitleElem.isNull() )
	  cw.cbAccountSelector->addItem( thisTitleElem.text(),
					 accountsList.at( i ).toElement().attribute( "id" ) );
	else
	  cw.cbAccountSelector->addItem( tr( "Unnamed account" ),
					 accountsList.at( i ).toElement().attribute( "id" ).isEmpty() ?
					 QString( "noid_%1" ).arg( i ) :
					 accountsList.at( i ).toElement().attribute( "id" ) );
      }

      qDebug() << "checking for last account ID";
      for( i = 0; i < accountsList.count(); i++ ) {
	if( accountsList.at( i ).toElement().attribute( "id" ) == lastAccountID ) {
	  qDebug() << "found it";
	  currentAccountElement = accountsList.at( i ).toElement();
	  currentAccountId = currentAccountElement.attribute( "id" );
	  cw.cbAccountSelector->setCurrentIndex( i );
	  populateBlogList();
	  connect( cw.cbAccountSelector, SIGNAL( activated( int ) ),
		   this, SLOT( changeAccount( int ) ) );
	  break;
	}
	// If it reaches the end of the loop with no joy
	if( i == accountsList.count()-1 ) {
 	  qDebug() << "using first account";
	  currentAccountElement = accountsDom.documentElement()
	    .firstChildElement( "account" );
	  populateBlogList();
	  connect( cw.cbAccountSelector, SIGNAL( activated( int ) ),
		   this, SLOT( changeAccount( int ) ) );
	}
      }
      accountsXmlFile.close();
    }
  }

  mainStack->setCurrentIndex( edID );
}

Catkin::Catkin( QString newPost, QWidget *parent )
  : QMainWindow( parent )
{
  QFont f, g, h;

  setAttribute( Qt::WA_DeleteOnClose );
  doUiSetup();

  readSettings();
  checkForEmptySettings();

  if( editorFontString != "" ) {
      f.fromString( editorFontString );
      EDITOR->setFont( f );
  } else {
      f = EDITOR->font();
      editorFontString = f.toString();
  }
  if( previewFontString != "" ) {
      g.fromString( previewFontString );
      previewWindow->setFont( g );
  } else {
      g = previewWindow->font();
      previewFontString = g.toString();
  }
  if( consoleFontString != "" ) {
    h.fromString( consoleFontString );
    console->setFont( h );
  } else {
    h = console->font();
    consoleFontString = h.toString();
  }

  cw.chComments->setEnabled( true );
  cw.chComments->setEnabled( true );
  cw.chComments->setCheckState( allowComments ? Qt::Checked :
				Qt::Unchecked );
  cw.chTB->setCheckState( allowTB ? Qt::Checked : Qt::Unchecked );

  handleEnableCategories();

  if( !location.isEmpty() ) {
    QFile accountsXmlFile( PROPERSEPS( QString( "%1/qtmaccounts.xml" ).arg( localStorageDirectory ) ) );
    if( !accountsDom.setContent( &accountsXmlFile ) ) {
#ifndef NO_DEBUG_OUTPUT
      qDebug() << "Can't read the XML";
#endif
      accountsXmlFile.close();
      accountsElement = accountsDom.createElement( "QTMAccounts" );
      currentAccountElement = accountsDom.createElement( "account" );
      accountsElement.appendChild( currentAccountElement );
      accountsDom.appendChild( accountsElement );
      accountsDom.insertBefore( accountsDom.createProcessingInstruction( "xml", "version=\"1.0\"" ),
				accountsDom.firstChild() );
      QHostInfo::lookupHost( server, this, SLOT( handleInitialLookup( QHostInfo ) ) );
    }
    else {
      accountsXmlFile.close();
      currentAccountElement = accountsDom.firstChildElement( "QTMAccounts" )
	  .firstChildElement( "account" );
      populateBlogList();
    }
  }
    //    QHostInfo::lookupHost( server, this, SLOT( handleInitialLookup( QHostInfo ) ) );

  EDITOR->setPlainText( newPost );
  mainStack->setCurrentIndex( edID );

}

Catkin::~Catkin()
{
  deleteLater();
}

#ifdef USE_SYSTRAYICON
void Catkin::setSTI( SysTrayIcon *_sti )
{
  sti = _sti;
}
#endif

void Catkin::doUiSetup()
{
  qApp->setWindowIcon( QIcon( QPixmap( ":/images/qtm-logo1.png" ) ) );

#ifdef USE_SYSTRAYICON
  sti = 0;
#endif

#if QT_VERSION >= 0x040300
  setUnifiedTitleAndToolBarOnMac( true );
#endif

  qtm = qobject_cast<Application *>( qApp );

  loadedEntryBlog = 999;
  noAutoSave = false;
  noAlphaCats = false;
  setAttribute( Qt::WA_QuitOnClose );

  userAgentString = QString( "QTM/%1" ).arg( QTM_VERSION );

  QCoreApplication::setOrganizationName( "Catkin Project" );
  QCoreApplication::setOrganizationDomain( "catkin.blogistan.co.uk" );
  QCoreApplication::setApplicationName( "QTM" );

  //bloggerTitleFormatStrings << "[none]" << "h1" << "h2" << "h3" << "h4" << "h5" << "h6"
  //			    << "strong" << "em";

  ui.setupUi( this );
  //ui.dwExcerpt->setVisible( false );
  // blogType = 0;
  currentHttpBusiness = 0;
  entryBlogged = false;
  http = new SafeHttp;

  // Setup main signals and slots

  //ui.action_Open->setEnabled( false );    // These actions are
  //ui.action_Save->setEnabled( false );    // greyed out as they
  //ui.actionSave_As->setEnabled( false );  // are not yet
  ui.actionPrint->setEnabled( false );    // implemented
  //ui.action_Close->setEnabled( false );
  ui.actionClose_and_Delete->setEnabled( false );

  connect( ui.actionAbout_Qt, SIGNAL( triggered( bool ) ),
	   qApp, SLOT( aboutQt() ) );
  connect( ui.action_About, SIGNAL( triggered( bool ) ),
	   this, SLOT( about() ) );
  connect( ui.actionE_xit, SIGNAL( triggered( bool ) ),
	   this, SLOT( doQuit() ) );
  connect( ui.action_Close, SIGNAL( triggered( bool ) ),
	   this, SLOT( close() ) );
  connect( ui.actionAccounts, SIGNAL( triggered( bool ) ),
	   this, SLOT( getAccounts() ) );
  connect( ui.actionPr_eferences, SIGNAL( triggered( bool ) ),
	   this, SLOT( getPreferences() ) );
  connect( ui.actionRefresh_blog_list, SIGNAL( triggered( bool ) ),
	   this, SLOT( refreshBlogList() ) );
  connect( ui.actionRefresh_categories, SIGNAL( triggered( bool ) ),
	   this, SLOT( refreshCategories() ) );
#if QT_VERSION >= 0x040200
#if defined USE_SYSTRAYICON
  connect( ui.actionQuickpost_Templates, SIGNAL( triggered( bool ) ),
	   this, SLOT( configureQuickpostTemplates() ) );
#else
  ui.actionQuickpost_Templates->setVisible( false );
  #endif
#else
  ui.actionQuickpost_Templates->setVisible( false );
#endif

  // Set up icons

  ui.action_Blog_this->setIcon( QIcon( QPixmap( blogThisIcon_xpm ) ) );
  ui.actionP_review->setIcon( QIcon( QPixmap( previewIcon_xpm ) ) );
  ui.action_Open->setIcon( QIcon( QPixmap( fileopen ) ) );
  ui.action_Save->setIcon( QIcon( QPixmap( filesave ) ) );
  // ui.actionPrint->setIcon( QIcon( QPixmap( fileprint ) ) );
  ui.action_Bold->setIcon( QIcon( QPixmap( mini_bold_xpm ) ) );
  ui.action_Italic->setIcon( QIcon( QPixmap( mini_ital_xpm ) ) );
  ui.actionU_nderline->setIcon( QIcon( QPixmap( underline_xpm ) ) );
  ui.actionBlockquote->setIcon( QIcon( QPixmap( bquote_xpm ) ) );
  ui.actionP_aragraph->setIcon( QIcon( QPixmap( paraIcon_xpm ) ) );
  ui.action_Link->setIcon( QIcon( QPixmap( linkIcon_xpm ) ) );
  ui.actionI_mage->setIcon( QIcon( QPixmap( imgIcon_xpm ) ) );
  ui.action_More->setIcon( QIcon( QPixmap( more_xpm ) ) );
  ui.actionAdd_tag->setIcon( QIcon( QPixmap( addtag_xpm ) ) );
  ui.actionAdd_tag_from_clipboard->setIcon( QIcon( QPixmap( addctag_xpm ) ) );
  ui.action_Remove_tag->setIcon( QIcon( QPixmap( remtag_xpm ) ) );

  // Action key sequences

  ui.actionStop_this_job->setShortcut( QKeySequence::fromString( "Ctrl+A" ) );
  ui.action_Bold->setShortcut( QKeySequence::fromString( "Ctrl+B" ) );
  ui.action_Italic->setShortcut( QKeySequence::fromString( "Ctrl+I" ) );
  /*ui.action_Cut->setShortcut( QKeySequence::fromString( "Ctrl+X" ) );
  ui.actionC_opy->setShortcut( QKeySequence::fromString( "Ctrl+C" ) );
  ui.actionPaste->setShortcut( QKeySequence::fromString( "Ctrl+V" ) ); */
  ui.actionI_mage->setShortcut( QKeySequence::fromString( "Ctrl+J" ) );
  ui.action_Link->setShortcut( QKeySequence::fromString( "Ctrl+L" ) );
  ui.action_Auto_link->setShortcut( QKeySequence::fromString( "Shift+Ctrl+L" ) );
  ui.actionLink_from_C_lipboard->setShortcut( QKeySequence::fromString( "Ctrl+U" ) );
  /*ui.action_Undo->setShortcut( QKeySequence::fromString( "Ctrl+Z" ) );
    ui.action_Redo->setShortcut( QKeySequence::fromString( "Ctrl+Y" ) ); */
  ui.action_View_Console->setShortcut( QKeySequence::fromString( "Shift+Ctrl+V" ) );
  ui.action_BES->setShortcut( QKeySequence( "Ctrl+1" ) );
  ui.actionC_ategories->setShortcut( QKeySequence( "Ctrl+2" ) );
  ui.action_Show_Excerpt_window->setShortcut( QKeySequence( "Ctrl+3" ) );
  ui.actionTechnorati_tags->setShortcut( QKeySequence( "Ctrl+4" ) );
  ui.actionTrackback_pings->setShortcut( QKeySequence( "Ctrl+5" ) );
  ui.actionAdd_tag->setShortcut( QKeySequence( "Ctrl++" ) );
  ui.actionAdd_tag_from_clipboard->setShortcut( QKeySequence( "Shift+Ctrl++" ) );
  ui.action_Remove_tag->setShortcut( QKeySequence( "Ctrl+-" ) );

  // Add actions to toolbar

  ui.toolBar->addAction( ui.action_Open );
  ui.toolBar->addAction( ui.action_Save );
  ui.toolBar->addSeparator();
  ui.toolBar->addAction( ui.action_Bold );
  ui.toolBar->addAction( ui.action_Italic );
  ui.toolBar->addAction( ui.actionU_nderline );
  ui.toolBar->addAction( ui.actionBlockquote );
  ui.toolBar->addAction( ui.action_Link );
  ui.toolBar->addAction( ui.actionI_mage );
  ui.toolBar->addAction( ui.action_More );
  ui.toolBar->addSeparator();
  ui.toolBar->addAction( ui.actionP_review );
  ui.toolBar->addAction( ui.action_Blog_this );
  ui.toolBar->addSeparator();
  ui.toolBar->setAllowedAreas( Qt::TopToolBarArea | Qt::BottomToolBarArea );

  // Connect actions to slots

  //  connect( ui.action_Open, SIGNAL( triggered( bool ) ), this, SLOT( abort() ) );
  connect( ui.action_New, SIGNAL( triggered( bool ) ),
	   this, SLOT( newDoc() ) );
  connect( ui.action_Open, SIGNAL( triggered( bool ) ),
	   this, SLOT( choose() ) );
  connect( ui.action_Save, SIGNAL( triggered( bool ) ),
	   this, SLOT( save() ) );
  connect( ui.actionSave_As, SIGNAL( triggered( bool ) ),
	   this, SLOT( saveAs() ) );
  connect( ui.action_Upload, SIGNAL( triggered( bool ) ),
	   this, SLOT( uploadFile() ) );
  connect( ui.actionStop_this_job, SIGNAL( triggered( bool ) ),
	   this, SLOT( stopThisJob() ) );
  connect( ui.actionSend_categories, SIGNAL( triggered( bool ) ),
	   this, SLOT( updatePostCategories() ) );
  connect( ui.actionSave_blogs, SIGNAL( triggered( bool ) ),
	   this, SLOT( saveBlogs() ) );

#ifdef Q_WS_MAC
  // The action to view the toolbar is not required on the Mac, because the grey
  // lozenge at top right shows and hides the toolbar.
  ui.actionView_Toolbar->setVisible( false );
  ui.toolBar->setVisible( true );
#else
  connect( ui.actionView_Toolbar, SIGNAL( toggled( bool ) ),
           ui.toolBar, SLOT( setVisible( bool ) ) );
#endif

  connect( ui.action_Bold, SIGNAL( triggered( bool ) ),
           this, SLOT( makeBold() ) );
  connect( ui.action_Italic, SIGNAL( triggered( bool ) ),
           this, SLOT( makeItalic() ) );
  connect( ui.actionU_nderline, SIGNAL( triggered( bool ) ),
           this, SLOT( makeUnderline() ) );
  connect( ui.actionBlockquote, SIGNAL( triggered( bool ) ),
	   this, SLOT( makeBlockquote() ) );
  connect( ui.action_More, SIGNAL( triggered( bool ) ),
           this, SLOT( insertMore() ) );
  connect( ui.action_Link, SIGNAL( triggered( bool ) ),
	   this, SLOT( insertLink() ) );
  connect( ui.actionLink_from_C_lipboard, SIGNAL( triggered( bool ) ),
           this, SLOT( insertLinkFromClipboard() ) );
  connect( ui.action_Self_link, SIGNAL( triggered( bool ) ),
	   this, SLOT( insertSelfLink() ) );
  connect( ui.action_Auto_link, SIGNAL( triggered( bool ) ),
	   this, SLOT( insertAutoLink() ) );
  connect( ui.actionI_mage, SIGNAL( triggered( bool ) ),
	   this, SLOT( insertImage() ) );
  connect( ui.actionIma_ge_from_Clipboard, SIGNAL( triggered( bool ) ),
           this, SLOT( insertImageFromClipboard() ) );
  connect( ui.action_Cut,  SIGNAL( triggered( bool ) ),   this, SLOT( cut() ) );
  connect( ui.actionC_opy, SIGNAL( triggered( bool ) ),   this, SLOT( copy() ) );
  connect( ui.actionCopy_upload_location, SIGNAL( triggered( bool ) ),
	   this, SLOT( copyURL() ) );
  connect( ui.actionPaste, SIGNAL( triggered( bool ) ),   this, SLOT( paste() ) );
  connect( ui.actionMarked_paragraphs, SIGNAL( triggered( bool ) ),
	   this, SLOT( pasteAsMarkedParagraphs() ) );
  connect( ui.actionBlockquote_2, SIGNAL( triggered( bool ) ),
	   this, SLOT( pasteAsBlockquote() ) );
  connect( ui.actionMarkdown_blockquote, SIGNAL( triggered( bool ) ),
	   this, SLOT( pasteAsMarkdownBlockquote() ) );
  connect( ui.actionUnordered_list_2, SIGNAL( triggered( bool ) ),
	   this, SLOT( pasteAsUnorderedList() ) );
  connect( ui.actionOrdered_list, SIGNAL( triggered( bool ) ),
	   this, SLOT( pasteAsOrderedList() ) );
  connect( ui.action_Undo, SIGNAL( triggered( bool ) ),   this, SLOT( undo() ) );
  connect( ui.action_Redo, SIGNAL( triggered( bool ) ),   this, SLOT( redo() ) );
  connect( ui.actionUnordered_list, SIGNAL( triggered( bool ) ),
	   this, SLOT( makeUnorderedList() ) );
  connect( ui.action_Ordered_list, SIGNAL( triggered( bool ) ),
	   this, SLOT( makeOrderedList() ) );
#if QT_VERSION >= 0x040200
  ui.action_Font->setVisible( false );
  ui.actionPreview_font->setVisible( false );
  ui.actionConsole_font->setVisible( false );
#else
  connect( ui.action_Font, SIGNAL( triggered( bool ) ),   this, SLOT( doFont() ) );
  connect( ui.actionPreview_font, SIGNAL( triggered( bool ) ),
	   this, SLOT( doPreviewFont() ) );
  connect( ui.actionConsole_font, SIGNAL( triggered( bool ) ),
	   this, SLOT( doConsoleFont() ) );
#endif
  connect( ui.actionP_review, SIGNAL( toggled( bool ) ),
	   this, SLOT( doPreview( bool ) ) );
  connect( ui.action_BES, SIGNAL( triggered( bool ) ),
	   this, SLOT( doViewBasicSettings() ) );
  connect( ui.actionC_ategories, SIGNAL( triggered( bool ) ),
	   this, SLOT( doViewCategories() ) );
  connect( ui.action_Show_Excerpt_window, SIGNAL( triggered( bool ) ),
	   this, SLOT( doViewExcerpt() ) );
  connect( ui.actionTechnorati_tags, SIGNAL( triggered( bool ) ),
	   this, SLOT( doViewTechTags() ) );
  connect( ui.actionTrackback_pings, SIGNAL( triggered( bool ) ),
	   this, SLOT( doViewTBPings() ) );
  // Connect signals related to Technorati tags
  connect( ui.actionAdd_tag, SIGNAL( triggered( bool ) ),
	   this, SLOT( addTechTag() ) );
  connect( ui.actionAdd_tag_from_clipboard, SIGNAL( triggered( bool ) ),
	   this, SLOT( addClipTag() ) );
  connect( ui.action_Remove_tag, SIGNAL( triggered( bool ) ),
	   this, SLOT( removeTechTag() ) );
  // Connect signals related to Trackback pings
  connect( ui.actionAdd_trackback_ping, SIGNAL( triggered( bool ) ),
	   this, SLOT( addTBPing() ) );
  connect( ui.actionAdd_ping_from_clip_board, SIGNAL( triggered( bool ) ),
	   this, SLOT( addClipTBPing() ) );
  connect( ui.actionRe_move_ping, SIGNAL( triggered( bool ) ),
	   this, SLOT( removeTBPing() ) );

  connect( ui.action_Blog_this, SIGNAL( triggered( bool ) ),
	   this, SLOT( blogThis() ) );
  connect( ui.action_What_s_this, SIGNAL( triggered( bool ) ),
	   this, SLOT( doWhatsThis() ) );

  // Set up the main layout

  /*mainSplitter = new QSplitter( this );
    mainSplitter->setHandleWidth( 0 ); */
  cWidget = new QWidget( this );

  leftWidget = new QWidget( cWidget );
  mainWindowLayout = new QHBoxLayout;
  mainWindowLayout->setMargin( 0 );
  mainWindowLayout->setMargin( 5 );
  cw.setupUi( leftWidget );
  // mainSplitter->addWidget( leftWidget );

  cw.cbBlogSelector->setMaxVisibleItems( 10 );
  cw.cbMainCat->setMaxVisibleItems( 10 );
  cw.lwTags->addAction( ui.actionAdd_tag );
  cw.lwTags->addAction( ui.actionAdd_tag_from_clipboard );
  cw.lwTags->addAction( ui.action_Remove_tag );
  cw.lwTBPings->addAction( ui.actionAdd_trackback_ping );
  cw.lwTBPings->addAction( ui.actionAdd_ping_from_clip_board );
  cw.lwTBPings->addAction( ui.actionRe_move_ping );

#ifdef Q_WS_MAC
  cw.lwOtherCats->setWhatsThis( tr( "Secondary categories, if your blog system supports "
				    "them.  To highlight more than one category, press "
				    "Command and click the mouse, or the left mouse button." ) );
#endif

  tagValidator = new QRegExpValidator( QRegExp( "([a-zA-Z0-9\\.%]+[\\+ ])*[a-zA-Z0-9\\.%]+" ), this );
  cw.leAddTag->setValidator( tagValidator );
  cw.cbDoTB->hide();

  // Set up main stack widget

  mainStack = new QStackedWidget( cWidget );
  previewWindow = new QTextBrowser( mainStack );
  previewWindowID = mainStack->addWidget( previewWindow );
  // previousRaisedLSWidget = previewWindowID;

  // Set up console
  console = new TEXTEDIT( mainStack );
  consoleID = mainStack->addWidget( console );
  console->setReadOnly( true );
  connect( ui.actionClear_console, SIGNAL( triggered( bool ) ),
	   console, SLOT( clear() ) );

  // Set up editor widget
  ed = new TEXTEDIT( mainStack );
#if QT_VERSION < 0x040400
  EDITOR->setAcceptRichText( false );
#endif
  edID = mainStack->addWidget( ed );
  EDITOR->setReadOnly( false );
  previousRaisedLSWidget = edID;
  mainWindowLayout->addWidget( leftWidget, 3 );
  mainWindowLayout->addWidget( mainStack, 6 );
  mainWindowLayout->setSpacing( 1 );

  // Set up search widget
  mainWindowLayoutWithSearch = new QVBoxLayout( cWidget );
  searchWidget = new QijSearchWidget( EDITOR, this );
  searchWidget->hide();
  mainWindowLayoutWithSearch->setSpacing( 1 );
#if QT_VERSION >= 0x040300
  mainWindowLayoutWithSearch->setContentsMargins( 5, 5, 5, 5 );
#else
  mainWindowLayoutWithSearch->setMargin( 5 );
#endif
  mainWindowLayoutWithSearch->addLayout( mainWindowLayout );
  mainWindowLayoutWithSearch->addWidget( searchWidget );
  connect( ui.action_Find, SIGNAL( triggered( bool ) ),
	   this, SLOT( handleFind() ) );
  connect( ui.actionFind_again, SIGNAL( triggered( bool ) ),
	   searchWidget, SLOT( findAgain() ) );
  ui.action_Find->setShortcut( QKeySequence::fromString( "Ctrl+F" ) );
  ui.actionFind_again->setShortcut( QKeySequence::fromString( "Ctrl+G" ) );
  cWidget->setLayout( mainWindowLayoutWithSearch );
  setCentralWidget( cWidget );

  connect( cw.pbRefresh, SIGNAL( clicked( bool ) ),
    this, SLOT( refreshCategories() ) );
  connect( cw.pbRefresh, SIGNAL( clicked( bool ) ),
	   cw.cbMainCat, SLOT( clear() ) );
  connect( cw.pbRefresh, SIGNAL( clicked( bool ) ),
	   cw.lwOtherCats, SLOT( clear() ) );
  connect( ui.action_View_Console, SIGNAL( toggled( bool ) ),
           this, SLOT( handleConsole( bool ) ) );
  connect( cw.leTitle, SIGNAL( editingFinished() ),
	   this, SLOT( changeCaptionAfterTitleChanged() ) );
  connect( cw.leAddTag, SIGNAL( returnPressed() ),
	   this, SLOT( addTechTagFromAddButton() ) );
  connect( cw.tbAddTag, SIGNAL( clicked() ),
	   this, SLOT( addTechTagFromLineEdit() ) );
  connect( cw.leTBPingURL, SIGNAL( returnPressed() ),
	   this, SLOT( addTBPingFromLineEdit() ) );
  connect( cw.tbTBAdd, SIGNAL( clicked() ),
	   this, SLOT( addTBPingFromLineEdit() ) );

  // Initialise recent file actions

  recentFilesMenu = new QMenu( this );
  noRecentFilesAction = new QAction( tr( "No recent files" ), this );
  noRecentFilesAction->setVisible( true );
  noRecentFilesAction->setEnabled( false );
  recentFilesMenu->addAction( noRecentFilesAction );
  ui.actionOpen_recent->setMenu( recentFilesMenu );
  for( int i = 0; i < 10; ++i ) {
    recentFileActions[i] = new QAction( this );
    recentFilesMenu->addAction( recentFileActions[i] );
    connect( recentFileActions[i], SIGNAL( triggered() ),
	     this, SLOT( openRecentFile() ) );
  }

  cw.cbPageSelector->setCurrentIndex( 0 );
  cw.stackedWidget->setCurrentIndex( 0 );

  pbCopyURL = new QPushButton( this );
  pbCopyURL->setText( tr( "Copy URL" ) );
  pbCopyURL->setWhatsThis( tr( "Copy the location of the last uploaded file." ) );
  statusBar()->addPermanentWidget( pbCopyURL );
  connect( pbCopyURL, SIGNAL( clicked() ), this, SLOT( copyURL() ) );
  pbCopyURL->hide();

  dirtyIndicator = new QLabel( this );
  dirtyIndicator->setPixmap( QPixmap( filesave ) );
  statusBar()->addPermanentWidget( dirtyIndicator );
  connect( EDITOR, SIGNAL( textChanged() ), this, SLOT( dirtify() ) );
  dirtyIndicator->hide();

  // Set up hash of entry attributes
  accountAttributes["categoriesEnabled"] = &categoriesEnabled;
  accountAttributes["postDateTime"] = &postDateTime;
  accountAttributes["comments"] = &allowComments;
  accountAttributes["trackback"] = &allowTB;

  accountStrings["server"] = &server;
  accountStrings["location"] = &location;
  accountStrings["port"] = &port;
  accountStrings["login"] = &login;
  accountStrings["password"] = &password;

  setWindowModified( false );
  entryEverSaved = false;
  cleanSave = false;
  loadAutoLinkDictionary();
}

bool Catkin::handleArguments()
{
  bool rv = true;
  int i;
  Catkin *c = 0;
  Catkin *d = 0;
  QStringList failedFiles;
  QStringList args = QApplication::arguments();

  //  qDebug() << "handling arguments: "<< args.size();

  if( args.size() > 1 ) {
    //qDebug() << "handling arguments";
    for( i = 1; i < args.size(); i++ ) {
      if( c ) // if there is a current new window
	d = c;
      //qDebug() << "handling an argument";
      QString curid = currentAccountElement.attribute( "id" );
      c = new Catkin( accountsDom, curid );
      if( c->load( args.at( i ) ) ) {
#ifdef USE_SYSTRAYICON
	c->setSTI( sti );
#endif
	if( d ) // if there's an old window
	  positionWidget( c, d );
	c->show();
	rv = false;
      }
      else {
	/*QMessageBox::showMessage( 0, QObject::tr( "Error" ),
	  QObject::tr( "Could not load %1." ).arg( args.at( i ) ),
	  QMessageBox::Cancel, QMessageBox::NoButton ); */
	failedFiles.append( args.at( i ) );
      }
      if( failedFiles.size() ) {
	if( failedFiles.size() < args.size()-1 ) {
	  QMessageBox::information( this, tr( "Error" ),
				    tr( "Could not load the following:\n\n%1" )
				    .arg( failedFiles.join( "\n" ) ),
				    QMessageBox::Ok );
	  rv = false;
	}
	else {
	  if( QMessageBox::question( 0, tr( "Error" ),
				     tr( "Could not load the following:\n\n%1" )
				     .arg( failedFiles.join( "\n" ) ),
				     tr( "&Open blank window" ), tr( "E&xit" ), 0 ) )
	    QApplication::exit();
	  else
	    rv = false;
	}
      }
    }
  }

  return rv;
}

void Catkin::about() // slot
{
  QDialog about_box( this );
  Ui::AboutBox abui;
  abui.setupUi( &about_box );
  abui.label->setText( abui.label->text().replace( "(VERSION)", QTM_VERSION ) );
  about_box.exec();
}

void Catkin::newDoc()
{
  //Catkin *ed = new Catkin( usersBlogs, categoryList, currentBlog, 0 );
  QString curid = currentAccountElement.attribute( "id" );
  Catkin *ed = new Catkin( accountsDom, curid );
  ed->setWindowTitle( tr( "QTM - new entry [*]" ) );
#ifdef USE_SYSTRAYICON
  ed->setSTI( sti );
#endif

  positionWidget( ed, this );

  ed->show();
}

void Catkin::positionWidget( QWidget *w, QWidget *refWidget )
{
  QDesktopWidget *dw = QApplication::desktop();
  QRect r = dw->availableGeometry();
  QRect g = refWidget->geometry(); // of the reference widget

  if( (g.right() + 30) >= (r.right() + 30) &&
      (g.bottom() + 30 ) >= (r.bottom() + 30 ) )
    w->move( r.left(), r.top() );
  else
    w->move( refWidget->x() + 30, refWidget->y() + 30 );
}

void Catkin::changeCaptionAfterTitleChanged()
{
  if( cw.leTitle->text().isEmpty() )
    setWindowTitle( tr( "QTM - new entry [*]" ) );
  else
    setWindowTitle( QString( "%1 - QTM [*]" ).arg( cw.leTitle->text().trimmed() ) );
}

void Catkin::closeEvent( QCloseEvent *event )
{
  QSettings settings;

#ifndef NO_DEBUG_OUTPUT
  qDebug( "close event" );
#endif

  if( isWindowModified() ) {
    if( !saveCheck() ) {
      event->ignore();
      qApp->setQuitOnLastWindowClosed( false );
    }
    else {
      settings.setValue( "account/lastAccountId", currentAccountId );
      event->accept();
    }
  } else {
    writeSettings();
    settings.setValue( "account/lastAccountId", currentAccountId );
    event->accept();
  }
}

void Catkin::showEvent( QShowEvent *event )
{
  // If the document is empty, the window unedited and the entry never saved,
  // chances are it's new
  if( EDITOR->document()->isEmpty() &&
      !dirtyIndicator->isVisible() &&
      !event->spontaneous() &&
      !entryEverSaved )
    cw.leTitle->setFocus( Qt::ActiveWindowFocusReason );

  QMainWindow::showEvent( event );
}

void Catkin::doQuit()
{
#ifndef NO_DEBUG_OUTPUT
  int i = QApplication::topLevelWidgets().size();
  qDebug() << i << " top level widgets";
#endif

  qApp->setQuitOnLastWindowClosed( true );
  qApp->closeAllWindows();
}

void Catkin::checkForEmptySettings()
{
  //Check if this is a brand new user
  if( localStorageDirectory.isEmpty() || server.isEmpty() ) {
    if( QMessageBox::question( 0, tr( "Welcome to QTM" ),
			       tr( "You do not have any preferences set, and QTM "
				   "needs to know where to find your blog, and where "
				   "to store your data.\n\n"
				   "Set these preferences now?" ),
			       QMessageBox::Yes | QMessageBox::Default,
			       QMessageBox::No ) == QMessageBox::Yes )
      getPreferences();
  }
}

void Catkin::readSettings()
{
  //int i;
  QString crf;
  Application::recentFile currentRF;

  QSettings settings;
  applicationVersion = settings.value( "application/version", "" ).toString();
  settings.beginGroup( "geometry" );
  resize( settings.value( "size", QSize( 640, 450 )).toSize() );
  move( settings.value( "position", QPoint( 20, 20 )).toPoint() );
  settings.endGroup();
  settings.beginGroup( "account" );
  // blogType = settings.value( "blogType", 0 ).toInt();
  server = settings.value( "server", "" ).toString();
  location = settings.value( "location", "" ).toString();
  login = settings.value( "login", "" ).toString();
  password = settings.value( "password", "" ).toString();
  localStorageDirectory = settings.value( "localStorageDirectory", "" ).toString();
  if( localStorageDirectory.contains( "~/" ) ) {
    localStorageDirectory.replace( "~", QDir::homePath() );
    settings.setValue( "localStorageDirectory", localStorageDirectory );
  }
  localStorageFileExtn = settings.value( "localStorageFileExtn", "cqt" ).toString();
  categoriesEnabled = settings.value( "categoriesEnabled", true ).toBool();
  useNewWindows = settings.value( "useNewWindows", true ).toBool();
  savePassword = settings.value( "savePassword", false ).toBool();
  postAsSave = settings.value( "postAsSave", true ).toBool();
  allowComments = settings.value( "allowComments", true ).toBool();
  allowTB = settings.value( "allowTB", true ).toBool();
  //  postDateTime = settings.value( "postDateTime", true ).toBool();
#ifdef USE_SYSTRAYICON
  copyTitle = settings.value( "copyTitle", true ).toBool();
#endif
#if QT_VERSION >= 0x040200
  allowRegexSearch = settings.value( "allowRegexSearch", false ).toBool();
#endif
  //useTwoNewlines = settings.value( "useTwoNewlines", false ).toBool();
  settings.endGroup();
  settings.beginGroup( "fonts" );
  editorFontString = settings.value( "editorFontString", "" ).toString();
  previewFontString = settings.value( "previewFontString", "" ).toString();
  consoleFontString = settings.value( "consoleFontString", "" ).toString();
  settings.endGroup();

#ifdef USE_SYSTRAYICON
  settings.beginGroup( "sysTrayIcon" );
  STI2ClickFunction = settings.value( "doubleClickFunction", 0 ).toInt();
  settings.endGroup();
#endif

  /*  settings.beginGroup( "recentFiles" );
  for( i = 0; i < 20; ++i ) {
    crf = settings.value( QString( "recentFile%1" ).arg( i ), "" ).toString();
    currentRF.filename = crf.section( "filename:", 1, 1 ).section( " ##title:", 0, 0 );
    currentRF.title = crf.section( " ##title:", 1, 1 );
    recentFiles.append( currentRF );
    }*/
  recentFiles = qtm->recentFiles();
  updateRecentFileMenu();
  connect( qtm, SIGNAL( recentFilesUpdated( QList<Application::recentFile> ) ),
	   this, SLOT( setRecentFiles( QList<Application::recentFile> ) ) );

  if( server.isEmpty() || location.isEmpty() || login.isEmpty() ) {
    // Adequate network details absent, therefore disable all widgets and actions
    // leading to network use.
    setNetworkActionsEnabled( false );
  }

  cw.page_3->setEnabled( true );
  cw.chComments->setEnabled( true );
  cw.chTB->setEnabled( true );
#if QT_VERSION >= 0x040200
  searchWidget->setExpertEnabled( allowRegexSearch );
#endif
}

void Catkin::handleEnableCategories()
{
  cw.gbCategory->setEnabled( categoriesEnabled );
  ui.actionRefresh_categories->setEnabled( categoriesEnabled );
  ui.actionSend_categories->setEnabled( categoriesEnabled );
}

void Catkin::setRecentFiles( const QList<Application::recentFile> &rfs )
{
  recentFiles = rfs;
  updateRecentFileMenu();
}

void Catkin::updateRecentFileMenu()
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

void Catkin::openRecentFile()
{
  QAction *action = qobject_cast<QAction *>( sender() );
  if( action )
    choose( action->data().toString() );
}

void Catkin::writeSettings()
{
  // int i;
  QSettings settings;

  settings.beginGroup( "geometry" );
  settings.setValue( "size", size() );
  settings.setValue( "position", pos() );
  settings.endGroup();
  /*
  settings.beginGroup( "recentFiles" );
  for( i = 0; i < 20; ++i ) {
    settings.setValue( QString( "recentFile%1" ).arg( i ),
		       QString( "filename:%1 ##title:%2" )
		       .arg( recentFiles.value( i ).filename )
		       .arg( recentFiles.value( i ).title ) );
		       }*/
}

void Catkin::callRefreshCategories()
{
  if( !currentHttpBusiness ) {
    cw.cbMainCat->clear();
    cw.lwOtherCats->clear();
    refreshCategories();
    // currentBlog = b;
  }
  else {
#ifdef QTM_DEBUG
    if( currentHttpBusiness != 13 ) {
      statusBar()->showMessage( tr( "changeBlog: All HTTP requests are blocked" ),
                                2000 );
      addToConsole( QString( "changeBlog %1 failed - HTTP job of type %2 ongoing" )
                    .arg( b )
                    .arg( currentHttpBusiness ) );
    }

#else
    if( currentHttpBusiness != 13 )
      statusBar()->showMessage( tr( "All HTTP requests are blocked." ), 2000 );
#endif
  }
#ifndef NO_DEBUG_OUTPUT
  qDebug() << "Finishing changeblog";
#endif
}

void Catkin::refreshCategories()
{
  // const int indent = 2;
  QDomElement param, value, integer, string;

  disconnect( SIGNAL( httpBusinessFinished() ) );
  /*  qDebug( "currentBlog: %d, list size %d\n", currentBlog,
      usersBlogs.size() );*/
  if( !currentHttpBusiness ) {
    // http = new SafeHttp;

    QDomDocument doc;
    QDomElement methodCall = doc.createElement( "methodCall" );
    methodCall.appendChild( XmlMethodName( doc, "mt.getCategoryList" ) );

    QDomElement params = doc.createElement( "params" );
    /*params.appendChild( XmlValue( doc, "string",
				  usersBlogs[currentBlog].section( "blogid:", 1, 1 ).
				  section( ";", 0, 0 ) ) ); */
    params.appendChild( XmlValue( doc, "string", cw.cbBlogSelector->itemData( cw.cbBlogSelector->currentIndex() ).toString() ) );
    params.appendChild( XmlValue( doc, "string", login ) );
    params.appendChild( XmlValue( doc, "string", password ) );

    methodCall.appendChild( params );
    doc.appendChild( methodCall );
    doc.insertBefore( doc.createProcessingInstruction( "xml",
						       "version=\"1.0\" encoding=\"UTF-8\"" ),
		      doc.firstChild() );
    QByteArray requestArray( doc.toByteArray( 2 ) );
    responseData = "";
    QHttpRequestHeader header( "POST", location );
    header.setValue( "Host", server );
    header.setValue( "User-Agent", userAgentString );
    http->setHost( server );
    http->request( header, requestArray );

    addToConsole( header.toString() );
    addToConsole( doc.toString() );

    //qDebug() << "setting busy cursor";
    if( QApplication::overrideCursor() == 0 )
      QApplication::setOverrideCursor( QCursor( Qt::BusyCursor ) );

    currentHttpBusiness = 13; // Processing mt.getCategoryList
    connect( http, SIGNAL( done( bool ) ),
	     this, SLOT( handleDone( bool ) ) );
    connect( http, SIGNAL( readyRead( const QHttpResponseHeader & ) ),
       this, SLOT( handleResponseHeader( const QHttpResponseHeader & ) ) );
    connect( http, SIGNAL( hostLookupFailed() ),
	     this, SLOT( handleHostLookupFailed() ) );
  }
  else {
#ifdef QTM_DEBUG
    statusBar()->showMessage( tr( "refreshCategories: All HTTP requests are blocked" ),
			      2000 );
#else
    statusBar()->showMessage( tr( "All HTTP requests are blocked." ), 2000 );
#endif
  }
}

void Catkin::getAccounts()
{
  QList<AccountsDialog::Account> acctsList, returnedAccountsList;
  AccountsDialog::Account acct;
  QDomNodeList accountsList, thisAccountsAttribs;
  QDomDocument newAccountsDom;
  QDomElement newQTMAccounts, newAccount, detailElement, nameElement, serverElement, locationElement, 
    portElement, loginElement, pwdElement, blogsElement, boolElement, attribsElement;
  QString oldCurrentAccountId, currentTitle;
  QStringList thisAccountsAttribStrings;
  int c, i, j;

  // Extract accounts list from account tree
  accountsList = accountsDom.elementsByTagName( "account" );
  for( i = 0; i < accountsList.count(); i++ ) {
    acct = AccountsDialog::Account();
    acct.id = accountsList.at( i ).toElement().attribute( "id" );
    detailElement = accountsList.at( i ).firstChildElement( "details" );
    acct.name = detailElement.firstChildElement( "title" ).text();
    acct.server = detailElement.firstChildElement( "server" ).text();
    acct.location = detailElement.firstChildElement( "location" ).text();
    acct.port = detailElement.firstChildElement( "port" ).text();
    acct.login = detailElement.firstChildElement( "login" ).text();
    acct.password = detailElement.firstChildElement( "password" ).text();

    acct.categoriesEnabled = false;
    acct.postDateTime = false;
    acct.comments = false;
    acct.trackback = false;

    thisAccountsAttribs = accountsList.at( i ).toElement().firstChildElement( "details" )
      .firstChildElement( "attributes" ).elementsByTagName( "attribute" );
    thisAccountsAttribStrings = QStringList();
    for( j = 0; j < thisAccountsAttribs.count(); j++ )
      thisAccountsAttribStrings << thisAccountsAttribs.at( j ).toElement().attribute( "name" );

    if( thisAccountsAttribStrings.contains( "categoriesEnabled" ) ) {
	acct.categoriesEnabled = true;
    }
    if( thisAccountsAttribStrings.contains( "postDateTime" ) ) {
      acct.postDateTime = true;
    }
    if( thisAccountsAttribStrings.contains( "comments" ) ) {
      acct.comments = true;
    }
    if( thisAccountsAttribStrings.contains( "trackback" ) ) {
	acct.trackback = true;
    }

    acctsList.append( acct );
  }

  AccountsDialog acctsDialog( acctsList, this );

  if( acctsDialog.exec() ) {
    returnedAccountsList = acctsDialog.accounts();
    newQTMAccounts = newAccountsDom.createElement( "QTMAccounts" );

    for( i = 0; i < returnedAccountsList.count(); ++i ) {
      newAccount = newAccountsDom.createElement( "account" );
      newAccount.setAttribute( "id", returnedAccountsList.at( i ).id );
      detailElement = newAccountsDom.createElement( "details" );
      nameElement = newAccountsDom.createElement( "title" );
      nameElement.appendChild( newAccountsDom.createTextNode( returnedAccountsList.at( i ).name ) );
      serverElement = newAccountsDom.createElement( "server" );
      serverElement.appendChild( newAccountsDom.createTextNode( returnedAccountsList.at( i ).server ) );
      locationElement = newAccountsDom.createElement( "location" );
      locationElement.appendChild( newAccountsDom.createTextNode( returnedAccountsList.at( i ).location ) );
      portElement = newAccountsDom.createElement( "port" );
      portElement.appendChild( newAccountsDom.createTextNode( returnedAccountsList.at( i ).port ) );
      loginElement = newAccountsDom.createElement( "login" );
      loginElement.appendChild( newAccountsDom.createTextNode( returnedAccountsList.at( i ).login ) );
      pwdElement = newAccountsDom.createElement( "password" );
      pwdElement.appendChild( newAccountsDom.createTextNode( returnedAccountsList.at( i ).password ) );

      detailElement.appendChild( nameElement );
      detailElement.appendChild( serverElement );
      detailElement.appendChild( locationElement );
      detailElement.appendChild( portElement );
      detailElement.appendChild( loginElement );
      detailElement.appendChild( pwdElement );

      if( returnedAccountsList.at( i ).categoriesEnabled || returnedAccountsList.at( i ).postDateTime ||
	  returnedAccountsList.at( i ).comments || returnedAccountsList.at( i ).trackback )
	attribsElement = newAccountsDom.createElement( "attributes" );

      if( returnedAccountsList.at( i ).categoriesEnabled ) {
	boolElement = newAccountsDom.createElement( "attribute" );
	boolElement.setAttribute( "name", "categoriesEnabled" );
	attribsElement.appendChild( boolElement );
      }
      if( returnedAccountsList.at( i ).postDateTime ) {
	boolElement = newAccountsDom.createElement( "attribute" );
	boolElement.setAttribute( "name", "postDateTime" );
	attribsElement.appendChild( boolElement );
      }
      if( returnedAccountsList.at( i ).comments ) {
	qDebug() << "comments attribute set";
	boolElement = newAccountsDom.createElement( "attribute" );
	boolElement.setAttribute( "name", "comments" );
	attribsElement.appendChild( boolElement );
      }
      if( returnedAccountsList.at( i ).trackback ) {
	qDebug() << "TB attribute set";
	boolElement = newAccountsDom.createElement( "attribute" );
	boolElement.setAttribute( "name", "trackback" );
	attribsElement.appendChild( boolElement );
      }

      if( !attribsElement.isNull() )
	detailElement.appendChild( attribsElement );

      newAccount.appendChild( detailElement );
      
      // Check if each account is matched from the old list; if it is, copy the blogs list
      for( j = 0; j < accountsList.count(); ++j ) {
	if( accountsList.at( j ).toElement().hasAttribute( "id" ) ) {
	  if( accountsList.at( j ).toElement().attribute( "id " ) == returnedAccountsList.at( i ).id ) {
	    blogsElement = accountsList.at( j ).firstChildElement( "blogs" );
	    if( !blogsElement.isNull() )
	      newAccount.appendChild( newAccountsDom.importNode( blogsElement, true ) );
	    break;
	  }
	}
      }
      newQTMAccounts.appendChild( newAccount );
    }

    // Now add processing instruction and the whole QTMAccounts element, and replace the old main
    // accounts tree with this one
    newAccountsDom.appendChild( newAccountsDom.createProcessingInstruction( "xml",
									    "version='1.0'" ) );
    newAccountsDom.appendChild( newQTMAccounts );
    accountsDom = newAccountsDom.cloneNode( true ).toDocument();

    oldCurrentAccountId = cw.cbAccountSelector->itemData( cw.cbAccountSelector->currentIndex() )
      .toString();
    int oldCurrentBlog = cw.cbBlogSelector->currentIndex();
    cw.cbAccountSelector->clear();
    accountsList = accountsDom.elementsByTagName( "account" );
    for( i = 0; i < accountsList.count(); ++i ) {
      currentTitle = accountsList.at( i ).firstChildElement( "details" )
	.firstChildElement( "title" ).text();
      if( currentTitle.isEmpty() )
	currentTitle = tr( "(Unnamed account)" );
      cw.cbAccountSelector->addItem( currentTitle, accountsList.at( i ).toElement().
				     attribute( "id" ) );
    }
    // Check if the old current account is in this list; if so, make it current again
    for( i = 0; i < accountsList.count(); ++i ) {
      if( accountsList.at( i ).toElement().attribute( "id" ) == oldCurrentAccountId ) {
	cw.cbAccountSelector->setCurrentIndex( i );
	currentAccountElement = accountsList.at( i ).toElement();

	QStringList accountStringNames( accountStrings.keys() );
	Q_FOREACH( QString s, accountStringNames ) {
	  *(accountStrings[s]) = currentAccountElement.firstChildElement( "details" )
	    .firstChildElement( s ).text();
	}

	// Now check if the current account has any blogs
	if( !currentAccountElement.firstChildElement( "blogs" ).isNull() )
	  populateBlogList();
	else
	  refreshBlogList();
	break;
      }
      if( i == accountsList.count()-1 ) {
	cw.cbAccountSelector->setCurrentIndex( 0 );
	refreshBlogList();
      }
    }
    connect( cw.cbAccountSelector, SIGNAL( activated( int ) ),
	     this, SLOT( changeAccount( int ) ) );
    saveAccountsDom();
  }
}

void Catkin::getPreferences()
{
  bool blogUnchanged;
  QSettings settings;

  PrefsDialog prefsDialog( this );
  /*if( blogType )
    prefsDialog->cbBlogType->setCurrentIndex( blogType-1 );
  else
  prefsDialog->cbBlogType->setCurrentIndex( 4 );
  prefsDialog.lePort->setEnabled( false );   // This can come later.
  prefsDialog.lPort->setEnabled( false );  //
  prefsDialog.leServer->setText( server );
  prefsDialog.leLocation->setText( location );
  prefsDialog.leLogin->setText( login );
  prefsDialog.lePassword->setText( password ); */
  if( localStorageDirectory.isEmpty() ) {
#ifdef Q_WS_WIN
    QString lsd = QString( "%1/QTM blog" ).arg( QDir::homePath() )
      .replace( "/", "\\" );
#else
    QString lsd = QString( "%1/qtm-blog" ).arg( QDir::homePath() );
#endif
    prefsDialog.leLocalDir->setText( lsd );
  }
  else
    prefsDialog.leLocalDir->setText( localStorageDirectory );
  prefsDialog.leFileExtn->setText( localStorageFileExtn );
  /*prefsDialog->chCategories->setCheckState( categoriesEnabled ? Qt::Checked :
    Qt::Unchecked ); */
  prefsDialog.chUseNewWindows->setCheckState( useNewWindows ? Qt::Checked :
				       Qt::Unchecked );
  /*prefsDialog->chSavePassword->setCheckState( savePassword ? Qt::Checked :
    Qt::Unchecked ); */
  prefsDialog.cbPostAsSave->setCheckState( postAsSave ? Qt::Checked :
					    Qt::Unchecked );
  prefsDialog.cbAllowComments->setCheckState( allowComments ? Qt::Checked :
					       Qt::Unchecked );
  prefsDialog.cbAllowTB->setCheckState( allowTB ? Qt::Checked :
				       Qt::Unchecked );
  //prefsDialog->chPostDateTime->setCheckState( postDateTime ? Qt::Checked : Qt::Unchecked );
#ifdef USE_SYSTRAYICON
  prefsDialog.chCopyTitle->setCheckState( copyTitle ? Qt::Checked : Qt::Unchecked );
#else
  prefsDialog.chCopyTitle->setVisible( false );
#endif
#if QT_VERSION >= 0x040200
  prefsDialog.chAllowRegexSearch->setCheckState( allowRegexSearch ? Qt::Checked : Qt::Unchecked );
#endif
  //prefsDialog->cbUseTwoNewlines->setCheckState( useTwoNewlines ? Qt::Checked : Qt::Unchecked );
  //prefsDialog->chUseBloggerTitleFormatting->setCheckState( useBloggerTitleFormatting ? Qt::Checked :
  //				     Qt::Unchecked );
  //prefsDialog->cbBloggerTitleFormat->setCurrentIndex( bloggerTitleFormat );
#ifdef Q_WS_MAC
  prefsDialog.setWindowFlags( Qt::Sheet );
#endif

#if QT_VERSION >= 0x040200
  prefsDialog.tabWidget->setTabText( 0, tr( "General" ) );
  prefsDialog.tabWidget->setTabText( 1, tr( "Fonts" ) );
  prefsDialog.tabWidget->setCurrentIndex( 0 );

  QFont editorFont = EDITOR->font();
  QFont previewFont = previewWindow->font();
  QFont consoleFont = console->font();
  prefsDialog.fcbComposer->setCurrentFont( editorFont );
  prefsDialog.sbComposer->setValue( editorFont.pointSize() );
  prefsDialog.fcbPreview->setCurrentFont( previewFont );
  prefsDialog.sbPreview->setValue( previewFont.pointSize() );
  prefsDialog.fcbConsole->setCurrentFont( consoleFont );
  prefsDialog.sbConsole->setValue( consoleFont.pointSize() );
#ifdef USE_SYSTRAYICON
  prefsDialog.cbSTI2ClickFunction->setCurrentIndex( STI2ClickFunction );
#else
  prefsDialog.cbSTI2ClickFunction->hide();
  prefsDialog.label_10->hide();
#endif
#endif
  prefsDialog.resize( QSize( prefsDialog.width(),
			     prefsDialog.minimumHeight() ) );
  if( prefsDialog.exec() ) {
    /*if( (server == prefsDialog->leServer->text())
	&& (location == prefsDialog->leLocation->text() ) )
      blogUnchanged = true;
    else
    blogUnchanged = false; */
#ifndef NO_DEBUG_OUTPUT
    qDebug( "Setting account variables" );
#endif
    /*blogType = prefsDialog->cbBlogType->currentIndex()+1; // 0 means no type is set
    if( blogType == 5 )
    blogType = 0; */
    /*    server = prefsDialog.leServer->text();
    location = prefsDialog.leLocation->text();
    login = prefsDialog.leLogin->text();
    password = prefsDialog.lePassword->text(); */
    localStorageDirectory = prefsDialog.leLocalDir->text();
    localStorageFileExtn = prefsDialog.leFileExtn->text();
    //categoriesEnabled = (int)prefsDialog.chCategories->checkState();
    useNewWindows = prefsDialog.chUseNewWindows->isChecked();
    //savePassword = prefsDialog.chSavePassword->isChecked();
    postAsSave = prefsDialog.cbPostAsSave->isChecked();
    allowComments = prefsDialog.cbAllowComments->isChecked();
    allowTB = prefsDialog.cbAllowTB->isChecked();
    //    postDateTime = prefsDialog.chPostDateTime->isChecked();
#ifdef USE_SYSTRAYICON
    copyTitle = prefsDialog.chCopyTitle->isChecked();
#endif
#if QT_VERSION >= 0x040200
    allowRegexSearch = prefsDialog.chAllowRegexSearch->isChecked();
#endif
    //useTwoNewlines = prefsDialog.cbUseTwoNewlines->isChecked();
    //useBloggerTitleFormatting = prefsDialog.chUseBloggerTitleFormatting->isChecked();
    //bloggerTitleFormat = prefsDialog.cbBloggerTitleFormat->currentIndex();
#if QT_VERSION >= 0x040200
#ifndef NO_DEBUG_OUTPUT
    qDebug( "setting fonts" );
#endif
    QFont ef = prefsDialog.fcbComposer->currentFont();
    ef.setPointSize( prefsDialog.sbComposer->value() );
    editorFontString = ef.toString();
    EDITOR->setFont( ef );
    QFont pf = prefsDialog.fcbPreview->currentFont();
    pf.setPointSize( prefsDialog.sbPreview->value() );
    previewFontString = pf.toString();
    previewWindow->setFont( pf );
    QFont cf = prefsDialog.fcbConsole->currentFont();
    cf.setPointSize( prefsDialog.sbConsole->value() );
    consoleFontString = cf.toString();
    console->setFont( cf );
#endif
#if defined USE_SYSTRAYICON
    STI2ClickFunction = prefsDialog.cbSTI2ClickFunction->currentIndex();
    if( sti ) {
#ifndef NO_DEBUG_OUTPUT
      qDebug( "setting double click function" );
#endif
      sti->setDoubleClickFunction( STI2ClickFunction );
      sti->setCopyTitle( copyTitle );
    }
#endif

    // Handle local directory settings; a default is used if none is specified
    if( localStorageDirectory.isEmpty() ) {
#ifdef Q_WS_WIN
      localStorageDirectory = PROPERSEPS( QString( "%1/QTM blog" ).arg( QDir::homePath() ) );
#else
      localStorageDirectory = QString( "%1/qtm-blog" ).arg( QDir::homePath() );
#endif
    }
    QDir qd( localStorageDirectory );
    if( !qd.exists() ) {
      addToConsole( tr( "Making directory %1" ).arg( localStorageDirectory ) );
      if( !qd.mkpath( localStorageDirectory ) )
	statusBar()->showMessage( tr( "Could not create QTM directory." ), 2000 );
      if( !noAutoSave )
	saveAccountsDom();
    }


    // Now check to see if this is an old-style Blogger account, which does not
    // support categories.

    if( server.contains( "blogger.com" ) ) {
      // Blogger no longer supports this API, so all these features are redundant.
      statusBar()->showMessage( tr( "Blogger no longer supports QTM or similar clients." ), 2000 );
      server = "";
      location = "";
    } else
      cw.page_3->setEnabled( true );

    handleEnableCategories();

#if QT_VERSION >= 0x040200
    searchWidget->setExpertEnabled( allowRegexSearch );
#endif

    settings.setValue( "application/version", QTM_VERSION );
    settings.beginGroup( "account" );
    /*    settings.setValue( "server", server );
    settings.setValue( "location", location );
    settings.setValue( "login", login );
    settings.setValue( "password", password ); */
    settings.setValue( "localStorageDirectory",
		       localStorageDirectory.replace( "~", QDir::homePath() ) );
    settings.setValue( "localStorageFileExtn", localStorageFileExtn );
    //settings.setValue( "categoriesEnabled", categoriesEnabled );
    settings.setValue( "useNewWindows", useNewWindows );
    //settings.setValue( "savePassword", savePassword );
    settings.setValue( "postAsSave", postAsSave );
    settings.setValue( "allowComments", allowComments );
    settings.setValue( "allowTB", allowTB );
    //    settings.setValue( "postDateTime", postDateTime );
#ifdef USE_SYSTRAYICON
    settings.setValue( "copyTitle", copyTitle );
#endif
#if QT_VERSION >= 0x040200
    settings.setValue( "allowRegexSearch", allowRegexSearch );
#endif
    //settings.setValue( "useTwoNewlines", useTwoNewlines );
    //settings.setValue( "useBloggerTitleFormatting", useBloggerTitleFormatting );
    //settings.setValue( "bloggerTitleFormat", bloggerTitleFormat );
    settings.endGroup();
#if QT_VERSION >= 0x040200
    settings.beginGroup( "fonts" );
    settings.setValue( "editorFontString", editorFontString );
    settings.setValue( "previewFontString", previewFontString );
    settings.setValue( "consoleFontString", consoleFontString );
    settings.endGroup();
#ifdef USE_SYSTRAYICON
    settings.beginGroup( "sysTrayIcon" );
    settings.setValue( "doubleClickFunction", STI2ClickFunction );
    settings.endGroup();
#endif
#endif

    /*    if( server.isEmpty() || location.isEmpty() || login.isEmpty() ||
	prefsDialog.noValidHost() ) {
      // Adequate network details absent or host not valid, therefore disable
      // all widgets and actions leading to network use.
      setNetworkActionsEnabled( false );
    }
    else {
      setNetworkActionsEnabled( true );
    
      if( !blogUnchanged )
	refreshBlogList();
    }
    connect( this, SIGNAL( httpBusinessFinished() ),
    this, SLOT( doInitialChangeBlog() ) ); */
  }
}

void Catkin::saveAccountsDom()
{
  QFile domOut( PROPERSEPS( QString( "%1/qtmaccounts.xml" ).arg( localStorageDirectory ) ) );
  if( domOut.open( QIODevice::WriteOnly ) ) {
    QTextStream domFileStream( &domOut );
    accountsDom.save( domFileStream, 2 );
    domOut.close();
  }
  else {
    QDir dir( localStorageDirectory );
    if( !dir.exists() ) {
      if( QMessageBox::question( 0, tr( "Cannot find storage directory" ),
				 tr( "QTM cannot find the directory you specified to "
				     "store your account data and files.\n\n"
				     "Create it?" ),
				 QMessageBox::Yes | QMessageBox::Default,
				 QMessageBox::No ) == QMessageBox::Yes ) {
	if( dir.mkpath( localStorageDirectory ) ) {
	  domOut.setFileName( PROPERSEPS( QString( "%1/qtmaccounts.xml" ).arg( localStorageDirectory ) ) );
	  if( domOut.open( QIODevice::WriteOnly ) ) {
	    QTextStream dfs( &domOut );
	    accountsDom.save( dfs, 2 );
	    domOut.close();
	  }
	  else
	    statusBar()->showMessage( tr( "Could not write to accounts file." ), 2000 );
	}
	else
	  statusBar()->showMessage( tr( "Could not create the directory." ), 2000 );
	//statusBar()->showMessage( tr( "Could not write to accounts file (error %1)" ).arg( (int)domOut.error() ), 2000 );
	//	  statusBar()->showMessage( tr( "Could not write to accounts file." ), 2000 );
      }
    }
  }
}

void Catkin::populateAccountList() // slot
{
  int i;
  QDomElement ct, detail;
  QString cid, cname;

  QDomNodeList accountNodeList = accountsDom.documentElement().elementsByTagName( "account" );
  int a = accountNodeList.count();

  if( a ) {
    cw.cbAccountSelector->clear();

    for( i = 0; i < a; i++ ) {
      cid = accountNodeList.at( i ).toElement().attribute( "id" );
      if( cid.isEmpty() )
	cid = QString( "nameless_%1" ).arg( i + 1 );
      detail = accountNodeList.at( i ).firstChildElement( "details" );
      if( !detail.isNull() ) {
	cname = detail.firstChildElement( "title" ).text();
	if( cname.isEmpty() )
	  cname = tr( "Unnamed account %1" ).arg( i + 1 );
	cw.cbBlogSelector->addItem( cname, cid );
      }
    }
  
  }
}

void Catkin::populateBlogList() // slot
{
  addToConsole( accountsDom.toString( 2 ) );

  QDomNodeList blogNodeList = currentAccountElement.firstChildElement( "blogs" )
       .elementsByTagName( "blog" );
  QDomNodeList catNodeList;
  QDomElement catsElement;
  QDomElement ct;
  int a = blogNodeList.count();
#ifndef NO_DEBUG_OUTPUT
  qDebug() << "Blogs: " << a;
#endif
  int b, i, j;

  if( a ) {
    cw.cbBlogSelector->clear();
    for( i = 0; i < a; i++ ) {
      ct = blogNodeList.at( i ).firstChildElement( "blogName" );
      cw.cbBlogSelector->addItem( blogNodeList.at( i ).firstChildElement( "blogName" ).text(),
				  QVariant( blogNodeList.at( i ).firstChildElement( "blogid" ).text() ));
      currentBlog = i;
      currentBlogElement = currentAccountElement.firstChildElement( "blogs" )
          .elementsByTagName( "blog" ).at( currentBlog ).toElement();
      currentBlogid = currentBlogElement.firstChildElement( "blogid" ).text();
    }

   currentBlog = cw.cbBlogSelector->currentIndex();

   catsElement = blogNodeList.at( currentBlog ).firstChildElement( "categories" );
   if( !catsElement.isNull() ) {
     catNodeList = catsElement.elementsByTagName( "category" );
     b = catNodeList.count();
     if( b ) {
       for( j = 0; j < b; j++ ) {
	 cw.cbMainCat->addItem( catNodeList.at( j ).firstChildElement( "categoryName" ).text(),
				QVariant( catNodeList.at( j ).firstChildElement( "categoryId" ).text() ) );
	 cw.lwOtherCats->addItem( catNodeList.at( j ).firstChildElement( "categoryName" ).text() );
       }
     }
     else {
       cw.cbMainCat->setEnabled( false );
       cw.lwOtherCats->setEnabled( false );
     }

   }
   connect( cw.cbBlogSelector, SIGNAL( activated( int ) ),
	    this, SLOT( changeBlog( int ) ) );
  }
}

void Catkin::refreshBlogList() // slot
{
  // http = new SafeHttp;

  QDomDocument doc;
  QDomElement methodCall = doc.createElement( "methodCall" );
  methodCall.appendChild( XmlMethodName( doc, "blogger.getUsersBlogs" ) );

  QDomElement params = doc.createElement( "params" );
  params.appendChild( XmlValue( doc, "string", "xxx" ) );
  params.appendChild( XmlValue( doc, "string", login ) );
  params.appendChild( XmlValue( doc, "string", password ) );
  methodCall.appendChild( params );

  doc.appendChild( methodCall );
  doc.insertBefore( doc.createProcessingInstruction( "xml",
						     "version=\"1.0\" encoding=\"UTF-8\"" ),
		    doc.firstChild() );

  responseData = "";
  QByteArray requestArray( doc.toByteArray( 2 ) );
  QHttpRequestHeader header( "POST", location );
  header.setValue( "Host", server );
  header.setValue( "User-Agent", userAgentString );
  http->setHost( server );
  http->request( header, requestArray );

  addToConsole( header.toString() );
  addToConsole( doc.toString() );

  //  qDebug() << "setting busy cursor";
  if( QApplication::overrideCursor() == 0 )
    QApplication::setOverrideCursor( QCursor( Qt::BusyCursor ) );

  currentHttpBusiness = 5; // Processing blogger.getUsersPosts request
  connect( http, SIGNAL( done( bool ) ),
	   this, SLOT( handleDone( bool ) ) );
  connect( http, SIGNAL( readyRead( const QHttpResponseHeader & ) ),
	   this, SLOT( handleResponseHeader( const QHttpResponseHeader & ) ) );
  connect( http, SIGNAL( hostLookupFailed() ),
	   this, SLOT( handleHostLookupFailed() ) );
}

void Catkin::handleResponseHeader( const QHttpResponseHeader &header ) // slot
{
  if( header.statusCode() == 404 ) {
    QMessageBox::warning( this, tr( "Error 404" ),
			  tr( "There is no web service at the location you\n"
			      "specified.  Please change it in the Preferences\n"
			      "window." ) );
    http->disconnect();
    http->abort();
    // http->deleteLater();
    // http = 0;
    setNetworkActionsEnabled( false );
  }
  else
    responseData.append( http->readAll() );
}

void Catkin::stopThisJob()
{
#ifndef NO_DEBUG_OUTPUT
  qDebug() << "Aborting.";
#endif

  http->disconnect();
  http->abort();

  currentHttpBusiness = 0;
  disconnect( this, SIGNAL( httpBusinessFinished() ), 0, 0 );
  if( QApplication::overrideCursor() != 0 )
    QApplication::restoreOverrideCursor();
}

void Catkin::handleDone( bool error ) // slot
{
  if( error )
    statusBar()->showMessage( tr( "The request failed." ), 2000 );
  else
    statusBar()->showMessage( QString( tr( "The request succeeded; %1 bytes received" ) )
			      .arg( responseData.size() ), 2000 );

  switch( currentHttpBusiness ) {
  case 5: // if handling blogger.getusersBlogs
    blogger_getUsersBlogs( responseData ); break;
  case 7: // if handling metaWeblog.newPost
    metaWeblog_newPost( responseData ); break;
  case 8: // if handling metaWeblog.editPost
    metaWeblog_editPost( responseData ); break;
  case 11: // if handling metaWeblog.newMediaObject
    metaWeblog_newMediaObject( responseData ); break;
  case 12: // if handling mt.publishPost
    mt_publishPost( responseData ); break;
  case 13: // if handling mt.getCategoryList
    mt_getCategoryList( responseData ); break;
  case 15: // if handling mt.setPostCategories
    mt_setPostCategories( responseData ); break;
  }

  http->disconnect();
  // delete http; http = 0;
  currentHttpBusiness = 0;
  emit httpBusinessFinished();
}

/*void Catkin::changeCurrentAccount( int a ) // slot
{
  currentAccount = a;
  }*/

void Catkin::changeCurrentBlog( int b ) // slot
{
  currentBlog = b;
}

/* void Catkin::changeBlog( int b ) // slot
{
  currentBlog = b;
  if( !currentHttpBusiness ) {
    cw.cbMainCat->clear();
    cw.lwOtherCats->clear();
    if( categoriesEnabled )
      refreshCategories();
    // currentBlog = b;
  }
  else {
#ifdef QTM_DEBUG
    if( currentHttpBusiness != 13 ) {
      statusBar()->showMessage( tr( "changeBlog: All HTTP requests are blocked" ),
				2000 );
      addToConsole( QString( "changeBlog %1 failed - HTTP job of type %2 ongoing" )
		       .arg( b )
		       .arg( currentHttpBusiness ) );
    }

#else
    if( currentHttpBusiness != 13 )
      statusBar()->showMessage( tr( "All HTTP requests are blocked." ), 2000 );
#endif
  }
}*/

void Catkin::changeAccount( int a ) // slot
{
  QString currentBlogText;
  int i;

  currentAccount = a;

  currentAccountElement = accountsDom.documentElement()
    .elementsByTagName( "account" ).at( a ).toElement();
  currentAccountId = currentAccountElement.toElement().attribute( "id" );
  qDebug() << "Current account: " << currentAccountElement.firstChildElement( "details" )
    .firstChildElement( "title" ).text();

  QStringList accountStringNames( accountStrings.keys() );
  QStringList accountAttribNames( accountAttributes.keys() );
  QDomNodeList attribNodes = currentAccountElement.firstChildElement( "details" )
    .elementsByTagName( "attribute" );
  int c = attribNodes.count();

  Q_FOREACH( QString s, accountStringNames )
    *(accountStrings[s]) = currentAccountElement.firstChildElement( "details" )
      .firstChildElement( s ).text();

  Q_FOREACH( QString t, accountAttribNames ) {
    *(accountAttributes[t]) = false;
    for( i = 0; i < attribNodes.count(); i++ ) {
      if( attribNodes.at( i ).toElement().attribute( "name" ) == t )
	*(accountAttributes[t]) = true;
    }
  }
    
  QDomElement blogsElement = currentAccountElement.firstChildElement( "blogs" );
  if( !blogsElement.isNull() ) {
    QDomNodeList blogsList = blogsElement.elementsByTagName( "blog" );
    int b = blogsList.count();
    if( b ) {
      qDebug() << "Blogs: " << b;
      cw.cbBlogSelector->clear();
      for( int i = 0; i < b; i++ )
	cw.cbBlogSelector->addItem( blogsList.at( i ).firstChildElement( "blogName" ).text(),
				    blogsList.at( i ).firstChildElement( "blogid" ).text() );

      emit blogRefreshFinished();
      if( QApplication::overrideCursor() != 0 )
	QApplication::restoreOverrideCursor();
    }
    else
      refreshBlogList();
  }
  else
    refreshBlogList();

}

void Catkin::changeBlog( int b ) // slot
{
  QString currentCategoryText;
#ifndef NO_DEBUG_OUTPUT
  qDebug() << "Starting changeblog" << b;
#endif
  currentBlog = b;

  currentBlogElement = currentAccountElement.firstChildElement( "blogs" )
    .elementsByTagName( "blog" ).at( currentBlog ).toElement();
  currentBlogid = currentBlogElement.firstChildElement( "blogid" ).text();
#ifndef NO_DEBUG_OUTPUT
  qDebug() << currentBlogid;
#endif
  // statusBar()->showMessage( QString( "blogid: %1" ).arg( currentBlogid ), 2000 );
  QDomElement catsElement = currentBlogElement.firstChildElement( "categories" );
  if( !catsElement.isNull() ) {
    QDomNodeList catsList = catsElement.elementsByTagName( "category" );
    int c = catsList.count();
    if( c ) {
#ifndef NO_DEBUG_OUTPUT
      qDebug() << "Categories: " << c;
#endif
      cw.cbMainCat->clear();
      cw.lwOtherCats->clear();
      for( int i = 0; i < c; i++ ) {
        currentCategoryText = catsList.at( i ).firstChildElement( "categoryName" ).text();
        cw.cbMainCat->addItem( currentCategoryText,
			       QVariant( catsList.at( i ).firstChildElement( "categoryId" ).text() ) );
        cw.lwOtherCats->addItem( currentCategoryText );
      }
      //      qDebug() << "refresh finished";
      emit categoryRefreshFinished();
      //      qDebug() << "setting busy cursor";
      if( QApplication::overrideCursor() != 0 )
	QApplication::restoreOverrideCursor();
    }
    else
      callRefreshCategories();
  }
  else
    callRefreshCategories();
}

/*void Catkin::blogger_getUsersBlogs( QByteArray response )
{
  QXmlInputSource xis;
  QXmlSimpleReader reader;
  XmlRpcHandler handler( currentHttpBusiness );
  QDomDocument doc;
  QDomNodeList blogNodeList;
  QDomDocumentFragment importedBlogList;


  console->insertPlainText( QString( response ) );
  cw.cbBlogSelector->disconnect( this, SLOT( changeBlog( int ) ) );
  disconnect( this, SIGNAL( httpBusinessFinished() ), 0, 0 );
  handler.setProtocol( currentHttpBusiness );
  reader.setContentHandler( &handler );
  reader.setErrorHandler( &handler );
  xis.setData( response );
  reader.parse( &xis );
  usersBlogs = handler.returnList();
  cw.cbBlogSelector->clear();
  if( usersBlogs[0].contains( "faultString" ) ) {
    QMessageBox::critical( this, tr( "Could not connect" ),
			   tr( "Could not connect to the server.\nThis may be "
			       "because you supplied a wrong file name "
			       "or password." ) );
    addToConsole( QString( "%1\n" ).arg( usersBlogs[0] ) );
  }
  else {
    for( int a = 0; a < usersBlogs.size(); a++ ) {
      addToConsole( QString( "%1\n" ).arg( usersBlogs[a] ) );
      cw.cbBlogSelector->addItem( QString( usersBlogs[a].section( "blogName:", 1 ) ).
				    section( ";", 0, 0 ) );
      currentBlog = cw.cbBlogSelector->currentIndex();
    }

  /*changeBlog( currentBlog );*
    if( !initialChangeBlog )
      connect( cw.cbBlogSelector, SIGNAL( currentIndexChanged( int ) ),
	       this, SLOT( changeBlog( int ) ) );

    connect( this, SIGNAL( httpBusinessFinished() ),
	     this, SLOT( doInitialChangeBlog() ) );
  }
} */

void Catkin::blogger_getUsersBlogs( QByteArray response )
{
  QXmlInputSource xis;
  QXmlSimpleReader reader;
  XmlRpcHandler handler( currentHttpBusiness );
  QDomDocument doc;
  QDomNodeList blogNodeList;
  QDomDocumentFragment importedBlogList;

  console->insertPlainText( QString( response ) );
  cw.cbBlogSelector->disconnect( this, SLOT( changeBlog( int ) ) );
  disconnect( this, SIGNAL( httpBusinessFinished() ), 0, 0 );
  handler.setProtocol( currentHttpBusiness );
  reader.setContentHandler( &handler );
  reader.setErrorHandler( &handler );
  xis.setData( response );
  reader.parse( &xis );
#ifndef NO_DEBUG_OUTPUT
  qDebug() << "just parsed blog list";
#endif
  /*usersBlogURIs = handler.returnList( "uri" );
  usersBlogIDs = handler.returnList( "blogid" );
  usersBlogNames = handler.returnList( "blogName" ); */

  // qDebug() << "Appending returned XML";
  importedBlogList = handler.returnXml();
  // addToConsole( doc.toString() );
  blogNodeList = importedBlogList.firstChildElement( "blogs" )
    .elementsByTagName( "blog" );

  cw.cbBlogSelector->clear();

  int i = blogNodeList.count();

  if( !i ) {
#ifndef NO_DEBUG_OUTPUT
    qDebug() << "No blogs";
#endif
    QString fstring = handler.faultString();
    if( !fstring.isEmpty() ) {
      QMessageBox::critical( this, tr( "Could not connect" ),
			     tr( "Could not connect to the server.\nThis may be "
				 "because you supplied a wrong file name "
				 "or password." ) );
      addToConsole( QString( "%1\n" ).arg( fstring ) );
    }
  }
  else {
#ifndef NO_DEBUG_OUTPUT
    qDebug() << "Blogs found";
#endif

    // importedBlogList = accountDom.importNode( doc.firstChildElement( "blogs" ), true );
    currentAccountElement.removeChild( currentAccountElement.firstChildElement( "blogs" ) );
    currentAccountElement.appendChild( accountsDom.importNode( importedBlogList.firstChildElement( "blogs" ), true ) );

    for( int a = 0; a < i; a++ ) {
      cw.cbBlogSelector->addItem( blogNodeList.at( a ).firstChildElement( "blogName" ).text(),
				  QVariant( blogNodeList.at( a ).firstChildElement( "blogid" ).text() ) );
      currentBlog = cw.cbBlogSelector->currentIndex();
    }
    addToConsole( accountsDom.toString( 2 ) );

  /*changeBlog( currentBlog );*/
    if( !initialChangeBlog )
      connect( cw.cbBlogSelector, SIGNAL( activated( int ) ),
	       this, SLOT( changeBlog( int ) ) );

    connect( this, SIGNAL( httpBusinessFinished() ),
	     this, SLOT( doInitialChangeBlog() ) );
  }
#ifndef NO_DEBUG_OUTPUT
  qDebug() << "Finished handling the output";
#endif
}

void Catkin::metaWeblog_newPost( QByteArray response )
{
  // Returned data should only contain a single string, and no structs. Hence
  // the XmlRpcHandler is not suitable.
#ifndef NO_DEBUG_OUTPUT
  qDebug() << "posted the piece";
#endif

  addToConsole( QString( response ) );
  if( response.contains( "<fault>" ) ) {
    statusBar()->showMessage( tr( "The submission returned a fault - see console." ), 2000 );
    QApplication::restoreOverrideCursor();
  }
  else {
    QString parsedData( response );
    entryNumber = parsedData.section( "<string>", 1, 1 )
      .section( "</string>", -2, -2 );
    connect( this, SIGNAL( httpBusinessFinished() ),
	     this, SLOT( setPostCategories() ) );

    if( !entryEverSaved ) {
      if( postAsSave && cleanSave ) {
	setWindowModified( false );
	dirtyIndicator->hide();
	connect( EDITOR, SIGNAL( textChanged() ), this, SLOT( dirtify() ) );
      }
    }
  }

  addToConsole( response );
  entryBlogged = true;
}

void Catkin::metaWeblog_editPost( QByteArray response )
{
  addToConsole( QString( response ) );

  if( response.contains( "<fault>" ) ) {
    statusBar()->showMessage( tr( "The submission returned a fault - see console." ), 2000 );
  }
  else {
    connect( this, SIGNAL( httpBusinessFinished() ),
	     this, SLOT( setPostCategories() ) );
    if( !entryEverSaved ) {
      if( postAsSave && cleanSave ) {
	setWindowModified( false );
	dirtyIndicator->hide();
	connect( EDITOR, SIGNAL( textChanged() ), this, SLOT( dirtify() ) );
      }
    }
  }
  if( QApplication::overrideCursor() != 0 )
    QApplication::restoreOverrideCursor();
}

void Catkin::metaWeblog_newMediaObject( QByteArray response )
{
  QXmlInputSource xis;
  QXmlSimpleReader reader;
  XmlRpcHandler handler;

#ifndef NO_DEBUG_OUTPUT
  qDebug() << "Handling RPC response";
#endif
  disconnect( this, SIGNAL( httpBusinessFinished() ), 0, 0 );
  addToConsole( QString( response ) );

  if( !response.contains( "<fault>" ) ) {
    if( response.contains( "<name>url</name>" ) ) {
      remoteFileLocation = QString( response )
	.section( "<string>", 1, 1 )
	.section( "</string>", 0, 0 );
    /*    handler.setProtocol( currentHttpBusiness );
    reader.setContentHandler( &handler );
    reader.setErrorHandler( &handler );
    xis.setData( response );
    reader.parse( &xis );

    QString rfl( handler.returnFirstEntry() );
    addToConsole( rfl );
    remoteFileLocation = rfl.section( "url:", 1, 1 ).section( ";", 0, -2 ); */
      pbCopyURL->show();
      ui.actionCopy_upload_location->setVisible( true );
      statusBar()->showMessage( tr( "Your file is here: %1" ).arg( remoteFileLocation ), 2000 );
    }
    else
      statusBar()->showMessage( tr( "The upload returned a fault." ), 2000 );
  }
  else {
    statusBar()->showMessage( tr( "The upload returned a fault." ), 2000 );
  }
#ifndef NO_DEBUG_OUTPUT
  qDebug() << "Finished handling response";
#endif

  if( QApplication::overrideCursor() != 0 )
    QApplication::restoreOverrideCursor();
}

void Catkin::mt_publishPost( QByteArray response )
{
  disconnect( this, SIGNAL( httpBusinessFinished() ), 0, 0 );
  addToConsole( QString( response ) );

  if( response.contains( "<fault>" ) )
    statusBar()->showMessage( tr( "An error occurred during rebuilding." ), 2000 );
  else
    statusBar()->showMessage( tr( "The post was published successfully." ), 2000 );

  if( QApplication::overrideCursor() != 0 )
    QApplication::restoreOverrideCursor();
}

void Catkin::mt_getCategoryList( QByteArray response )
{
  QXmlInputSource xis;
  QXmlSimpleReader reader;
  XmlRpcHandler handler( currentHttpBusiness );
  QDomDocumentFragment importedCategoryList;
  QDomElement newCategoriesElement, currentCategory, currentID, currentName;
  QString xmlRpcFaultString;
  // int currentCatNo;
  QStringList catList;
  //QHash<QString, QString> catList;

  cw.lwOtherCats->reset();

  addToConsole( response );

  handler.setProtocol( currentHttpBusiness );
  reader.setContentHandler( &handler );
  reader.setErrorHandler( &handler );
  xis.setData( response );
  reader.parse( &xis );
  while( !handler.isMethodResponseFinished() )
    QCoreApplication::processEvents();

  bool xfault = handler.fault();

  importedCategoryList = handler.returnXml();
  xmlRpcFaultString = handler.faultString();
  // addToConsole( doc.toString() );

  categoryNames = handler.returnList( "categoryName" );
  categoryIDs = handler.returnList( "categoryId" );
  // categoryList = handler.returnList();

  QDomElement returnedCategoriesElement = importedCategoryList.firstChildElement( "categories" );
  QDomNodeList returnedCats = returnedCategoriesElement.elementsByTagName( "category" );
  int i = returnedCats.size();
  //catList = QHash();
  for( int j = 0; j < i; j++ )
    if( !returnedCats.at( j ).firstChildElement( "categoryId" ).isNull() &&
	!returnedCats.at( j ).firstChildElement( "categoryName" ).isNull() ) {
      catList.append( QString( "%1 ##ID:%2" )
		      .arg( returnedCats.at( j ).firstChildElement( "categoryName" ).text() )
		      .arg( returnedCats.at( j ).firstChildElement( "categoryId" ).text() ) );
      //      qDebug() << catList.last();
  }
  if( !noAlphaCats )
    qSort( catList.begin(), catList.end(), Catkin::caseInsensitiveLessThan );

  if( xfault ) {
      QMessageBox::critical( this, tr( "Could not connect" ),
			     tr( "Could not connect to the server.\n"
				 "This may be because you supplied\n"
				 "the wrong file name or password." ) );
  }
  else {
    if( !i ) {
      statusBar()->showMessage( tr( "There are no categories." ) );
      cw.cbMainCat->setEnabled( false );
      cw.lwOtherCats->setEnabled( false );
    }
    else {
      //currentBlogElement.removeChild( currentBlogElement.firstChildElement( "categories" ) );
      // currentBlogElement.appendChild( accountsDom.importNode( returnedCategoriesElement, true ) );
      newCategoriesElement = accountsDom.createElement( "categories" );
      QStringList::iterator it;
      for( it = catList.begin(); it != catList.end(); ++it ) {
	cw.cbMainCat->addItem( it->section( " ##ID:", 0, 0 ),
			       QVariant( it->section( " ##ID:", 1, 1 ) ) );
	//	qDebug() << it->section( "##ID:", 0, 0 ) << it->section( "##ID:", 1, 1 );
	cw.lwOtherCats->addItem( it->section( " ##ID:", 0, 0 ) );
	currentCategory = accountsDom.createElement( "category" );
	currentID = accountsDom.createElement( "categoryId" );
	currentID.appendChild( accountsDom.createTextNode( it->section( " ##ID:", 1 ) ) );
	currentName = accountsDom.createElement( "categoryName" );
	currentName.appendChild( accountsDom.createTextNode( it->section( " ##ID:", 0, 0 ) ) );
	currentCategory.appendChild( currentID );
	currentCategory.appendChild( currentName );
	newCategoriesElement.appendChild( currentCategory );
      }
      if( currentBlogElement.firstChildElement( "categories" ).isNull() )
	currentBlogElement.appendChild( newCategoriesElement );
      else
	currentBlogElement.replaceChild( newCategoriesElement, currentBlogElement.firstChildElement( "categories" ) );

      cw.cbMainCat->setEnabled( true );
      cw.lwOtherCats->setEnabled( true );
      if( !noAutoSave ) {
	QFile domOut( PROPERSEPS( QString( "%1/qtmaccounts.xml" ).arg( localStorageDirectory ) ) );
	if( domOut.open( QIODevice::WriteOnly ) ) {
	  QTextStream domFileStream( &domOut );
	  accountsDom.save( domFileStream, 2 );
	  domOut.close();
	}
	else
	  statusBar()->showMessage( tr( "Could not write to accounts file (error %1)" ).arg( (int)domOut.error() ), 2000 );
	//	  statusBar()->showMessage( tr( "Could not write to accounts file." ), 2000 );
      }
    }
  }

  addToConsole( accountsDom.toString( 2 ) );

  connect( this, SIGNAL( httpBusinessFinished() ),
	   this, SIGNAL( categoryRefreshFinished() ) );

  //qDebug() << "setting busy cursor";
  if( QApplication::overrideCursor() != 0 )
    QApplication::restoreOverrideCursor();
}
/*
  // while( !handler.isMethodResponseFinished() ) { }
  categoryList = handler.returnList();

  if( categoryList.size() == 0 ) {
    statusBar()->showMessage( tr( "There are no categories." ) );
    cw.cbMainCat->setEnabled( false );
    cw.lwOtherCats->setEnabled( false );
  }
  else {
    if( categoryList[0].contains( "faultString" ) ) {
      QMessageBox::critical( this, tr( "Could not connect" ),
			     tr( "Could not connect to the server.\n"
				 "This may be because you supplied\n"
				 "the wrong file name or password." ) );
      addToConsole( QString( "%1\n" ).arg( categoryList[0] ) );
    }
    else {
      for( int a = 0; a < categoryList.size(); a++ ) {
	buffer = QString( "%1 ##ID:%2" )
	  .arg( categoryList[a].section( "categoryName:", 1, 1 )
		.section( ";", 0, 0 ) )
	  .arg( categoryList[a].section( "categoryId:", 1, 1 )
		.section( ";", 0, 0 ) );
	categoryList[a] = buffer;
      }
      qSort( categoryList.begin(), categoryList.end(), Catkin::caseInsensitiveLessThan );

      for( int b = 0; b < categoryList.size(); b++ ) {
	addToConsole( categoryList[b] );
	cw.cbMainCat->addItem( QString( categoryList[b].section( " ##ID:", 0, 0 ) ) );
	//					.section( ";", 0, 0 ) ) );
	cw.lwOtherCats->addItem( QString( categoryList[b].section( " ##ID:", 0, 0 ) ) );
	//					  .section( ";", 0, 0 ) ) );
      }
      cw.cbMainCat->setEnabled( true );
      cw.lwOtherCats->setEnabled( true );
    }
    connect( this, SIGNAL( httpBusinessFinished() ),
	     this, SIGNAL( categoryRefreshFinished() ) );
  }
  }*/

bool Catkin::caseInsensitiveLessThan( const QString &s1, const QString &s2 )
{
  return s1.toLower() < s2.toLower();
}

void Catkin::mt_setPostCategories( QByteArray response )
{
  disconnect( this, SIGNAL( httpBusinessFinished() ), 0, 0 );

  QXmlInputSource xis;
  QXmlSimpleReader reader;
  XmlRpcHandler handler( currentHttpBusiness );
  QList<QString> parsedData;
  QString rdata( response );

  if( rdata.contains( "<fault>" ) )
    statusBar()->showMessage( tr( "Categories not set successfully; see console." ), 2000 );
  else {
    statusBar()->showMessage( tr( "Categories set successfully." ), 2000 );
    // EDITOR->setReadOnly( true );

    if( location.contains( "mt-xmlrpc.cgi" ) && cw.cbStatus->currentIndex() == 1 )
      connect( this, SIGNAL( httpBusinessFinished() ),
	       this, SLOT( publishPost() ) );
  }
  addToConsole( rdata );
  QApplication::restoreOverrideCursor();
}

void Catkin::handleConsole( bool isChecked )
{
  switch( isChecked ) {
    case false:
      ui.actionP_review->setEnabled( true );
      mainStack->setCurrentIndex( previousRaisedLSWidget );
      // searchWidget->clearSearchText();
      searchWidget->setTextEdit( EDITOR );
      break;
    case true:
      ui.actionP_review->setEnabled( false );
      previousRaisedLSWidget = mainStack->currentIndex();
      mainStack->setCurrentIndex( consoleID );
      searchWidget->setTextEdit( console );
  }
}

void Catkin::makeBold()
{
  EDITOR->insertPlainText( QString( "<strong>%1</strong>" ).arg( EDITOR->textCursor().selectedText() ) );
}

void Catkin::makeItalic()
{
  EDITOR->insertPlainText( QString( "<em>%1</em>" ).arg( EDITOR->textCursor().selectedText() ) );
}

void Catkin::makeUnderline()
{
  EDITOR->insertPlainText( QString( "<u>%1</u>" ).arg( EDITOR->textCursor().selectedText() ) );
}

void Catkin::insertMore()
{
  if( !EDITOR->toPlainText().contains( "<!--more-->" ) )
    EDITOR->insertPlainText( "<!--more-->" );
  else
    statusBar()->showMessage( tr( "A 'more' tag already exists." ), 2000 );
}

void Catkin::makeBlockquote()
{
  EDITOR->insertPlainText( QString( "<blockquote>%1</blockquote>" )
			   .arg( EDITOR->textCursor().selectedText() ) );
}

void Catkin::makePara()
{
  EDITOR->insertPlainText( QString( "<p>%1</p>" ).arg( EDITOR->textCursor().selectedText() ) );
}

void Catkin::insertLink( bool isAutoLink )
{
  QString linkString, titleString;
  QString insertionString = "";
  QString selectedString = EDITOR->textCursor().selectedText();
  QString selectedStringLC;
  QDialog linkEntry( this );
  Ui::LinkEntry leui;
#ifdef Q_WS_MAC
  linkEntry.setWindowFlags( Qt::Sheet );
#endif
  leui.setupUi( &linkEntry );
  if( isAutoLink )
    leui.cbMakeAutoLink->setChecked( Qt::Checked );
  if( !selectedString.isEmpty() )
    leui.leLinkText->setText( selectedString );

  if( linkEntry.exec() ) {
    linkString = leui.leLinkURL->text();
    // linkString.replace( QChar( '&' ), "&amp;" );
    insertionString += QString( "<a href=\"%1\"" ).arg( linkString );
    if( leui.leLinkTitle->text().size() ) {
      titleString = leui.leLinkTitle->text();
      insertionString += QString( " title=\"%1\"" ).arg( titleString );
    }
    if( leui.chLinkTarget->isChecked() ) {
      switch( leui.cbLinkTarget->currentIndex() ) {
      case 0: insertionString += QString( " target=\"_top\"" );   break;
      case 1: insertionString += QString( " target=\"_blank\"" ); break;
      case 2: insertionString += QString( " target=\"view_window\"" ); break;
      }
    }
    insertionString += ">";
    EDITOR->insertPlainText( QString( "%1%2</a>" )
			     .arg( insertionString )
			     .arg( leui.leLinkText->text() ) );
  }
  if( leui.cbMakeAutoLink->isChecked() ) {
    selectedStringLC = selectedString.toLower().trimmed();
    autoLinkDictionary.insert( selectedStringLC, linkString );
    autoLinkTitleDictionary.insert( selectedStringLC, titleString );
    if( leui.chLinkTarget->isChecked() )
      autoLinkTargetDictionary.insert( selectedStringLC, leui.cbLinkTarget->currentIndex() );
    else
      autoLinkTargetDictionary.remove( selectedStringLC );
    saveAutoLinkDictionary();
  }
}

void Catkin::insertLinkFromClipboard()
{
  QString linkString( QApplication::clipboard()->text() );
  // linkString.replace( QChar( '&' ), "&amp;" );
  EDITOR->insertPlainText( QString( "<a href=\"%1\">%2</a>" )
                           .arg( linkString )
                           .arg( EDITOR->textCursor().selectedText() ) );
}

void Catkin::insertSelfLink()
{
  QString linkString( EDITOR->textCursor().selectedText() );

  if( QUrl( linkString, QUrl::StrictMode ).isValid() )
    EDITOR->insertPlainText( QString( "<a href=\"%1\">%1</a>" ).arg( linkString ) );
  else
    statusBar()->showMessage( tr( "The selection is not a valid URL." ), 2000 );
}

void Catkin::insertAutoLink()
{
  QString selectedText = EDITOR->textCursor().selectedText();
  QString selectedTextLC = selectedText.toLower().trimmed();
  QString titleString, targetString;
  QList<QString> targets;
  targets << "_top" << "_blank" << "view_window";

  if( autoLinkDictionary.contains( selectedTextLC ) ) {
    titleString = QString( " title=\"%1\"" )
      .arg( autoLinkTitleDictionary.value( selectedTextLC ) );
    if( autoLinkTargetDictionary.contains( selectedTextLC ) )
      targetString = QString( " target=\"%1\"" )
	.arg( targets.value( autoLinkTargetDictionary.value( selectedTextLC ) ) );

    EDITOR->insertPlainText( QString( "<a href=\"%1\"%2%3>%4</a>" )
			     .arg( autoLinkDictionary.value( selectedTextLC ) )
			     .arg( titleString )
			     .arg( targetString )
			     .arg( selectedText ) );
  }
  else
    insertLink( true );
}

void Catkin::insertImage()
{
  QString insertionString;
  QDialog image_entry( this );
  Ui::ImageEntry ieui;
#ifdef Q_WS_MAC
  image_entry.setWindowFlags( Qt::Sheet );
#endif
  ieui.setupUi( &image_entry );
  if( image_entry.exec() ) {
    insertionString = QString( "<img src=\"%1\"" )
      .arg( ieui.leImageURL->text() );
    if( ieui.leAltText->text().size() )
      insertionString += QString( " alt=\"%1\"" )
	.arg( ieui.leAltText->text() );
    if( ieui.chAlign->isChecked() ) {
      if( ieui.cbAlign->currentIndex() )
	insertionString += " align=\"right\"";
      else
	insertionString += " align=\"left\"";
    }
    insertionString += ">";
    EDITOR->insertPlainText( insertionString );
  }
}

void Catkin::insertImageFromClipboard()
{
  QString linkString( QApplication::clipboard()->text() );
  EDITOR->insertPlainText( QString( "<img src=\"%1\">%2</img>" )
                           .arg( linkString )
                           .arg( EDITOR->textCursor().selectedText() ) );
}

void Catkin::cut()
{
  EDITOR->cut();
}

void Catkin::copy()
{
  EDITOR->copy();
}

void Catkin::paste()
{
  EDITOR->paste();
}

void Catkin::undo()
{
  EDITOR->document()->undo();
}

void Catkin::redo()
{
  EDITOR->document()->redo();
}

void Catkin::makeUnorderedList()
{
  QString listString = EDITOR->textCursor().selection().toPlainText();

  if( !listString.isEmpty() )
    EDITOR->insertPlainText( getHTMLList( QString( "ul" ), listString ) );
}

void Catkin::makeOrderedList()
{
  QString listString = EDITOR->textCursor().selection().toPlainText();

  if( !listString.isEmpty() )
    EDITOR->insertPlainText( getHTMLList( QString( "ol" ), listString ) );
}

QString & Catkin::getHTMLList( QString tag, QString & text )
{
  QString return_value, workstring;
  QStringList worklist;

  worklist = text.split( "\n" );
  for( int a = 0; a < worklist.count(); a++ ) {
    worklist[a].prepend( "<li>" );
    worklist[a].append( "</li>" );
  }
  text = worklist.join( "\n" );
  text.prepend( QString( "<%1>" ).arg( tag ) );
  text.append( QString( "</%1>" ).arg( tag ) );

  return text;
}

void Catkin::pasteAsMarkedParagraphs()
{
  QString insertion = QApplication::clipboard()->text().trimmed();

  if( !insertion.isEmpty() ) {
    insertion.replace( QRegExp( "\n+" ), "</p>\n<p>" );
    insertion.prepend( "<p>" );
    insertion.append( "</p>" );
    EDITOR->insertPlainText( insertion );
  }
}

void Catkin::pasteAsBlockquote()
{
  QString insertion = QApplication::clipboard()->text().trimmed();

  if( !insertion.isEmpty() ) {
    insertion.replace( QRegExp( "\n{1,2}" ), "\n\n" );
    EDITOR->insertPlainText( QString( "<blockquote>%1</blockquote>" )
			     .arg( insertion ) );
  }
}

void Catkin::pasteAsMarkdownBlockquote()
{
  QString insertion = QApplication::clipboard()->text().trimmed();
  QString separator = "\n>\n";

  if( !insertion.isEmpty() ) {
    insertion.prepend( ">" );
    insertion.replace( "\n", "\n>" );
    EDITOR->insertPlainText( insertion );
  }
}

void Catkin::pasteAsUnorderedList()
{
  QString insertion = QApplication::clipboard()->text().trimmed()
    .replace( QRegExp( "\n{2,}" ), "\n" );

  if( !insertion.isEmpty() )
    EDITOR->insertPlainText( getHTMLList( QString( "ul" ), insertion ) );
}

void Catkin::pasteAsOrderedList()
{
  QString insertion = QApplication::clipboard()->text().trimmed()
    .replace( QRegExp( "\n{2,}" ), "\n" );

  if( !insertion.isEmpty() )
    EDITOR->insertPlainText( getHTMLList( QString( "ol" ), insertion ) );
}

void Catkin::doPreview( bool isChecked )
{
  QString line, techTagString;
  QString conversionString = "", conversionStringB = "";
  QTextDocument cDoc;

  if( isChecked ) {
    ui.action_View_Console->setEnabled( false );
    conversionString += QString( "<b>%1</b>\n\n" )
      .arg( cw.leTitle->text().size() ?
	    cw.leTitle->text() : "<i>Untitled</i>" );
    conversionString += EDITOR->toPlainText();
    QTextStream a( &conversionString );
    QRegExp re( "^(<table|thead|tfoot|caption|tbody|tr|td|th|div|dl|dd|dt|ul|ol|li|pre|select|form|blockquote|address|math|p|h[1-6])>" );
    do {
      line = a.readLine();
      if( !line.isNull() ) {
	if( re.exactMatch( line ) )
	  conversionStringB += line;
	else
	  conversionStringB += QString( "<p>%1</p>" ).arg( line );
      }
    } while( !a.atEnd() );

    previewWindow->setHtml( conversionStringB );
    previousRaisedLSWidget = mainStack->currentIndex();
    connect( previewWindow, SIGNAL( highlighted( const QString & ) ),
	     this, SLOT( showHighlightedURL( const QString & ) ) );
    mainStack->setCurrentIndex( previewWindowID );
    searchWidget->setTextEdit( previewWindow );
  } else {
    ui.action_View_Console->setEnabled( true );
    mainStack->setCurrentIndex( previousRaisedLSWidget );
    previewWindow->disconnect( SIGNAL( highlighted( const QString & ) ) );
    searchWidget->setTextEdit( EDITOR );
  }
}

void Catkin::showHighlightedURL( const QString &highlightedURL )
{
  statusBar()->showMessage( highlightedURL, 2000 );
}

void Catkin::blogThis()
{
  newMTPost();
}

void Catkin::newMTPost()
{
  // For the time being we are presuming that this is a MT or Wordpress blog.

  QDomDocument doc;
  QDomElement methodCall, params, param, member, value, integer,
    string, rpcstruct, rpcarray, actualValue;
  QString description, extEntry, techTagString;
  bool takeComms = cw.chComments->isChecked();
  bool takeTB = cw.chTB->isChecked();
  bool blogidIsInt;
  int count, tags;
  QList<QString> tblist;
  // QLineEdit *lePassword;

  if( !currentHttpBusiness && !entryBlogged ) {

    if( EDITOR->toPlainText().contains( "<!--more-->" ) ) {
      description = QString( EDITOR->toPlainText() )
	.section( "<!--more-->", 0, 0 );
      extEntry = QString( EDITOR->toPlainText() )
	.section( "<!--more-->", -1, -1 );
    } else {
      description = QString( EDITOR->toPlainText() );
      extEntry = "";
    }

    dateTime = QDateTime::currentDateTime().toString( Qt::ISODate );
    dateTime.remove( QChar( '-' ) );

    if( cw.lwTags->count() ) {
      tags = cw.lwTags->count();
      techTagString = "<p style=\"text-align:right;font-size:10px;\">Technorati Tags: ";
      for( count = 0; count < tags; count++ ) {
	techTagString.append( QString( "<a href=\"http://technorati.com/tag/%1\""
				       " rel=\"tag\">%2</a>%3" )
			      .arg( cw.lwTags->item( count )->text().replace( ' ', '+' ) )
			      .arg( cw.lwTags->item( count )->text()
				    .replace( "+", " " ) )
			      .arg( (count == tags-1) ? "</p>\n\n" : ", " ) );
      }

      if( cw.rbStartOfMainEntry->isChecked() )
	description.insert( 0, techTagString );
      else
	description.append( techTagString );
    }

    QString blogid = cw.cbBlogSelector->itemData( cw.cbBlogSelector->currentIndex() ).toString();
    /*    QString blogid = usersBlogs[currentBlog].section( "blogid:", 1, 1 )
	  .section( ";", 0, 0 );*/
    QRegExp blogidRegExp( "^[0-9]+$" );
    blogidIsInt = blogidRegExp.exactMatch( blogid );

    methodCall = doc.createElement( "methodCall" );
    methodCall.appendChild( XmlMethodName( doc, "metaWeblog.newPost" ) );

    params = doc.createElement( "params" );
    params.appendChild( XmlValue( doc, blogidIsInt ? "int" : "string", blogid ) );
    params.appendChild( XmlValue( doc, "string", login ) );
    params.appendChild( XmlValue( doc, "string", password ) );

    param = doc.createElement( "param" );
    value = doc.createElement( "value" );
    rpcstruct = doc.createElement( "struct" );
    rpcstruct.appendChild( XmlMember( doc, "title", "string",
				      cw.leTitle->text() ) );
    rpcstruct.appendChild( XmlMember( doc, "description", "string", description ) );
    if( postDateTime )
	rpcstruct.appendChild( XmlMember( doc, "dateCreated", "dateTime.iso8601", dateTime ) );
    rpcstruct.appendChild( XmlMember( doc, "mt_allow_comments", "boolean",
				      takeComms ? "1" : "0" ) );
    rpcstruct.appendChild( XmlMember( doc, "mt_allow_pings", "boolean",
				      takeTB ? "1" : "0" ) );
    rpcstruct.appendChild( XmlMember( doc, "mt_text_more", "string", extEntry ) );
    if( cw.teExcerpt->toPlainText().length() )
      rpcstruct.appendChild( XmlMember( doc, "mt_excerpt", "string",
					cw.teExcerpt->toPlainText().replace( QChar( '&' ), "&amp;" ) ) );
    else
      rpcstruct.appendChild( XmlMember( doc, "mt_excerpt", "string", "" ) );
    rpcstruct.appendChild( XmlMember( doc, "mt_keywords", "string", "" ) );

    if( cw.lwTBPings->count() ) {
      for( count = 0; count < cw.lwTBPings->count(); count++ )
	tblist.append( cw.lwTBPings->item( count )->text() );
      rpcstruct.appendChild( XmlRpcArray( doc, "mt_tb_ping_urls", tblist ) );
    }

    value.appendChild( rpcstruct );
    param.appendChild( value );
    params.appendChild( param );
    params.appendChild( XmlValue( doc, "boolean",
				  cw.cbStatus->currentIndex() ? "1" : "0" ) );
    methodCall.appendChild( params );
    doc.appendChild( methodCall );
    doc.insertBefore( doc.createProcessingInstruction( "xml",
						       "version=\"1.0\" encoding=\"UTF-8\"" ),
		      doc.firstChild() );

    QByteArray requestArray( doc.toByteArray() );
    responseData = "";
    // http = new SafeHttp;
    QHttpRequestHeader header( "POST", location );
    header.setValue( "Host", server );
    header.setValue( "User-Agent", userAgentString );
    http->setHost( server );
    http->request( header, requestArray );

    addToConsole( header.toString() );
    addToConsole( doc.toString() );

    if( QApplication::overrideCursor() == 0 )
      QApplication::setOverrideCursor( QCursor( Qt::BusyCursor ) );
    if( postAsSave && !entryEverSaved ) {
      cleanSave = true;
      connect( EDITOR, SIGNAL( textChanged() ), this, SLOT( dirtify() ) );
    }
    currentHttpBusiness = 7; // Processing metaWeblog.newPost request
    disconnect( this, SIGNAL( httpBusinessFinished() ) );
    connect( http, SIGNAL( done( bool ) ),
	     this, SLOT( handleDone( bool ) ) );
    connect( http, SIGNAL( readyRead( const QHttpResponseHeader & ) ),
	     this, SLOT( handleResponseHeader( const QHttpResponseHeader & ) ) );
    connect( http, SIGNAL( hostLookupFailed() ),
	     this, SLOT( handleHostLookupFailed() ) );
  }
  else {
    if( currentHttpBusiness ) {
#ifdef QTM_DEBUG
    statusBar()->showMessage( tr( "refreshCategories: All HTTP requests are blocked" ),
			      2000 );
#else
    statusBar()->showMessage( tr( "All HTTP requests are blocked." ), 2000 );
#endif
  }
    else
      submitMTEdit();
  }
}

void Catkin::submitMTEdit()
{
  QDomDocument doc;
  QDomElement methodCall, params, param, value, rpcstruct, rpcarray;
  QString description, extEntry, techTagString;
  int count, tags;
  bool takeComms = cw.chComments->isChecked();
  bool takeTB = cw.chTB->isChecked();
  // bool blogidIsInt;
  QList<QString> tblist;

  if( EDITOR->toPlainText().contains( "<!--more-->" ) ) {
    description = QString( EDITOR->toPlainText() )
	.section( "<!--more-->", 0, 0 );
    extEntry = QString( EDITOR->toPlainText() )
	.section( "<!--more-->", -1, -1 );
  } else {
    description = QString( EDITOR->toPlainText() );
    extEntry = "";
  }

  if( cw.lwTags->count() ) {
    tags = cw.lwTags->count();
    techTagString = "<p style=\"text-align:right;font-size:10px;\">Technorati Tags: ";
    for( count = 0; count < tags; count++ ) {
      techTagString.append( QString( "<a href=\"http://technorati.com/tag/%1\""
				     " rel=\"tag\">%2</a>%3" )
			    .arg( cw.lwTags->item( count )->text() )
			    .arg( cw.lwTags->item( count )->text()
				  .replace( "+", " " ) )
			    .arg( (count == tags-1) ? "</p>" : ", " ) );
    }

    if( cw.rbStartOfMainEntry->isChecked() )
      description.insert( 0, techTagString );
    else
      description.append( techTagString );
  }

  // blogid.toInt( &blogidIsInt );

  methodCall = doc.createElement( "methodCall" );
  methodCall.appendChild( XmlMethodName( doc, "metaWeblog.editPost" ) );

  params = doc.createElement( "params" );
  params.appendChild( XmlValue( doc, "string", entryNumber ) );
  params.appendChild( XmlValue( doc, "string", login ) );
  params.appendChild( XmlValue( doc, "string", password ) );

  param = doc.createElement( "param" );
  value = doc.createElement( "value" );
  rpcstruct = doc.createElement( "struct" );
  rpcstruct.appendChild( XmlMember( doc, "title", "string",
				    cw.leTitle->text() ) );
  rpcstruct.appendChild( XmlMember( doc, "description", "string", description ) );
  rpcstruct.appendChild( XmlMember( doc, "dateCreated", "dateTime.iso8601", dateTime ) );
  rpcstruct.appendChild( XmlMember( doc, "mt_allow_comments", "boolean",
				    takeComms ? "1" : "0" ) );
  rpcstruct.appendChild( XmlMember( doc, "mt_allow_pings", "boolean",
				    takeTB ? "1" : "0" ) );
  rpcstruct.appendChild( XmlMember( doc, "mt_text_more", "string", extEntry ) );
  if( cw.teExcerpt->toPlainText().length() )
    rpcstruct.appendChild( XmlMember( doc, "mt_excerpt", "string",
				      cw.teExcerpt->toPlainText().replace( QChar( '&' ), "&amp;" ) ) );
  else
    rpcstruct.appendChild( XmlMember( doc, "mt_excerpt", "string", "" ) );
  rpcstruct.appendChild( XmlMember( doc, "mt_keywords", "string", "" ) );

  if( cw.lwTBPings->count() ) {
    for( count = 0; count < cw.lwTBPings->count(); count++ )
      tblist.append( cw.lwTBPings->item( count )->text() );
    rpcstruct.appendChild( XmlRpcArray( doc, "mt_tb_ping_urls", tblist ) );
  }

  value.appendChild( rpcstruct );
  param.appendChild( value );
  params.appendChild( param );
  params.appendChild( XmlValue( doc, "boolean",
				cw.cbStatus->currentIndex() ? "1" : "0" ) );
  methodCall.appendChild( params );
  doc.appendChild( methodCall );
  doc.insertBefore( doc.createProcessingInstruction( "xml",
						     "version=\"1.0\" encoding=\"UTF-8\"" ),
		    doc.firstChild() );

  QByteArray requestArray( doc.toByteArray() );
  responseData = "";
  // requestArray.append( XmlData );
  // http = new SafeHttp;
  QHttpRequestHeader header( "POST", location );
  header.setValue( "Host", server );
  header.setValue( "User-Agent", userAgentString );
  http->setHost( server );
  http->request( header, requestArray );

  addToConsole( header.toString() );
  addToConsole( doc.toString() );

  if( QApplication::overrideCursor() == 0 )
    QApplication::setOverrideCursor( QCursor( Qt::BusyCursor ) );
  if( postAsSave && !entryEverSaved ) {
    cleanSave = true;
    connect( EDITOR, SIGNAL( textChanged() ), this, SLOT( dirtify() ) );
  }

  currentHttpBusiness = 8; // Processing metaWeblog.editPost request
  connect( http, SIGNAL( done( bool ) ),
	   this, SLOT( handleDone( bool ) ) );
  connect( http, SIGNAL( readyRead( const QHttpResponseHeader & ) ),
	   this, SLOT( handleResponseHeader( const QHttpResponseHeader & ) ) );
  connect( http, SIGNAL( hostLookupFailed() ),
	   this, SLOT( handleHostLookupFailed() ) );
}

void Catkin::updatePostCategories()
{
  if( entryBlogged )
    setPostCategories();
  else
    statusBar()->showMessage( tr( "This entry has not been posted yet." ), 2000 );
}

void Catkin::setPostCategories()
{
  QDomDocument doc;
  QString secCatId, secCatName;
  QDomElement cat;

#ifndef NO_DEBUG_OUTPUT
  qDebug() << "starting to post categories";
#endif
  if( categoriesEnabled ) {
    if( !currentHttpBusiness ) {
      QDomElement methodCall = doc.createElement( "methodCall" );
      methodCall.appendChild( XmlMethodName( doc, "mt.setPostCategories" ) );
      QDomElement params = doc.createElement( "params" );
      params.appendChild( XmlValue( doc, "string", entryNumber ) );
      params.appendChild( XmlValue( doc, "string", login ) );
      params.appendChild( XmlValue( doc, "string", password ) );

      QDomElement param = doc.createElement( "param" );
      QDomElement value = doc.createElement( "value" );
      QDomElement array = doc.createElement( "array" );
      QDomElement data = doc.createElement( "data" );
      QDomElement arrayValue = doc.createElement( "value" );
      QDomElement arrayStruct = doc.createElement( "struct" );
#ifndef NO_DEBUG_OUTPUT
      qDebug() << "posting prim cat";
#endif
      cat = currentBlogElement.firstChildElement( "categories" )
          .childNodes().at( cw.cbMainCat->currentIndex() ).toElement();
      //QString primCat = cat.firstChildElement( "categoryId" ).text();
      QString primCat = cw.cbMainCat->itemData( cw.cbMainCat->currentIndex() ).toString();
      //QString primCat = categoryIDs.value( cw.cbMainCat->currentIndex() );
      //	.section( " ##ID:", 1, 1 );
#ifndef NO_DEBUG_OUTPUT
      qDebug() << "posted prim cat";
#endif
      //	.section( ";", 0, 0 );
      arrayStruct.appendChild( XmlMember( doc, "categoryId", "string", primCat ) );
      arrayStruct.appendChild( XmlMember( doc, "isPrimary", "boolean", "1" ) );
      arrayValue.appendChild( arrayStruct );
      data.appendChild( arrayValue );

      for( int a = 0; a < cw.lwOtherCats->count(); a++ ) {
	if( cw.lwOtherCats->isItemSelected( cw.lwOtherCats->item( a ) ) ) {
	  cat = currentBlogElement.firstChildElement( "categories" ).childNodes().at( a ).toElement();
	  secCatId = cw.cbMainCat->itemData( a ).toString();
	  secCatName = cw.cbMainCat->itemText( a );
	  arrayValue = doc.createElement( "value" );
	  arrayStruct = doc.createElement( "struct" );
	  arrayStruct.appendChild( XmlMember( doc, "categoryId", "int", secCatId ) );
	  arrayStruct.appendChild( XmlMember( doc, "categoryName", "string",
					      secCatName ) );
	  arrayStruct.appendChild( XmlMember( doc, "isPrimary", "boolean", "0" ) );
	  arrayValue.appendChild( arrayStruct );
	  data.appendChild( arrayValue );
	}
      }

      array.appendChild( data );
      value.appendChild( array );
      param.appendChild( value );
      params.appendChild( param );
      methodCall.appendChild( params );
      doc.appendChild( methodCall );
      doc.insertBefore( doc.createProcessingInstruction( "xml",
							 "version=\"1.0\" encoding=\"UTF-8\"" ),
			doc.firstChild() );
      QByteArray requestArray( doc.toByteArray( 2 ) );

      responseData = "";
      // http = new SafeHttp;
      QHttpRequestHeader header( "POST", location );
      header.setValue( "Host", server );
      header.setValue( "User-Agent", userAgentString );
      http->setHost( server );
      http->request( header, requestArray );

      addToConsole( header.toString() );
      addToConsole( doc.toString() );

      if( QApplication::overrideCursor() == 0 )
	QApplication::setOverrideCursor( QCursor( Qt::BusyCursor ) );
#ifndef NO_DEBUG_OUTPUT
      qDebug() << "posted categories";
#endif
      currentHttpBusiness = 15; // Processing mt.setPostCategories request
      connect( http, SIGNAL( done( bool ) ),
	       this, SLOT( handleDone( bool ) ) );
      connect( http, SIGNAL( readyRead( const QHttpResponseHeader & ) ),
	       this, SLOT( handleResponseHeader( const QHttpResponseHeader & ) ) );
      connect( http, SIGNAL( hostLookupFailed() ),
	       this, SLOT( handleHostLookupFailed() ) );
    }
    else {
#ifdef QTM_DEBUG
      statusBar()->showMessage( tr( "setPostCategories: All HTTP requests are blocked" ),
				2000 );
#else
      statusBar()->showMessage( tr( "All HTTP requests are blocked." ), 2000 );
#endif
      QApplication::restoreOverrideCursor();
    }
  }
  else {
    // An override cursor might have been set when posting an entry in a
    // non-category-enabled blog
    if( QApplication::overrideCursor() != 0 )
      QApplication::restoreOverrideCursor();
  }
}

void Catkin::publishPost() // slot
{
  QDomDocument doc;

  if( !currentHttpBusiness ) {
    QDomElement methodCall = doc.createElement( "methodCall" );
    methodCall.appendChild( XmlMethodName( doc, "mt.publishPost" ) );
    QDomElement params = doc.createElement( "params" );

    params.appendChild( XmlValue( doc, "string", entryNumber ) );
    params.appendChild( XmlValue( doc, "string", login ) );
    params.appendChild( XmlValue( doc, "string", password ) );
    methodCall.appendChild( params );
    doc.appendChild( methodCall );
    doc.insertBefore( doc.createProcessingInstruction( "xml",
						       "version=\"1.0\" encoding=\"UTF-8\"" ),
		      doc.firstChild() );

    QByteArray requestArray( doc.toByteArray() );

    responseData = "";
    QHttpRequestHeader header( "POST", location );
    header.setValue( "Host", server );
    header.setValue( "User-Agent", userAgentString );
    http->setHost( server );
    http->request( header, requestArray );

    addToConsole( header.toString() );
    addToConsole( doc.toString() );

    if( QApplication::overrideCursor() == 0 )
      QApplication::setOverrideCursor( QCursor( Qt::BusyCursor ) );
    currentHttpBusiness = 12; // Processing mt.publishPost request
    connect( http, SIGNAL( done( bool ) ),
	     this, SLOT( handleDone( bool ) ) );
    connect( http, SIGNAL( readyRead( const QHttpResponseHeader & ) ),
	     this, SLOT( handleResponseHeader( const QHttpResponseHeader & ) ) );
    connect( http, SIGNAL( hostLookupFailed() ),
	     this, SLOT( handleHostLookupFailed() ) );

  }
  else
    statusBar()->showMessage( tr( "Cannot publish; HTTP is blocked" ), 2000 );
}

void Catkin::saveAs()
{
  //int i;
  //recentFile thisFile;
  QString suggestedFilename;

  if( cw.leTitle->text().isEmpty() )
    suggestedFilename = localStorageDirectory;
  else {
    suggestedFilename = QString( "%1/%2" )
      .arg( localStorageDirectory )
      .arg( cw.leTitle->text().remove( QRegExp( "[\"/\\\\<>\\?:\\*\\|]+" ) ) );
  }

  QString extn = QString( "%1 (*.%2)" ).arg( tr( "Blog entries" ) )
    .arg( localStorageFileExtn );
  QString fn = QFileDialog::getSaveFileName( this, tr( "Choose a filename" ),
					     suggestedFilename, extn );

  if ( !fn.isEmpty() ) {
    if( !localStorageFileExtn.isEmpty() )
      if( !fn.endsWith( QString( ".%1" ).arg( localStorageFileExtn ) ) )
	fn.append( QString( ".%1" ).arg( localStorageFileExtn ) );
    filename = fn;
    /*
This section has been commented out as unnecessary; the getSaveFileName function
already checks for the existence of a file.  Keeping in code until tested on other platforms.

 if( QFile::exists( fn ) ) {
      if( !QMessageBox::warning( 0, tr( "File exists" ),
				 tr( "A file by this name already exists. Continue?" ),
				 QMessageBox::Yes, QMessageBox::No | QMessageBox::Default ) ) {
	QFile::remove( fn );
	save( fn );
      }
    }
    else */
    save( fn );

    /*    for( i = 0; i < 20; ++i ) {
      if( i == recentFiles.count() ) {
	thisFile.title = cw.leTitle->text();
	thisFile.filename = fn;
	recentFiles.prepend( thisFile );
	break;
      }
      if( recentFiles.value( i ).filename == fn ) {
	recentFiles.removeAt( i );
	recentFiles.prepend( recentFiles.value( i ) );
	break;
      }
      }*/
    qtm->addRecentFile( cw.leTitle->text(), fn );
  }
  else
    statusBar()->showMessage( tr("Saving aborted"), 2000 );
}


void Catkin::save()
{
  if ( filename.isEmpty() ) {
    saveAs();
    return;
  }

  save( filename );
}

void Catkin::save( const QString &fname )
{
  int count, tags;
  QString text = EDITOR->document()->toPlainText();
  QFile f( fname );

  if( !f.open( QIODevice::WriteOnly ) ) {
    statusBar()->showMessage( tr( "Could not write to %1" ).arg(fname),
			  2000 );
    return;
  }

  QTextStream out( &f );
  out << "QTM saved blog entry v3.0\n";
  out << QString( "Title:%1\n" ).arg( cw.leTitle->text() );
  out << QString( "Publish:%1\n" ).arg( QString::number( cw.cbStatus->currentIndex() ) );
  if( entryBlogged )
    out << QString( "EntryNumber:%1\n" ).arg( entryNumber );
  out << QString( "Comments:%1\n" ).arg( cw.chComments->isChecked() ? "1" : "0" );
  out << QString( "TB:%1\n" ).arg( cw.chTB->isChecked() ? "1" : "0" );
  /*  out << QString( "Server:%1\n" ).arg( server );
  out << QString( "Location:%1\n" ).arg( location );
  out << QString( "Login:%1\n" ).arg( login );
  if( savePassword )
    out << QString( "Password:%1\n" ).arg( password );
  if( cw.lwTags->count() ) {
  tags = cw.lwTags->count(); */
  out << QString( "AcctBlog:%1@%2 (%3)" ) // Include the blog name so it can be relayed to the user later
    .arg( currentBlogid )
    .arg( currentAccountId )
    .arg( cw.cbBlogSelector->itemText( cw.cbBlogSelector->currentIndex() ) );
  out << "Tags:";
  for( count = 0; count < tags; count++ ) {
    out << QString( count ? ";%1" : "%1" )
      .arg( cw.lwTags->item( count )->text().replace( ' ', '+' ) );
  }
  out << "\n";

  QDomNodeList catNodeList = currentBlogElement.firstChildElement( "categories" ).elementsByTagName( "category" );
  out << QString( "Blog:%1\n" ).arg( cw.cbBlogSelector->currentIndex() );
  out << QString( "PrimaryID:%1\n" ).arg( cw.cbMainCat->itemData( cw.cbMainCat->currentIndex() ).toString() );
  QString catsList;
  int cats = 0;
  for( int a = 0; a < cw.lwOtherCats->count(); a++ ) {
    if( cw.lwOtherCats->isItemSelected( cw.lwOtherCats->item( a ) ) ) {
      if( cats )
	catsList.append( QString( ";%1" ).arg( cw.cbMainCat->itemData( a ).toString() ) );
      else
	catsList.append( cw.cbMainCat->itemData( a ).toString() );
      cats++;
    }
  }
  out << QString( "CatIDs:%1\n" ).arg( catsList );
  if( cw.teExcerpt->toPlainText().length() > 0 )
    out << QString( "Excerpt:%1\n" )
      .arg( cw.teExcerpt->toPlainText().replace( QChar( '\n' ), "\\n" ) );
  out << QString( "Text:\n%1" ).arg( text );

  dirtyIndicator->hide();
  setWindowModified( false );
  connect( EDITOR, SIGNAL( textChanged() ), this, SLOT( dirtify() ) );

  entryEverSaved = true;
}

void Catkin::choose( QString fname )
{
  //int i;
  //recentFile thisFile;
  QString fn;
  QString extn( QString( "%1 (*.%2)" ).arg( tr( "Blog entries" ) )
		.arg( localStorageFileExtn ) );

  if( fname.isEmpty() )
    fn = QFileDialog::getOpenFileName( this, tr( "Choose a file to open" ),
				    localStorageDirectory, extn );
  else
    fn = fname;

  if ( !fn.isEmpty() ) {
    if( !useNewWindows ) {
      if( saveCheck( true ) ) {
	if( !load( fn ) )
	  statusBar()->showMessage( tr( "File could not be loaded." ), 2000 );
	else {
	  /*	  thisFile.title = cw.leTitle->text();
	  thisFile.filename = fn;
	  addToConsole( QString( "filename: %1\ntitle: %2" )
			.arg( thisFile.filename ).arg( thisFile.title ) );
	  for( i = 0; i < 20; ++i ) {
	    if( i == recentFiles.count() )
	      break;
	    if( recentFiles.value( i ).filename == fn )
	      recentFiles.removeAt( i );
	  }
	  recentFiles.prepend( thisFile ); */

	  //	  updateRecentFileList( cw.leTitle->text(), fn );
	  qtm->addRecentFile( cw.leTitle->text(), fn );
	  //updateRecentFileMenu();

	  dirtyIndicator->hide();
	  setWindowModified( false );
	  connect( EDITOR, SIGNAL( textChanged() ), this, SLOT( dirtify() ) );
	}
      }
    }
    else {
      // Catkin *e = new Catkin( usersBlogs, categoryList, currentBlog, 0 );
      Catkin *e = new Catkin( true );
      //if( e->load( fn, usersBlogs, categoryList ) ) {
      //if( e->load( fn, accountsDom ) ) {
      if( e->load( fn, true ) ) {
#ifdef USE_SYSTRAYICON
	e->setSTI( sti );
#endif

	positionWidget( e, this );
	/*	thisFile.title = cw.leTitle->text();
	thisFile.filename = fn;
	addToConsole( QString( "filename: %1\ntitle: %2" )
		      .arg( thisFile.filename ).arg( thisFile.title ) );
	for( i = 0; i < 20; ++i ) {
	  if( i == recentFiles.count() )
	    break;
	  if( recentFiles.value( i ).filename == fn )
	    recentFiles.removeAt( i );
	}
	recentFiles.prepend( thisFile ); */
	qtm->addRecentFile( e->postTitle(), fn );
	//updateRecentFileMenu();
	//e->setRecentFiles( recentFiles );

	e->show();
      }
      else {
	statusBar()->showMessage( "Loading of new window failed", 2000 );
	e->deleteLater();
      }
    }
  }
  else
    statusBar()->showMessage( tr("Loading aborted"), 2000 );
}

void Catkin::updateRecentFileList( const QString &title, const QString &filename )
{
  Application::recentFile thisFile;
  int i;

  thisFile.title = title;
  thisFile.filename = filename;

  for( i = 0; i < 20; ++i ) {
    if( i == recentFiles.count() )
      break;
    if( recentFiles.value( i ).filename == filename )
      recentFiles.removeAt( i );
  }
  recentFiles.prepend( thisFile );
}

/*bool Catkin::load( const QString &fname, QList<QString> uB,
		   QList<QString> cl )
{
  usersBlogs = uB;
  categoryList = cl;

  /*  qDebug( "outputting blog list" );
  Q_FOREACH( QString _blog, usersBlogs ) {
    addToConsole( QString( "%1\n" ).arg( _blog ) );
    }*
  return load( fname );
  }*/

/* bool Catkin::load( const QString &fname,
		   QStringList uBids, QStringList uBnames, QStringList uBuris,
		   QStringList clids, QStringList clnames ) */
bool Catkin::load( const QString &fname, QDomDocument &dd )
{
  /*  usersBlogIDs = uBids;
  usersBlogNames = uBnames;
  usersBlogURIs = uBuris;
  categoryIDs = clids;
  categoryNames = clnames; */
  accountsDom = dd;
  /*currentAccountElement = accountsDom.firstChildElement( "QTMAccounts" )
    .firstChildElement( "account" );
    populateBlogList(); */

  return load( fname );
}


bool Catkin::load( const QString &fname, bool fromSTI )
{
  QDialog *pwd = 0;
  Ui::dPassword pui;
  addToConsole( "Starting load" );
  QMap<QString, QString> emap;
  QString currentLine, key, value, fetchedText, tags;
  QStringList otherCatStringList;
  QDomNodeList accts;
  bool getDetailsAgain = false;
  bool isOK;
  int b, c, d;
  noAutoSave = true;
  QFile f( fname );

  ui.actionSave_blogs->setEnabled( true );
  ui.actionSave_blogs->setVisible( true );

  if( fromSTI ) {
    accountsElement = accountsDom.createElement( "QTMAccounts" );
    currentAccountElement = accountsDom.createElement( "account" );
    accountsElement.appendChild( currentAccountElement );
    accountsDom.appendChild( accountsElement );
    accountsDom.insertBefore( accountsDom.createProcessingInstruction( "xml", "version=\"1.0\"" ),
			      accountsDom.firstChild() );
  }


  if( !f.open( QIODevice::ReadOnly ) ) {
    addToConsole( "Cannot open file" );
    statusBar()->showMessage( tr( "Cannot open file." ), 2000 );
    return false;
  }

  QTextStream t( &f );
  if( !t.readLine().startsWith( "QTM saved blog entry" ) ) {
    addToConsole( "Not a QTM blog entry" );
    //qDebug( "Not a QTM blog entry.\n" );
    statusBar()->showMessage( tr( "This is not a QTM blog entry." ), 2000 );
    return false;
  }

  do {
    currentLine = t.readLine();
    if( currentLine.startsWith( "Text:" ) )
      break;
    key = currentLine.section( QChar( ':' ), 0, 0 );
    value = currentLine.section( QChar( ':' ), 1 );
    emap[key] = value;
    addToConsole( currentLine );
  } while( !t.atEnd() );

  if( emap.contains( "Title" ) ) {
    cw.leTitle->setText( emap.value( "Title" ) );
    setWindowTitle( QString( "%1 - QTM [*]" ). arg( emap.value( "Title" ) ) );
  }

  if( emap.contains( "Publish" ) )
    cw.cbStatus->setCurrentIndex( emap.value( "Publish" ) == "1" ? 1 : 0 );

  if( emap.contains( "EntryNumber" ) ) {
    entryNumber = emap.value( "EntryNumber" );
    entryBlogged = true;
  }

  if( emap.contains( "Comments" ) )
    cw.chComments->setChecked( emap.value( "Comments" ) == "1" ? true : false );

  if( emap.contains( "TB" ) )
    cw.chTB->setChecked( emap.value( "TB" ) == "1" ? true : false );

  if( emap.contains( "Server" ) ) {
    if( emap.value( "Server" ) != server ) {
      server = emap.value( "Server" );
      getDetailsAgain = true;
    }
  }

  if( emap.contains( "Location" ) ) {
    if( emap.value( "Location" ) != location ) {
      location = emap.value( "Location" );
      getDetailsAgain = true;
    }
  }

  if( emap.contains( "Login" ) ) {
    if( emap.value( "Login" ) != login ) {
      login = emap.value( "Login" );
      getDetailsAgain = true;
    }
  }

  if( emap.contains( "Password" ) ) {
    if( emap.value( "Password" ) != password ) {
      password = emap.value( "Password" );
      getDetailsAgain = true;
    }
    noPassword = false;
  } else {
    password = "";
    noPassword = true;
  }

  if( emap.contains( "Tags" ) ) {
    tags = emap.value( "Tags" );
    d = tags.count( QChar( ';' ) );
    if( d ) {
      for( c = 0; c <= d; c++ )
	cw.lwTags->addItem( tags.section( QChar( ';' ), c, c ) );
    } else
      cw.lwTags->addItem( tags );
  }

  if( emap.contains( "Blog" ) ) {
    b = emap.value( "Blog" ).toInt( &isOK );
    if( isOK ) {
      currentBlog = b;
      loadedEntryBlog = b;
    }
  }

  if( emap.contains( "AcctBlog" ) ) {
    loadedAccountId = emap.value( "AcctBlog" ).section( "@", 1, 1 ).section( " (", 0, 0 );
    b = emap.value( "AcctBlog" ).section( "@", 0, 0 ).toInt( &isOK );
    if( isOK ) {
      currentBlog = b;
      loadedEntryBlog = b;
    }
  }

  /*
  if( emap.contains( "BlogID" ) ) {
    b = emap.value( "BlogID" ).toInt( &isOK );
    if( isOK ) {
      currentBlog = b;
      loadedEntryBlog = b;
    }
    }*/

  // Get old-style index categories
  if( emap.contains( "Primary" ) ) {
    b = emap.value( "Primary" ).toInt( &isOK );
    if( isOK ) {
      primaryCat = b;
      noAlphaCats = true;
      getDetailsAgain = true;
    }
    else {
      primaryCat = 0;
      addToConsole( tr( "Assignation of primary category failed" ) );
    }
  }

  if( emap.contains( "Cats" ) ) {
    otherCatsList = emap.value( "Cats" );
    otherCatStringList = otherCatsList.split( ';' );
    noAlphaCats = true;
    getDetailsAgain = true;
  }

  // Get new-style ID categories
  if( emap.contains( "PrimaryID" ) ) {
    b = emap.value( "PrimaryID" ).toInt( &isOK );
    if( isOK )
      primaryCat = b;
    else {
      primaryCat = 0;
      addToConsole( tr( "Assignation of primary category failed" ) );
    }
  }

  if( emap.contains( "CatIDs" ) )
    otherCatsList = emap.value( "CatIDs" );

  if( emap.contains( "Excerpt" ) )
    cw.teExcerpt->setPlainText( QString( emap.value( "Excerpt" ) ).replace( "\\n", "\n" ) );

  while( !t.atEnd() )
    fetchedText += QString( "%1\n" ).arg( t.readLine() );

  EDITOR->setPlainText( fetchedText );
  f.close();
  connect( EDITOR, SIGNAL( textChanged() ), this, SLOT( dirtify() ) );
  dirtyIndicator->hide();
  setWindowModified( false );

  // First of all, deal with entries saved to accounts

  if( !loadedAccountId.isNull() ) {
    accts = accountsDom.elementsByTagName( "account" );
    for( int g = 0; g <= accts.count(); g++ ) {
      if( g == accts.count() ) {
	// i.e. if it gets to the end of the accounts tree without finding the account
	QMessageBox::information( 0, tr( "QTM - No such account" ),
				  tr( "QTM could not find this account (perhaps it was deleted).\n\n"
				      "Will set up a blank default account; you will need to fill in the access"
				      "details by choosing Accounts from the File menu." ),
				  QMessageBox::Ok );
	QDomElement newDefaultAccount = accountsDom.createElement( "account" );
	newDefaultAccount.setAttribute( "id", QString( "newAccount_%1" ).arg( QDateTime::currentDateTime().toString( Qt::ISODate ) ) );
	QDomElement newDetailElement = accountsDom.createElement( "details" );
	QDomElement newNameElement = accountsDom.createElement( "name" );
	newNameElement.appendChild( QDomText( accountsDom.createTextNode( tr( "New blank element" ) ) ) );
	newDetailElement.appendChild( newNameElement );
	newDefaultAccount.appendChild( newDetailElement );
	accountsDom.documentElement().appendChild( newDefaultAccount );
	currentAccountElement = newDefaultAccount;
	return true;
      }

      if( accts.at( g ).toElement().attribute( "id" ) == loadedAccountId ) {
	populateAccountList();
	currentAccountElement = accts.at( g ).toElement();

	QString st;
	for( int h = 0; h < cw.cbAccountSelector->count(); h++ ) {
	  st = cw.cbAccountSelector->itemData( h ).toString();
 	  if( st == loadedAccountId )
	    cw.cbAccountSelector->setCurrentIndex( h );
	}

	// Now populate the blog list
	QDomNodeList blogNodeList = currentAccountElement.elementsByTagName( "blog" );
	cw.cbBlogSelector->clear();
	for( int hh = 0; hh < blogNodeList.count(); hh++ ) {
	  cw.cbBlogSelector->addItem( blogNodeList.at( hh ).toElement()
				      .firstChildElement( "blogName" ).text(),
				      blogNodeList.at( hh ).toElement()
				      .firstChildElement( "blogid" ).text() );
	  cw.cbBlogSelector->disconnect( SIGNAL( activated( int ) ), this, 0 );
	  connect( cw.cbBlogSelector, SIGNAL( activated( int ) ),
		   this, SLOT( changeBlog( int ) ) );
	}
	if( !isOK ) {
	  QMessageBox::information( 0, tr( "QTM - Invalid blog" ),
				    tr( "Could not get a valid blog number.  Please set it again." ),
				    QMessageBox::Ok );
	  return true;
	}
	
	// Now populate and set the categories
	cw.cbBlogSelector->setCurrentIndex( currentBlog );
	QDomElement catsElement = blogNodeList.at( currentBlog ).firstChildElement( "categories" );
	if( !catsElement.isNull() ) {
	  QDomNodeList catNodeList = catsElement.elementsByTagName( "category" );
	  int b = catNodeList.count();
	  if( b ) {
	    for( int j = 0; j < b; j++ ) {
	      cw.cbMainCat->addItem( catNodeList.at( j ).firstChildElement( "categoryName" ).text(),
				     QVariant( catNodeList.at( j ).firstChildElement( "categoryId" ).text() ) );
	      cw.lwOtherCats->addItem( catNodeList.at( j ).firstChildElement( "categoryName" ).text() );
	    }
	    for( int i = 0; i < catNodeList.size(); i++ ) {
	      QString cc = catNodeList.at( i ).firstChildElement( "categoryId" ).text();
	      if( cc == QString::number( primaryCat ) )
		cw.cbMainCat->setCurrentIndex( i );
	      else {
		if( otherCatStringList.contains( cc ) )
		  cw.lwOtherCats->setItemSelected( cw.lwOtherCats->item( i ), true );
	      }
	    }
	  }
	  else {
	    cw.cbMainCat->setEnabled( false );
	    cw.lwOtherCats->setEnabled( false );
	  }
	}
	return true;
      }
    }
    return true;
  }

  // Now we know this isn't an account entry, check whether the saved details actually
  // belong to an account; if it does, there is no need to check for the password


  QDomElement details; 
  QDomNodeList blogs;


  accts = accountsDom.documentElement().elementsByTagName( "account" );
  for( int e = 0; e <= accts.count(); e++ ) {
    if( e == accts.count() ) {

    }
    details = accts.at( e ).toElement().firstChildElement( "details" );
    if( details.firstChildElement( "server" ).text() == server &&
	details.firstChildElement( "location" ).text() == location &&
	details.firstChildElement( "login" ).text() == login ) {
      currentAccountElement = accts.at( e ).toElement();
      // First check whether the blog still exists
      blogs = currentAccountElement.elementsByTagName( "blogs" );
      if( currentBlog > blogs.count() ) {
	// Message box telling the user to set the blog and category himself cos the blog is gone
	return true;
      }
      else {
	// currentBlogAccount = blogs.at( currentBlog );
	setLoadedPostCategories();
	return true;
      }

    }

   
  }

  if( noPassword ) {
    QDialog pwd;
    //pwd = new QDialog;
    pui.setupUi( &pwd );
    if( pwd.exec() ) {
      password = pui.lePassword->text();
    }
    else
      QMessageBox::warning( 0, tr( "No password" ),
			    tr( "This entry was saved without a password.\n"
				"You will need to set one, using the\n"
				"Preferences window." ),
			    QMessageBox::Ok, QMessageBox::NoButton );
      
  }

  QDomElement newAcct, newDetails, newServer, newLocation, newLogin, newPwd;
  newAcct = accountsDom.createElement( "account" );
  newAcct.setAttribute( "id", tr( "newAccount_%1" ).arg( QDateTime::currentDateTime().toString( Qt::ISODate ) ) );
  newDetails = accountsDom.createElement( "details" );
  newServer = accountsDom.createElement( "server" );
  newServer.appendChild( QDomText( accountsDom.createTextNode( server ) ) );
  newLocation = accountsDom.createElement( "location" );
  newLocation.appendChild( QDomText( accountsDom.createTextNode( location ) ) );
  newLogin = accountsDom.createElement( "login" );
  newLogin.appendChild( QDomText( accountsDom.createTextNode( login ) ) );
  newPwd = accountsDom.createElement( "password" );
  newPwd.appendChild( QDomText( accountsDom.createTextNode( password ) ) );
  newDetails.appendChild( newServer );
  newDetails.appendChild( newLocation );
  newDetails.appendChild( newLogin );
  newDetails.appendChild( newPwd );
  newAcct.appendChild( newDetails );
  accountsDom.documentElement().appendChild( newAcct );
  currentBlogElement = newAcct;

  connect( this, SIGNAL( categoryRefreshFinished() ),
	   this, SLOT( setLoadedPostCategories() ) );
  refreshBlogList();

  filename = fname;
  entryEverSaved = true;

  return true;

  // This is the old routine, commented-out

  /*
  if( fromSTI )
    getDetailsAgain = true;

  if( !noPassword ) {
    if( getDetailsAgain ) {
      initialChangeBlog = true;
      //connect( this, SIGNAL( httpBusinessFinished() ),
	this, SLOT( doInitialChangeBlog() ) );
      connect( this, SIGNAL( categoryRefreshFinished() ),
	       this, SLOT( setLoadedPostCategories() ) );
      refreshBlogList();
    }
    else {
      if( currentBlog != cw.cbBlogSelector->currentIndex() ) {
	cw.cbBlogSelector->setCurrentIndex( currentBlog );
	changeBlog( currentBlog );
	connect( this, SIGNAL( categoryRefreshFinished() ),
		 this, SLOT( setLoadedPostCategories() ) );
      }
      else
	setLoadedPostCategories();
    }
  } else {
    if( !getDetailsAgain && (currentBlog == cw.cbBlogSelector->currentIndex()) ) {
      // No need for a password
      setLoadedPostCategories();
    }
    else {
      QDialog pwd;
      // pwd = new QDialog;
      pui.setupUi( &pwd );
      if( pwd.exec() ) {
	password = pui.lePassword->text();
	if( getDetailsAgain ) {
	  //qDebug( "Now getting blog list" );
	  refreshBlogList();
	  //connect( this, SIGNAL( httpBusinessFinished() ),
	  //  this, SLOT( doInitialChangeBlog() ) );
	  connect( this, SIGNAL( categoryRefreshFinished() ),
		   this, SLOT( setLoadedPostCategories() ) );
	}
	else {
	  QDomNodeList blogNodes = currentAccountElement.firstChildElement( "blogs" )
	    .elementsByTagName( "blog" );
	  if( !cw.cbBlogSelector->count() ) {
	    //for( int z = 0; z < usersBlogs.size(); z++ )
	    //  cw.cbBlogSelector->addItem( usersBlogs[z].section( "blogName:", 1 )
	    //  .section( ";", 0, 0 ) );
	    int i = blogNodes.size();
	    //	      usersBlogIDs.size() : usersBlogNames.size();
	    for( int z = 0; z < i; z++ )
	      cw.cbBlogSelector->addItem( blogNodes.at( z ).firstChildElement( "blogName" )
					  .text(),
					  QVariant( blogNodes.at( z ).firstChildElement( "blogid" ).text() ) );
	  }
	  cw.cbBlogSelector->setCurrentIndex( currentBlog );
	  currentBlogElement = blogNodes.at( currentBlog ).toElement();
	  //qDebug() << "doing the change";
	  connect( this, SIGNAL( categoryRefreshFinished() ),
		   this, SLOT( setLoadedPostCategories() ) );
	  changeBlog( currentBlog );
	}
      }
      else
	QMessageBox::warning( 0, tr( "No password" ),
			  tr( "This entry was saved without a password.\n"
			      "You will need to set one, using the\n"
			      "Preferences window." ),
			      QMessageBox::Ok, QMessageBox::NoButton );
   }
  }
  */

}

void Catkin::setLoadedPostCategories() // slot
{
  int a, j, z;
  int i = 0;
  QString c, cc, d;
  bool isOK;

  disconnect( SIGNAL( categoryRefreshFinished() ) );
  disconnect( SIGNAL( httpBusinessFinished() ) );

  QDomNodeList blogNodes = currentAccountElement.firstChildElement( "blogs" )
    .elementsByTagName( "blog" );

  if( !cw.cbBlogSelector->count() ) {
    j = blogNodes.size();
    for( z = 0; z < j; z++ )
      cw.cbBlogSelector->addItem( blogNodes.at( z ).firstChildElement( "blogName" ).text(),
				  QVariant( blogNodes.at( z ).firstChildElement( "blogName" ).text() ) );
    cw.cbBlogSelector->setCurrentIndex( currentBlog );
  }

  currentBlogElement = blogNodes.at( currentBlog ).toElement();
  //qDebug() << "current blog is" << currentBlogElement.firstChildElement( "blogName" ).text();

  //qDebug( "Just populated blog selector" );

  if( noAlphaCats ) {
    cw.cbMainCat->setCurrentIndex( primaryCat );

    QString f( otherCatsList.section( QChar( ';' ), -1, -1 ) );

    do {
      c = otherCatsList.section( QChar( ';' ), i, i );
      a = c.toInt( &isOK );
      if( isOK ) {
	cw.lwOtherCats->setItemSelected( cw.lwOtherCats->item( a ), true );
      }
      else
	break;
      i++;
    } while( c != f );
    initialChangeBlog = false;
  }
  else {
    QDomNodeList catNodes = currentBlogElement.firstChildElement( "categories" ).elementsByTagName( "category" );
    QStringList catStringList = otherCatsList.split( ";" );
    //qDebug() << "current Blog is" << currentBlogElement.firstChildElement( "blogName" ).text();

    for( i = 0; i < catNodes.size(); i++ ) {
      cc = catNodes.at( i ).firstChildElement( "categoryId" ).text();
      if( cc == QString::number( primaryCat ) )
	cw.cbMainCat->setCurrentIndex( i );
      else {
	if( catStringList.contains( cc ) )
	  cw.lwOtherCats->setItemSelected( cw.lwOtherCats->item( i ), true );
      }
    }
    initialChangeBlog = false;
  }
}

void Catkin::saveBlogs()
{
  QFile domOut( PROPERSEPS( QString( "%1/qtmaccounts.xml" ).arg( localStorageDirectory ) ) );
  if( domOut.open( QIODevice::WriteOnly ) ) {
    QTextStream domFileStream( &domOut );
    accountsDom.save( domFileStream, 2 );
    domOut.close();
  }
  else
    statusBar()->showMessage( tr( "Could not write to accounts file (error %1)" ).arg( (int)domOut.error() ), 2000 );
}

void Catkin::uploadFile()
{
  QString fileInBase64;
  QByteArray conversionBuffer;
  QFile *inFile;
  QDomDocument doc;

  if( !currentHttpBusiness ) {
    QString uploadFilename = QFileDialog::getOpenFileName( this,
							   tr( "Select file to upload" ),
							   QDir::homePath() );
    if( !uploadFilename.isEmpty() ) {
      if( QFile::exists( uploadFilename ) ){
	inFile = new QFile( uploadFilename );
	if( inFile->open( QIODevice::ReadOnly ) ) {
	  conversionBuffer = inFile->readAll();
	  if( conversionBuffer.isEmpty() )
	    statusBar()->showMessage( tr( "This file was empty, or an error occurred." ), 2000 );
	  else {
	    QApplication::processEvents();
	    fileInBase64 = toBase64( conversionBuffer );

	    if( !fileInBase64.isEmpty() ) {
	      QDomElement methodCall = doc.createElement( "methodCall" );
	      methodCall.appendChild( XmlMethodName( doc, "metaWeblog.newMediaObject" ) );
	      QDomElement params = doc.createElement( "params" );

	      /*QString blogid = usersBlogs[currentBlog].section( "blogid:", 1, 1 )
		.section( ";", 0, 0 );*/
	      QString blogid = currentBlogElement.firstChildElement( "blogid" ).text();
	      QRegExp blogidRegExp( "^[0-9]+$" );
	      bool blogidIsInt = blogidRegExp.exactMatch( blogid );

	      params.appendChild( XmlValue( doc, blogidIsInt ? "int" : "string", blogid ) );
	      params.appendChild( XmlValue( doc, "string", login ) );
	      params.appendChild( XmlValue( doc, "string", password ) );

	      QDomElement param = doc.createElement( "param" );
	      QDomElement value = doc.createElement( "value" );
	      QDomElement rpcStruct = doc.createElement( "struct" );
	      rpcStruct.appendChild( XmlMember( doc, "bits", "base64", fileInBase64 ) );
	      rpcStruct.appendChild( XmlMember( doc, "name", "string",
						QFileInfo( uploadFilename ).fileName() ) );
	      value.appendChild( rpcStruct );
	      param.appendChild( value );
	      params.appendChild( param );

	      methodCall.appendChild( params );
	      doc.appendChild( methodCall );
	      doc.insertBefore( doc.createProcessingInstruction( "xml",
								 "version=\"1.0\" encoding=\"UTF-8\"" ),
				doc.firstChild() );

	      currentHttpBusiness = 11;

	      QByteArray requestArray( doc.toByteArray() );
	      responseData = "";
	      QHttpRequestHeader header( "POST", location );
	      header.setValue( "Host", server );
	      header.setValue( "User-Agent", userAgentString );
	      http->setHost( server );
	      http->request( header, requestArray );

	      addToConsole( header.toString() );
	      addToConsole( doc.toString() );

	      if( QApplication::overrideCursor() == 0 )
		QApplication::setOverrideCursor( QCursor( Qt::BusyCursor ) );

	      connect( http, SIGNAL( done( bool ) ),
		       this, SLOT( handleDone( bool ) ) );
	      connect( http, SIGNAL( readyRead( const QHttpResponseHeader & ) ),
		       this, SLOT( handleResponseHeader( const QHttpResponseHeader & ) ) );
	      connect( http, SIGNAL( hostLookupFailed() ),
		       this, SLOT( handleHostLookupFailed() ) );
#ifndef NO_DEBUG_OUTPUT
	      qDebug() << "Posted image.";
#endif
	    }
	    else
	      statusBar()->showMessage( tr( "Upload was cancelled." ), 2000 );
	  }
	  inFile->close();
	}
	else
	  statusBar()->showMessage( tr( "Could not open the file." ), 2000 );
      }
      else
	statusBar()->showMessage( tr( "That file could not be found." ), 2000 );
    }
  }
  else
    statusBar()->showMessage( tr( "HTTP requests are blocked." ), 2000 );
}

void Catkin::doWhatsThis()
{
  QWhatsThis::enterWhatsThisMode();
}

void Catkin::doViewBasicSettings()
{
  cw.cbPageSelector->setCurrentIndex( 0 );
}

void Catkin::doViewCategories()
{
  cw.cbPageSelector->setCurrentIndex( 1 );
}

void Catkin::doViewExcerpt()
{
  cw.cbPageSelector->setCurrentIndex( 2 );
  cw.teExcerpt->setFocus();
}

void Catkin::doViewTechTags()
{
  cw.cbPageSelector->setCurrentIndex( 3 );
}

void Catkin::doViewTBPings()
{
  cw.cbPageSelector->setCurrentIndex( 4 );
}

void Catkin::addTechTag()
{
  cw.cbPageSelector->setCurrentIndex( 3 );
  cw.leAddTag->setFocus();
}

void Catkin::addClipTag()
{
  int l;
  QRegExpValidator tagFormat( QRegExp( "^http:\\/\\/(www\\.)?technorati\\.com\\/tag\\/([a-zA-Z0-9\\.%]+[\\+ ])*[a-zA-Z0-9\\.%]+$" ), this );
  QString tagText = QApplication::clipboard()->text();

  cw.cbPageSelector->setCurrentIndex( 3 );
  addToConsole( QString( "Validating %1" ).arg( tagText ) );
  if( tagFormat.validate( tagText, l ) != QValidator::Acceptable ) {
    statusBar()->showMessage( tr( "This is not a valid tag." ), 2000 );
  } else {
    statusBar()->showMessage( tr( "This tag validates OK." ), 2000 );
    tagText.remove( QRegExp( "(http:\\/\\/)?(www\\.)?technorati\\.com\\/tag\\/" ) );
    cw.lwTags->addItem( tagText );
  }
}

void Catkin::addTechTagFromLineEdit()
{
  if( !cw.leAddTag->text().isEmpty() ) {
    cw.lwTags->addItem( cw.leAddTag->text() );
    cw.leAddTag->clear();
  }
}

void Catkin::addTechTagFromAddButton()
{
  int i;
  QString lineEditText = cw.leAddTag->text();

  if( !lineEditText.isEmpty() ) {
    if( tagValidator->validate( lineEditText, i ) == QValidator::Acceptable ) {
      statusBar()->showMessage( tr( "This tag validates." ), 2000 );
      cw.lwTags->addItem( cw.leAddTag->text() );
      cw.leAddTag->clear();
    } else {
      statusBar()->showMessage( tr( "This is not a valid tag." ), 2000 );
    }
  }
}

void Catkin::addTBPing()
{
  cw.cbPageSelector->setCurrentIndex( 4 );
  cw.leTBPingURL->setFocus();
}


void Catkin::addClipTBPing()
{
  QString clipboardText = QApplication::clipboard()->text();

  if( !clipboardText.isEmpty() ) {
    if( QUrl( clipboardText ).isValid() ) {
      statusBar()->showMessage( tr( "This URL validates." ) );
      cw.lwTBPings->addItem( clipboardText );
    } else
      statusBar()->showMessage( tr( "This is not a valid URL." ), 2000 );
  }
}

void Catkin::addTBPingFromLineEdit()
{
  QString lineEditText = cw.leTBPingURL->text();

  if( !lineEditText.isEmpty() ) {
    if( QUrl( lineEditText ).isValid() ) {
      statusBar()->showMessage( tr( "This URL validates." ), 2000 );
      cw.lwTBPings->addItem( lineEditText );
      cw.leTBPingURL->clear();
    } else
      statusBar()->showMessage( tr( "This is not a valid URL." ), 2000 );
  }
}

void Catkin::removeTechTag()
{
  int c = cw.lwTags->currentRow();
  cw.lwTags->takeItem( c );
}

void Catkin::addTBPingFromAddButton()
{
  QString lineEditText = cw.leTBPingURL->text();

  if( !lineEditText.isEmpty() ) {
    if( QUrl( lineEditText ).isValid() ) {
      statusBar()->showMessage( tr( "This URL validates." ), 2000 );
      cw.lwTBPings->addItem( cw.leTBPingURL->text() );
      cw.leTBPingURL->clear();
    } else
      statusBar()->showMessage( tr( "This is not a valid URL." ), 2000 );
  }
}

void Catkin::removeTBPing()
{
  int c = cw.lwTBPings->currentRow();
  cw.lwTBPings->takeItem( c );
}

void Catkin::doFont()
{
#if QT_VERSION < 0x040200
    bool isOK;
    QSettings settings;
    QFont f( QFontDialog::getFont( &isOK, EDITOR->font() ) );
    EDITOR->setFont( f );
    editorFontString = f.toString();
    settings.beginGroup( "fonts" );
    settings.setValue( "editorFontString", editorFontString );
    settings.endGroup();
#endif
}

void Catkin::doPreviewFont()
{
#if QT_VERSION < 0x040200
  bool isOK;
  QSettings settings;
  QFont f( QFontDialog::getFont( &isOK, previewWindow->font() ) );
  previewWindow->setFont( f );
  previewFontString = f.toString();
  settings.beginGroup( "fonts" );
  settings.setValue( "previewFontString", previewFontString );
  settings.endGroup();
#endif
}

void Catkin::doConsoleFont()
{
#if QT_VERSION < 0x040200
  bool isOK;
  QSettings settings;
  QFont f( QFontDialog::getFont( &isOK, console->font() ) );
  console->setFont( f );
  consoleFontString = f.toString();
  settings.beginGroup( "fonts" );
  settings.setValue( "consoleFontString", consoleFontString );
  settings.endGroup();
#endif
}

void Catkin::dirtify()
{
  dirtyIndicator->show();
  setWindowModified( true );
  disconnect( EDITOR, 0, dirtyIndicator, 0 );
  cleanSave = false;
}

void Catkin::setPostClean()
{
  dirtyIndicator->hide();
  setWindowModified( false );
  connect( EDITOR, SIGNAL( textChanged() ), this, SLOT( dirtify() ) );
  cleanSave = false;
}

void Catkin::showStatusBarMessage( const QString &msg )
{
  statusBar()->showMessage( msg, 2000 );
}

bool Catkin::saveCheck( bool )
{
  bool return_value;

  if( !isWindowModified() )
    return_value = true;
  else
    return_value = saveCheck();

  return return_value;
}

bool Catkin::saveCheck()
{
  bool return_value;

  activateWindow();
  int a = QMessageBox::warning( this, tr( "Not saved" ),
				tr( "You have not saved this file.\n"
				    "Save first?" ),
				QMessageBox::Yes | QMessageBox::Default,
				QMessageBox::No, QMessageBox::Cancel );
  switch( a ) {
  case QMessageBox::Cancel:
    return_value = false;
    break;
  case QMessageBox::Yes:
    save();
  default:
    return_value = true;
  }

  return return_value;
}

void Catkin::doInitialChangeBlog()
{
#ifdef QTM_DEBUG
  addToConsole( QString( "loadedEntryBlog: %1\n" ).arg( loadedEntryBlog ) );
#endif
  disconnect( this, SIGNAL( httpBusinessFinished() ), 0, 0 );
  if( loadedEntryBlog != 999 ) {
    currentBlog = loadedEntryBlog;
    loadedEntryBlog = 999;
    cw.cbBlogSelector->setCurrentIndex( currentBlog );
    currentBlogElement = currentAccountElement.firstChildElement( "blogs" )
      .elementsByTagName( "blog" ).at( currentBlog ).toElement();
    currentBlogid = currentBlogElement.firstChildElement( "blogid" ).text();
#ifndef NO_DEBUG_OUTPUT
    qDebug() << currentBlogid;
#endif

  }
  changeBlog( currentBlog );

  connect( cw.cbBlogSelector, SIGNAL( activated( int ) ),
	   this, SLOT( changeBlog( int ) ) );
}

void Catkin::copyURL()
{
  QApplication::clipboard()->setText( remoteFileLocation );
  ui.actionCopy_upload_location->setVisible( false );
  pbCopyURL->hide();
}

void Catkin::handleFind()
{
  if( searchWidget->isVisible() ) {
    ui.action_Find->setText( tr( "&Find ..." ) );
    searchWidget->close();
  }
  else {
    ui.action_Find->setText( tr( "Exit &find" ) );
    searchWidget->find();
  }
}

QDomElement Catkin::XmlValue( QDomDocument &doc, QString valueType,
			      QString actualValue, QString memberType )
{
  QDomElement param = doc.createElement( memberType );
  QDomElement value = doc.createElement( "value" );
  QDomElement realValue = doc.createElement( valueType );
  realValue.appendChild( QDomText( doc.createTextNode( actualValue ) ) );
  value.appendChild( realValue );
  param.appendChild( value );

  return param;
}

QDomElement Catkin::XmlMember( QDomDocument &doc, QString valueName,
			       QString valueType, QString actualValue )
{
  QDomElement mem = doc.createElement( "member" );
  QDomElement name = doc.createElement( "name" );
  name.appendChild( QDomText( doc.createTextNode( valueName ) ) );
  mem.appendChild( name );
  QDomElement value = doc.createElement( "value" );
  QDomElement realValue = doc.createElement( valueType );
  realValue.appendChild( QDomText( doc.createTextNode( actualValue ) ) );
  value.appendChild( realValue );
  mem.appendChild( value );

  return mem;
}

QDomElement Catkin::XmlMethodName( QDomDocument &doc, QString methodName )
{
  QDomElement methName = doc.createElement( "methodName" );
  methName.appendChild( QDomText( doc.createTextNode( methodName ) ) );

  return methName;
}

QDomElement Catkin::XmlRpcArray( QDomDocument &doc, QString valueName,
				 QList<QString> text )
{
  QDomElement arrayValue, arrayValueString;
  int i;

  QDomElement mem = doc.createElement( "member" );
  QDomElement name = doc.createElement( "name" );
  name.appendChild( QDomText( doc.createTextNode( valueName ) ) );
  mem.appendChild( name );
  QDomElement value = doc.createElement( "value" );
  QDomElement array = doc.createElement( "array" );
  QDomElement arrayData = doc.createElement( "data" );

  for( i = 0; i < text.count(); i++ ) {
    arrayValue = doc.createElement( "value" );
    arrayValueString = doc.createElement( "string" );
    arrayValueString.appendChild( QDomText( doc.createTextNode( text.value( i ) ) ) );
    arrayValue.appendChild( arrayValueString );
    arrayData.appendChild( arrayValue );
  }

  array.appendChild( arrayData );
  value.appendChild( array );
  mem.appendChild( value );

  return mem;
}

void Catkin::setNetworkActionsEnabled( bool a )
{
  ui.actionRefresh_blog_list->setEnabled( a );
  ui.actionRefresh_categories->setEnabled( categoriesEnabled ? a : false );
  ui.actionSend_categories->setEnabled( categoriesEnabled ? a : false );
  ui.action_Blog_this->setEnabled( a );
  cw.lBlog->setEnabled( a );
  cw.cbBlogSelector->setEnabled( a );
  cw.pbRefresh->setEnabled( a );
}

void Catkin::handleInitialLookup( const QHostInfo &hostInfo )
{
  //qDebug( "Tested for correct hostname\n" );
  if( hostInfo.error() == QHostInfo::NoError ) {
    refreshBlogList();
    setNetworkActionsEnabled( true );
  }
  else
    setNetworkActionsEnabled( false );
}

void Catkin::handleHostLookupFailed()
{
  statusBar()->showMessage( tr( "The host specified could not be found." ), 2000 );

  http->disconnect();
  http->abort();
  /* http->deleteLater();
  connect( http, SIGNAL( destroyed() ),
  this, SLOT( deleteHttp() ) ); */

  disconnect( SIGNAL( httpBusinessFinished() ) );
  if( QApplication::overrideCursor() != 0 )
    QApplication::restoreOverrideCursor();
}

void Catkin::configureQuickpostTemplates()
{
#if QT_VERSION >= 0x040200
#if defined USE_SYSTRAYICON
  if( sti )
    sti->configureQuickpostTemplates( this );
#endif
#endif
}

void Catkin::setPublishStatus( int publishStatus )
{
  cw.cbStatus->setCurrentIndex( publishStatus );
}

QString Catkin::postTitle()
{
  return cw.leTitle->text();
}

void Catkin::setPostTitle( const QString &t )
{
  cw.leTitle->setText( t );
}

void Catkin::saveAutoLinkDictionary()
{
  QDomDocument doc;
  QDomElement currentElement, currentKey, currentValue, currentTitle, currentTarget;
  QDomText titleText, targetText;

  QDomElement autoLinkDictionaryElement = doc.createElement( "AutoLinkDictionary" );
  QHashIterator<QString, QString> aldIterator( autoLinkDictionary );

  while( aldIterator.hasNext() ) {
    aldIterator.next();
    currentElement = doc.createElement( "entry" );
    currentKey = doc.createElement( "key" );
    currentKey.appendChild( QDomText( doc.createTextNode( aldIterator.key() ) ) );
    currentValue = doc.createElement( "URL" );
    currentValue.appendChild( QDomText( doc.createTextNode( aldIterator.value() ) ) );
    currentElement.appendChild( currentKey );
    currentElement.appendChild( currentValue );
    if( autoLinkTitleDictionary.contains( aldIterator.key() ) ) {
      currentTitle = doc.createElement( "title" );
      titleText = doc.createTextNode( autoLinkTitleDictionary.value( aldIterator.key() ) );
      currentTitle.appendChild( titleText );
      currentElement.appendChild( currentTitle );
    }
    if( autoLinkTargetDictionary.contains( aldIterator.key() ) ) {
      currentTarget = doc.createElement( "target" );
      targetText = doc.createTextNode( QString::number( autoLinkTargetDictionary
							.value( aldIterator.key() ) ) );
      currentTarget.appendChild( targetText );
      currentElement.appendChild( currentTarget );
    }
    autoLinkDictionaryElement.appendChild( currentElement );
  }

  doc.appendChild( autoLinkDictionaryElement );
  doc.insertBefore( doc.createProcessingInstruction( "xml",
						     "version=\"1.0\" encoding=\"UTF-8\"" ),
		    doc.firstChild() );

  QSettings settings;
  settings.beginGroup( "account" );
  QString dictionaryFileName = QString( "%1/qtmautolinkdict.xml" )
    .arg( settings.value( "localStorageDirectory", "" ).toString() );
  settings.endGroup();
  QFile *dictionaryFile = new QFile( dictionaryFileName );
  if( dictionaryFile->open( QIODevice::WriteOnly ) ) {
    QTextStream dictionaryFileStream( dictionaryFile );
    doc.save( dictionaryFileStream, 4 );
    dictionaryFile->close();
  }
  else
    statusBar()->showMessage(  tr( "Could not write to dictionary file" ), 2000 );
  dictionaryFile->deleteLater();

}

void Catkin::loadAutoLinkDictionary()
{
  QString dictionaryFileName, errorString, currentKey, currentURL;
  int currentTarget;
  QDomElement currentKeyElement, currentURLElement, currentTitleElement, currentTargetElement;
  QSettings settings;
  int errorLine, errorCol;
  bool ok;

  settings.beginGroup( "account" );
  dictionaryFileName = QString( "%1/qtmautolinkdict.xml" ).
    arg( settings.value( "localStorageDirectory", "" ).toString() );
  settings.endGroup();

  if( QFile::exists( dictionaryFileName ) ) {
    QDomDocument dictDoc( "autoLinkDictionary" );
    QFile file( dictionaryFileName );
    if( !dictDoc.setContent( &file, true, &errorString, &errorLine, &errorCol ) )
      QMessageBox::warning( 0, tr( "Failed to read templates" ),
			    QString( tr( "Error: %1\n"
					 "Line %2, col %3" ) )
			    .arg( errorString ).arg( errorLine )
			    .arg( errorCol ) );
    else {
      QDomNodeList entryList = dictDoc.elementsByTagName( "entry" );
      for( int i = 0; i < entryList.count(); i++ ) {
	currentKeyElement = entryList.at( i ).firstChildElement( "key" );
	currentURLElement = entryList.at( i ).firstChildElement( "URL" );
	currentTitleElement = entryList.at( i ).firstChildElement( "title" );
	currentTargetElement = entryList.at( i ).firstChildElement( "target" );
	if( currentKeyElement.isNull() || currentURLElement.isNull() )
	  continue;
	else {
	  currentKey = currentKeyElement.text();
	  currentURL = currentURLElement.text();
	  autoLinkDictionary.insert( currentKey, currentURL );
	  if( !currentTitleElement.isNull() )
	    autoLinkTitleDictionary.insert( currentKey, currentTitleElement.text() );
	  if( !currentTargetElement.isNull() ) {
	    currentTarget = currentTargetElement.text().toInt( &ok );
	    if( ok )
	      autoLinkTargetDictionary.insert( currentKey, currentTarget );
	  }
	}
      }
    }
  }
}

// This method was adapted from the QByteArray source code in Qt v4.3.1

QByteArray Catkin::toBase64( QByteArray &qbarray )
{
  QProgressDialog pdialog( tr( "Converting file to Base64" ),
			   tr( "Cancel" ),
			   0, qbarray.size(), 0 );
  pdialog.setWindowModality( Qt::WindowModal );
  pdialog.setMinimumDuration( 2000 );
  pdialog.setWindowTitle( "QTM" );

  const char alphabet[] = "ABCDEFGH" "IJKLMNOP" "QRSTUVWX" "YZabcdef"
    "ghijklmn" "opqrstuv" "wxyz0123" "456789+/";
  const char padchar = '=';
  int padlen = 0;

  char *data = qbarray.data();
  int size = qbarray.size();

  QByteArray tmp;
  tmp.resize(((size * 4) / 3) + 3);

  int i = 0;
  char *out = tmp.data();
  while (i < size) {
    int chunk = 0;
    chunk |= int(uchar(data[i++])) << 16;
    if (i == size) {
      padlen = 2;
    } else {
      chunk |= int(uchar(data[i++])) << 8;
      if (i == size) padlen = 1;
      else chunk |= int(uchar(data[i++]));
    }

    int j = (chunk & 0x00fc0000) >> 18;
    int k = (chunk & 0x0003f000) >> 12;
    int l = (chunk & 0x00000fc0) >> 6;
    int m = (chunk & 0x0000003f);
    *out++ = alphabet[j];
    *out++ = alphabet[k];
    if (padlen > 1) *out++ = padchar;
    else *out++ = alphabet[l];
    if (padlen > 0) *out++ = padchar;
    else *out++ = alphabet[m];

    if( pdialog.wasCanceled() )
      break;
    if( i % 1024 == 0 ) {
      pdialog.setValue( i );
      qApp->processEvents();
    }
  }

  tmp.truncate(out - tmp.data());
  if( pdialog.wasCanceled() )
    return QByteArray();
  else
    return tmp;
}

void Catkin::addToConsole( const QString &t )
{
  QString text = t;
#ifndef NO_DEBUG_OUTPUT
  if( currentHttpBusiness == 11 )
    qDebug() << "Adding image to console";
#endif
  text.replace( QString( "<string>%1</string>" ).arg( password ),
		tr( "<string>(password omitted)</string>" ) );
  QTextCursor cursor( console->textCursor() );

  cursor.movePosition( QTextCursor::End );
  console->setTextCursor( cursor );
  if( text.contains( "<base64>" ) )
    console->insertPlainText( text.section( "<base64>", 0, 0, QString::SectionIncludeTrailingSep )
			      .append( tr( " ... base64 encoded file omitted ... " ) )
			      .append( text.section( "</base64>", 1, 1, QString::SectionIncludeLeadingSep ) ) );
  else
    console->insertPlainText( text );

#ifndef NO_DEBUG_OUTPUT
  if( currentHttpBusiness == 11 )
    qDebug() << "Added image to console";
#endif
}

