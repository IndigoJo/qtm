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

  setupUi( this );

  // Set up internal account lists; one for the current contents of the accounts,
  // one for backup
  accountList = acctList;
  originalAccountList = acctList;

  /*  teTemplateText->setWhatsThis( tr( "<p>Enter the text of your template here, in HTML "
				    "format.</p>"
				    "<p>Four place-markers are supported:</p>"
				    "<p><strong>%title%</strong> - The page's title<br />"
				    "<strong>%url%</strong> - the full web location (URL) of "
				    "the page being blogged;<br />"
				    "<strong>%host%</strong> - the page's host name (after http://), and<br />"
				    "<strong>%domain%</strong> - the page's hostname without the \"www\".</p>" ) );
  lTemplate->setWhatsThis( tr( "<p>Enter the text of your template here, in HTML "
			       "format.</p>"
			       "<p>Four place-markers are supported:</p>"
			       "<p><strong>%title%</strong> - The page's title<br />"
			       "<strong>%url%</strong> - the full web location (URL) of "
			       "the page being blogged;<br />"
			       "<strong>%host%</strong> - the page's host name (after http://), and<br />"
			       "<strong>%domain%</strong> - the page's hostname without the \"www\".</p>" ) );
  cbDefaultPublishStatus->setWhatsThis( tr( "<p>Set this to Draft or Publish if you always want "
					    "posts based on this template to have that status.</p>"
					    "<p>Otherwise, the post will have the status you specify in "
					    "the Preferences window.</p>" ) );
  lDefaultPublishStatus->setWhatsThis( tr( "<p>Set this to Draft or Publish if you always want "
					   "posts based on this template to have that status.</p>"
					   "<p>Otherwise, the post will have the status you specify in "
					   "the Preferences window.</p>" ) );
  */


  // lowestNumber = lwTemplates->count();

  hboxLayout->setStretchFactor( hboxLayout->itemAt( 0 )->layout(), 2 );
  hboxLayout->setStretchFactor( hboxLayout->itemAt( 1 )->layout(), 3 );

  /*  //QHBoxLayout *bl = qobject_cast<QHBoxLayout *>(lwTemplates->layout()->parent() );
  QHBoxLayout *bl = (QHBoxLayout *)(lwTemplates->layout()->parent());
  if( bl ) {
    bl->setStretchFactor( lwTemplates->layout(), 2 );
    bl->setStretchFactor( leName->layout(), 3 );
    }*/
  /*  lwTemplates->layout()->parent()->setStretchFactor(
      lwTemplates->layout()->parent()->setColumnStretch( 1, 3 );*/

  for( int i = 0; i < accountList.count()l i++ ) {
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
  /*  leName->setEnabled( false );
  cbHostedBlogType->setEnabled( false );
  leServer->setEnabled( false );
  leLocation->setEnabled( false );
  lePort->setEnabled( false );
  leLogin->setEnabled( false );
  lePassword->setEnabled( false );*/

  doingNewAccount = false;
  // setClean();
  currentRow = -1;
  defaultCPStatus = status;

  addAccount = new QAction( tr( "&Add account" ), lwAccountList );
  removeAccount = new QAction( tr( "&Remove this account" ), lwAccountList );
  lwAccountList->addAction( addAccount );
  lwAccountList->addAction( removeAccount );
  lwAccountList->setContextMenuPolicy( Qt::ActionsContextMenu );

  /*  insertTabIntoTemplate = new QAction( teTemplateText );
  insertTabIntoTemplate->setShortcut( tr( "Ctrl+T" ) );
  insertTabIntoTemplate->setShortcutContext( Qt::WidgetShortcut );
  teTemplateText->addAction( insertTabIntoTemplate ); */

  connect( addAccount, SIGNAL( triggered( bool ) ),
	   this, SLOT( doNewAccount() ) );
  connect( removeAccount, SIGNAL( triggered( bool ) ),
	   this, SLOT( removeThisAccount() ) );
  connect( lwAccountList, SIGNAL( currentRowChanged( int ) ),
    this, SLOT( changeListIndex( int ) ) );

  entryDateTime = QDateTime();
}

