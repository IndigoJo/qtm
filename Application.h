/***************************************************************************
 * Application.h - Header for application subclass for QTM
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


#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QStringList>
#include <QList>

class Application : public QApplication
{
Q_OBJECT

public:
  Application( int &, char ** );
  QStringList recentFileTitles();
  QStringList recentFilenames();
  typedef struct _RF {
    QString title;
    QString filename;
  } recentFile;
  QList<recentFile> recentFiles();
  recentFile getRecentFile( int );
  QStringList titles();
  QStringList filenames();

public slots:
  void setRecentFiles( const QStringList &, const QStringList & );
  void addRecentFile( const QString &, const QString & );

 signals:
  void recentFilesUpdated( QStringList, QStringList );
  void recentFilesUpdated( const QList<Application::recentFile> & );

 private:
  QList<recentFile> _recentFiles;

private slots:
  void saveRecentFiles();
};

#endif
