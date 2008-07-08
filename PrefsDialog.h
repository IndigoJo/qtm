/***************************************************************************
 * PrefsDialog.h - Header file for QTM's preferences dialog
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


#include <QtGlobal>

class QDialog;
class QWidget;
class QHostInfo;

#if QT_VERSION >= 0x040200
#include "ui_PrefsForm.h"
#else
#include "ui_PrefsForm41.h"
#endif

class PrefsDialog : public QDialog, public Ui::PrefsForm
{
Q_OBJECT

public:
  PrefsDialog( QWidget *parent = 0 );
 bool noValidHost();

private slots:
  void on_pbBrowse_clicked();
  void on_pbWhatsThis_clicked();
  void on_okButton_clicked();
  void on_cbBlogType_currentIndexChanged( int );
  void handleHostInfo( const QHostInfo & );

 private:
  bool nvh;
};