/*void AccountsDialog::on_lwTemplates_clicked( const QModelIndex &ind )
{
  qDebug( "lwTemplates clicked" );

  int row = ind.row();

  if( row != currentRow ) {
    if( dirty ) {
      qDebug( "status is dirty" );
      int ret = QMessageBox::warning( 0, tr("QTM"),
				      tr("The template has been modified.\n"
					 "Do you want to save your changes?"),
				      tr( "&Save" ), tr( "&Discard" ),
				      tr( "&Cancel" ), 0, 2 );

      switch( ret ) {
      case 2:
	return; // Click will have no effect
      case 0:
	acceptTemplate();
      default:
	lwTemplates->setCurrentRow( row );
	changeListIndex( row );
      }
    }
    else {
      lwTemplates->setCurrentRow( row );
      changeListIndex( row );
    }
  }
  else
    qDebug( "row is current row" );
    }*/

void AccountsDialog::changeListIndex( int index )
{
  if( doingNewAccount ) {
    if( dirty )
      lwAccountList->item( (lwAccountList->count())-1 )->setText( leName->text().isEmpty() ?
								tr( "(No name)" ) :
								leName->text() );
    else {
      /*      _templateTitles.removeAt( currentRow );
      _templateStrings.removeAt( currentRow );
      _defaultPublishStates.removeAt( currentRow );
      _assocHostStrings.removeAt( currentRow );
      _copyTitleStates.removeAt( currentRow ); */
      accountList.removeAt( currentRow );
      lwAccountList->takeItem( lwAccountList->count()-1 );
    }
  }

  doingNewAccount = false;
  currentRow = index;

  /*  currentTemplateTitle = _templateTitles[currentRow];
      currentTemplateString = _templateStrings[currentRow]; */

  leName->setText( accountList[currentRow].name );
  leServer->setText( accountList[currentRow].server );
  leLocation->setText( accountList[currentRow].location );
  lePort->setText( accountList[currentRow].port );
  leLogin->setText( accountList[currentRow].login );
  lePassword->setText( accountList[currentRow].password );

  Q_FOREACH( QWidget *w, accountWidgets )
    w->setEnabled( true );
  // setClean();
}

void AccountsDialog::doNewAccount()
{
  entryDateTime = QDateTime::currentDateTime();
  Q_FOREACH( QWidget *w, accountWidgets )
    w->setEnabled( true );

  /*  _templateTitles.append( QString() );
  _templateStrings.append( QString() );
  _defaultPublishStates.append( int( 2 ) );
  _assocHostStrings.append( QString() );
  _copyTitleStates.append( bool( defaultCPStatus ) ); */
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
    /*    _templateTitles.clear();
    _templateStrings.clear();
    _defaultPublishStates.clear();
    _copyTitleStates.clear();
    _assocHostStrings.clear(); */
    foreach( QWidget *w, accountWidgets ) {
      if( qobject_cast<QLineEdit *>( w ) )
	w->clear();
      w->setEnabled( false );
    }
    /*    leName->clear();
    leName->setEnabled( false );
    teTemplateText->clear();
    teTemplateText->setEnabled( false );
    cbDefaultPublishStatus->addItem( "" );
    cbDefaultPublishStatus->setCurrentIndex( 3 );
    cbDefaultPublishStatus->setEnabled( false );
    teAssocHosts->clear();
    teAssocHosts->setEnabled( false );
    chCopyTitle->setEnabled( false ); */
    // pbAccept->setEnabled( false );
    currentRow = -1;
  } else {
    lwAccountList->takeItem( c );
    accountList.removeAt( c );
  }
  // setClean();
  // qDebug( "exiting" );
  /*if( originalTitles.size() < lowestNumber )
    lowestNumber = titles.size(); */
}

void AccountsDialog::setDirty()
{
  dirty = true;
  Q_FOREACH( QWidget *w, accountWidgets )
    disconnect( w, 0, this, SLOT( setDirty() ) );

  /*  disconnect( leName, 0, this, SLOT( setDirty() ) );
  disconnect( teTemplateText, 0, this, SLOT( setDirty() ) );
  disconnect( teAssocHosts, 0, this, SLOT( setDirty() ) );
  disconnect( cbDefaultPublishStatus, 0, this, SLOT( setDirty() ) );
  disconnect( chCopyTitle, 0, this, SLOT( setDirty() ) ); */
  // disconnect( this, SLOT( setDirty() ) );
}

