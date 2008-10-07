/***************************************************************************
 * SuperMenu.cc - Default menu for QTM on Mac OS X
 *
 * Copyright (C) 2007, Matthew J Smith
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


#include <QMenu>
#include <QAction>
#include <QMenuBar>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QMessageBox>
#include <QFileDialog>
#include <QKeySequence>
#include <QDialog>

#include "EditingWindow.h"
#include "SuperMenu.h"
#include "SysTrayIcon.h"
#include "ui_aboutbox.h"

SuperMenu::SuperMenu( QWidget *parent, SysTrayIcon *sti )
  : QMenuBar( parent )
{
  fileMenu = addMenu( tr( "&File" ) );
  newEntryAction = fileMenu->addAction( tr( "&New entry" ), this, SLOT( newEntry() ) );
  newEntryAction->setShortcut( QKeySequence( tr( "Ctrl+N" ) ) );
  openAction = fileMenu->addAction( tr( "&Open ..." ), this, SLOT( choose() ) );
  openAction->setShortcut( QKeySequence( tr( "Ctrl+O" ) ) );
  quitAction = fileMenu->addAction( tr( "&Quit" ), this, SLOT( quit() ) );
  quitAction->setMenuRole( QAction::QuitRole );
  aboutAction = fileMenu->addAction( tr( "About QTM" ), this, SLOT( about() ) );
  _sti = sti;
}

void SuperMenu::newEntry()
{
  EditingWindow *c = new EditingWindow;
  c->setSTI( _sti );
  c->setWindowTitle( QObject::tr( "QTM - new entry [*]" ) );
  c->show();
#if defined QT_VERSION >= 0x040300
  QApplication::alert( c );
#endif
}

void SuperMenu::choose()
{
  QSettings settings;
  QString localStorageFileExtn, localStorageDirectory;
  QStringList filesSelected;

  settings.beginGroup( "account" );
  localStorageFileExtn = settings.value( "localStorageFileExtn", "cqt" ).toString();
  localStorageDirectory = settings.value( "localStorageDirectory", "" ).toString();
  settings.endGroup();

  QString extn( QString( "%1 (*.%2)" ).arg( tr( "Blog entries" ) )
		.arg( localStorageFileExtn ) );
  
  QString fn =
    QFileDialog::getOpenFileName( 0, tr( "Choose a file to open" ),
    localStorageDirectory, extn );

  if( !fn.isEmpty() ) {
    EditingWindow *e = new EditingWindow( true );
    if( !e->load( fn, true ) ) {
      QMessageBox::warning( 0, "QTM",
			    tr( "Could not load the file you specified." ),
			    QMessageBox::Cancel, QMessageBox::NoButton );
      e->deleteLater();
    } else {
      e->setSTI( _sti );
      e->show();
      e->activateWindow();
#if defined QT_VERSION >= 0x040300
      QApplication::alert( e );
#endif
    }
  }
}


void SuperMenu::quit()
{
  QCoreApplication::quit();
}

void SuperMenu::about()
{
  QDialog about_box( 0 );
  Ui::AboutBox abui;

  abui.setupUi( &about_box );
  about_box.exec();
}
