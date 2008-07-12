/************************************************************************************************
 *
 * AccountsDialog.cc - Multi-account form for QTM
 *
 * Copyright (C) 2007, 2008, Matthew J Smith
 *
 * This file is part of QTM.
 * QTM program is free software; you can redistribute it and/or modify
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
 *************************************************************************************************/


#include <QWidget>
#include <QDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QComboBox>
#include <QString>
#include <QStringList>
#include <QList>
#include <QWhatsThis>
#include <QMessageBox>
#include <QModelIndex>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateTime>
#if QT_VERSION <= 0x040300
#include <QTextDocument>
#endif

#include "AccountsDialog.h"
#include "ui_AccountsForm.h"

AccountsDialog::AccountsDialog( QList<Account> &acctList, QWidget *parent )
  : QDialog( parent )
{
  QString *a;
  networkBiz = noBusiness;

  setupUi( this );

  // Set up internal account lists; one for the current contents of the accounts,
  // one for backup
  accountList = acctList;
  originalAccountList = acctList;

  hboxLayout->setStretchFactor( hboxLayout->itemAt( 0 )->layout(), 2 );
  hboxLayout->setStretchFactor( hboxLayout->itemAt( 1 )->layout(), 3 );



  for( int i = 0; i < accountList.count(); i++ ) {
    a = &(accountList.at( i ).name);
    if( a->isEmpty() )
      lwAccountList->addItem( tr( "(No name)" ) );
    else
      lwAccountList->addItem( *a );
  }
  //qDebug( "added the templates" );

  // Set up list of account widgets;
  accountWidgets << leName << cbHostedBlogTypes << leServer << leLocation
		 << lePort << leLogin << lePassword
		 << chCategories << chPostDateTime << chComments << chTB;

  pbReset->setVisible( false );

  Q_FOREACH( QWidget *w, accountWidgets )
    w->setEnabled( false );

  doingNewAccount = false;
  // setClean();
  currentRow = -1;
  defaultCPStatus = status;

  addAccount = new QAction( tr( "&Add account" ), lwAccountList );
  removeAccount = new QAction( tr( "&Remove this account" ), lwAccountList );
  lwAccountList->addAction( addAccount );
  lwAccountList->addAction( removeAccount );
  lwAccountList->setContextMenuPolicy( Qt::ActionsContextMenu );

  connect( addAccount, SIGNAL( triggered( bool ) ),
	   this, SLOT( doNewAccount() ) );
  connect( removeAccount, SIGNAL( triggered( bool ) ),
	   this, SLOT( removeThisAccount() ) );
  connect( lwAccountList, SIGNAL( currentRowChanged( int ) ),
    this, SLOT( changeListIndex( int ) ) );

  entryDateTime = QDateTime();
}

void AccountsDialog::changeListIndex( int index )
{
  if( doingNewAccount ) {
    if( dirty )
      lwAccountList->item( (lwAccountList->count())-1 )->setText( leName->text().isEmpty() ?
								tr( "(No name)" ) :
								leName->text() );
    else {
      accountList.removeAt( currentRow );
      lwAccountList->takeItem( lwAccountList->count()-1 );
    }
  }

  doingNewAccount = false;
  currentRow = index;

  leName->setText( accountList[currentRow].name );
  leServer->setText( accountList[currentRow].server );
  leLocation->setText( accountList[currentRow].location );
  lePort->setText( accountList[currentRow].port );
  leLogin->setText( accountList[currentRow].login );
  lePassword->setText( accountList[currentRow].password );

  chCategoriesEnabled->setCheckState( accountList[currentRow].categoriesEnabled ?
				      Qt::Checked : Qt::Unchecked );
  chPostDateTime->setCheckState( accountList[currentRow].postDateTime ?
				 Qt::Checked : Qt::Unchecked );
  chComments->setCheckState( accountList[currentRow].comments ? Qt::Checked :
			     Qt::Unchecked );
  chTB->setCheckState( accountList[currentRow].trackback ? Qt::Checked : Qt::Unchecked );

  Q_FOREACH( QWidget *w, accountWidgets )
    w->setEnabled( true );
}