void AccountsDialog::setClean()
{
  // qDebug( "clean" );
  dirty = false;

  // disconnect( this, SLOT( setDirty() ) ); // to prevent duplicates
  Q_FOREACH( QWidget *w, accountWidgets )
    disconnect( w, 0, this, SLOT( setDirty() ) );

  /*  disconnect( leName, 0, this, SLOT( setDirty() ) );
  disconnect( teTemplateText, 0, this, SLOT( setDirty() ) );
  disconnect( teAssocHosts, 0, this, SLOT( setDirty() ) );
  disconnect( cbDefaultPublishStatus, 0, this, SLOT( setDirty() ) );
  disconnect( chCopyTitle, 0, this, SLOT( setDirty() ) );
  */

  Q_FOREACH( QWidget *v, accountWidgets ) {
    if( qobject_cast<QLineEdit *>( v ) )
      connect( v, SIGNAL( textChanged( const QString & ) ),
	       this, SLOT( setDirty() ) );
  }
  /*  connect( leName, SIGNAL( textChanged( const QString & ) ),
	   this, SLOT( setDirty() ) );
  connect( teTemplateText, SIGNAL( textChanged() ),
	   this, SLOT( setDirty() ) );
  connect( teAssocHosts, SIGNAL( textChanged() ),
	   this, SLOT( setDirty() ) );
  connect( cbDefaultPublishStatus, SIGNAL( currentIndexChanged( int ) ),
	   this, SLOT( setDirty() ) );
  connect( chCopyTitle, SIGNAL( clicked( bool ) ),
  this, SLOT( setDirty() ) ); */
}

void AccountsDialog::acceptAccount()
{
  Account newAcct;
  if( doingNewAccount ) {
    accountList.append( currentAcct );
    /*
    _templateTitles.append( leName->text() );
    _templateStrings.append( teTemplateText->toPlainText() );
    _defaultPublishStates.append( cbDefaultPublishStatus->currentIndex() );
    _copyTitleStates.append( chCopyTitle->isChecked() );
    _assocHostStrings.append( teAssocHosts->toPlainText().trimmed() );
    lwTemplates->item( lwTemplates->count()-1 )->setText( leName->text().isEmpty() ?
							  tr( "(No name)" ) :
							  leName->text() );*/
    if( lwAccountList->count() == 1 )
      connect( lwAccountList, SIGNAL( currentRowChanged( int ) ),
	       this, SLOT( changeListIndex( int ) ) );

    /*   QListWidgetItem *newItem = new QListWidgetItem( leName->text() );
    lwTemplates->addItem( newItem );
    lwTemplates->setCurrentItem( newItem );*/

  } else {
    if( lwAccountList->count() ) {
      lwAccountList->item( currentRow )->setText( leName->text().isEmpty() ?
						  tr( "(No name)" ) : leName->text() );
      /*      _templateTitles[currentRow] = leName->text();
      _templateStrings[currentRow] = teTemplateText->toPlainText();
      _defaultPublishStates[currentRow] = cbDefaultPublishStatus->currentIndex();
      _copyTitleStates[currentRow] = chCopyTitle->isChecked();
      _assocHostLists[currentRow] = teAssocHosts->toPlainText().trimmed().split( QRegExp( "\\n+" ) ); */
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

  // We need to get rid of any empty or whitespace-only associated hosts entries
  /*  QRegExp emptyString( "^\\s+$" );
  int i, j;
  _assocHostLists = compileAssocHostLists( _assocHostStrings );
  if( _assocHostLists.count() ) {
    for( i = 0; i < _assocHostLists.count(); i++ ) {
      for( j = 0; j < _assocHostLists[i].count(); j++ ) {
	if( _assocHostLists[i].at( j ).isEmpty() ||
	    emptyString.exactMatch( _assocHostLists[i].at( j ) ) )
	  _assocHostLists[i].removeAt( j );
      }
    }
  }
  */
  accept();
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