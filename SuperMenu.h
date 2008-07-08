/***************************************************************************
 * SuperMenu.h - Default menu for QTM on Mac OS X
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


#include <QMenuBar>
class QAction;
class QMenu;
class Catkin;
class SysTrayIcon;

class SuperMenu : public QMenuBar
{
Q_OBJECT

public:
  SuperMenu( QWidget *parent = 0, SysTrayIcon *sti = 0 );

private:
  QMenu *fileMenu;
  QAction *newEntryAction;
  QAction *openAction;
  QAction *quitAction;
  QAction *aboutAction;
  SysTrayIcon *_sti;

private slots:
  void newEntry();
  void choose();
  void quit();
  void about();
};