void AccountsDialog::doNewAccount()
{
  entryDateTime = QDateTime::currentDateTime();
  Q_FOREACH( QWidget *w, accountWidgets )
    w->setEnabled( true );

  accountList.append( Account() );

  lwAccountList->disconnect( SIGNAL( currentRowChanged( int ) ),
			     this, SLOT( changeListIndex( int ) ) );
  currentRow = -1;
  Q_FOREACH( QWidget *v, accountWidgets )
    if( qobject_cast<QLineEdit *>( w ) )
      w->clear();

  lwAccountList->addItem( tr( "New account" ) );
  currentRow = lwAccountList->count()-1;
  lwAccountList->setCurrentRow( lwAccountList->count()-1 );
  connect( lwAccountList, SIGNAL( currentRowChanged( int ) ),
	   this, SLOT( changeListIndex( int ) ) );

  doingNewAccount = true;
  leName->setFocus( Qt::OtherFocusReason );
  connect( qApp, SIGNAL( focusChanged( QWidget *, QWidget * ) ),
           this, SLOT( assignSlug() ) );

  setClean();
}

void AccountsDialog::removeThisAccount()
{
  int c = lwAccountList->currentRow();
  // lwTemplates->takeItem( c );

  if( lwAccountList->count() == 1 ) {
    lwAccountList->disconnect( SIGNAL( currentRowChanged( int ) ) );
    lwAccountList->clear();
    accountList.clear();

    if( qobject_cast<QLineEdit *>( w ) )
      w->clear();
      w->setEnabled( false );
    }
    currentRow = -1;
  } else {
    lwAccountList->takeItem( c );
    accountList.removeAt( c );
  }
}

void AccountsDialog::setDirty()
{
  dirty = true;
  Q_FOREACH( QWidget *w, accountWidgets )
    disconnect( w, 0, this, SLOT( setDirty() ) );
}

void AccountsDialog::setClean()
{
  // qDebug( "clean" );
  dirty = false;

  // disconnect( this, SLOT( setDirty() ) ); // to prevent duplicates
  Q_FOREACH( QWidget *w, accountWidgets )
    disconnect( w, 0, this, SLOT( setDirty() ) );

  Q_FOREACH( QWidget *v, accountWidgets ) {
    if( qobject_cast<QLineEdit *>( v ) )
      connect( v, SIGNAL( textChanged( const QString & ) ),
	       this, SLOT( setDirty() ) );
  }
}

void AccountsDialog::acceptAccount()
{
  Account newAcct;
  if( doingNewAccount ) {
    accountList.append( currentAcct );

    if( lwAccountList->count() == 1 )
      connect( lwAccountList, SIGNAL( currentRowChanged( int ) ),
	       this, SLOT( changeListIndex( int ) ) );
  } else {
    if( lwAccountList->count() ) {
      lwAccountList->item( currentRow )->setText( leName->text().isEmpty() ?
						  tr( "(No name)" ) : leName->text() );
    }
  }

  doingNewAccount = false;
  setClean();
  // qDebug( "Finished accept routine" );
}

/* Private slot method to assign a unique slug name to identify an account.
 * It's done by the second, which should ensure uniqueness as long as identical
 * accounts aren't created by a program.
 */
void AccountsDialog::assignSlug()
{
  currentAccountId = QString( "%1_%2" ).arg( entryDateTime )
                                       .arg( leName->text().toLower()
                                             .replace( QRegExp( "\\s" ), "_" ) );
  accountList[currentRow].id = currentAccountId;
}

void AccountsDialog::on_pbWhatsThis_clicked()
{
  QWhatsThis::enterWhatsThisMode();
}

void AccountsDialog::on_pbOK_clicked()
{
  //qDebug( "Accept clicked" );
  //on_pbAccept_clicked();

  if( doingNewAccount && !dirty )
    accountList.remove( currentRow );

  accept();
}

void AccountsDialog::on_leBlogURL_returnPressed()
{
  int i;

  QUrl uri = QUrl( leBlogURI->text(), QUrl::TolerantMode );
  QString uris = uri.toString();

  if( !uri.isValid() ) {
    QMessageBox::information( 0, tr( "QTM: URI not valid" ),
			      tr( "That web location is not valid." ),
			      QMessageBox::Cancel );
    return;
  }

  bool found = false;
  QStringList hostedAccountStrings, hostedAccountServers, hostedAccountLocations;
  wpmuHosts << "wordpress.com" << "blogsome.com" << "blogs.ie"
	    << "hadithuna.com" << "edublogs.com" << "weblogs.us"
	    << "blogvis.com" << "blogates.com";
  /*  hostedAccountEndpoints << "@host@;xmlrpc.php"
			 << "www.typepad.com;/t/api"
			 << "www.squarespace.com;/do/process/external/PostInterceptor"
			 << "@host@;xmlrpc.php"
			 << "@host@;xmlrpc.php"
			 << "@host@;xmlrpc.php"; */

  // First check for TypePad and SquareSpace
  if( uris.contains( "squarespace.com" ) ) {
    leServer->setText( "www.squarespace.com" );
    leLocation->setText( "/do/process/external/PostInterceptor" );
    return;
  }

  if( uris.contains( "typepad.com" ) ) {
    leServer->setText( "www.typepad.com" );
    leLocation->setText( "/t/api" );
    return;
  }

  // Now check if it's a Wordpress MU host
  for( i = 0; i <= wpmuHosts.count(); i++ ) {
    if( i < wpmuHosts.at( i ) ) {
      if( uris.contains( wpmuHosts.at( i ) ) ) {
	leLocation->setText( uris );
	leServer->setText( "/xmlrpc.php" );
	return;
      }
    }
  }

  // Now test for a rsd.xml file
  http->setHost( uris.host() );
  QString loc( uris.path() );
  QRegExp re( "/.*\\.[shtml|dhtml|phtml|html|htm|php|cgi|pl|py]$" );
  if( re.exactMatch( loc ) )
    http->get( loc.section( "/", -2, 0 ) );
  else
    http->get( loc );

  connect( http, SIGNAL( requestFinished( int, bool ) ),
	   this, SLOT( handleResponseHeader() ) );
  connect( http, SIGNAL( done( bool ) ),
	   this, SLOT( handleHttpDone( bool ) ) );

}

void AccountsDialog::requestFinished( int /* id */,
				      bool error )
{
  if( !error )
    httpByteArray = http->readAll();
}

void AccountsDialog::handleHttpDone( bool error )
{
  QDomDocument rsdXml;
  QDomNodeList attributes;
  QDomElement thisApi;
  int i;
  QUrl url;

  if( !error ) {
    switch( networkBiz ) {
    case FindingRsdXml:
      rsdXml = QDomDocument( QString( httpByteArray ) );
      httpByteArray = QByteArray();
      if( rsdXml.documentElement().tagName == "rsd" ) {
	attributes = rsdXml.documentElement().firstChildElement( "apis" )
	  .elementsByTagName( "api" );
	for( i = 0; i < attributes.count(); i++ ) {
	  if( attributes.at( i ).attribute( "name" ) == "MetaWeblog" ) {
	    url = QUrl( attributes.at( i ).attribute( "apiLink" ) );
	    if( url.isValid() ) {
	      leServer->setText( url.host() );
	      leLocation->setText( url.path() );
	      break;
	    }
	  }
	}
      }
      break;
    case FindingXmlrpcPhp:
      // It only needs to exist; it's only likely to return an error message
      leServer->setText( currentHost );
      currentHost = QString();
      leLocation->setText( "/xmlrpc.php" );
      break;

    }
  }
  //  else {
  networkBiz = NoBusiness;
}

void AccountsDialog::on_leName_textEdited( const QString &newName )
{
  if( currentRow != -1 ) {
    // qDebug( "title edited" );
    lwAccountList->item( currentRow )->setText( newName.isEmpty() ?
						tr( "(No name)" ) : newName );
    accountList[currentRow].name = newName;
  }
}

void AccountsDialog::on_leServer_textEdited( const QSting &newServ )
{
  if( currentRow != -1 )
    accountList[currentRow].server = newServ;
}

void AccountsDialog::on_leLocation_textEdited( const QSting &text )
{
  if( currentRow != -1 )
    accountList[currentRow].location = text;
}

void AccountsDialog::on_lePort_textEdited( const QString &text )
{
  bool ok;

  if( currentRow != -1 )
    accountList[currentRow].port = text;
}

void AccountsDialog::on_leLogin_textEdited( const QString &text )
{
  if( currentRow != -1 )
    accountList[currentRow].login = text;
}

void AccountsDialog::on_lePassword_textEdited( const QString &text )
{
  if( currentRow != -1 )
    accountList[currentRow].password = text;
}

void AccountsDialog::on_chCategories_toggled( bool state )
{
  if( currentRow != -1 )
    accountList[currentRow].categoriesEnabled = state;
}

void AccountsDialog::on_chPostDateTime_toggled( bool state )
{
  if( currentRow != -1 )
    accountList[currentRow].postDateTime = state;
}

void AccountsDialog::on_chComments_toggled( bool state )
{
  if( currentRow != 1 )
    accountList[currentRow].comments = state;
}

void AccountsDialog::on_chTB_toggled( bool state )
{
  if( currentRow != 1 )
    accountList[currentRow].trackback = state;
}

