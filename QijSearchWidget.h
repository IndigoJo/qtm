/*******************************************************************************
 *
 * Foxhound Search Widget - A Qt document search widget based on the
 * Firefox search widget
 *
 * Written by Matthew J Smith (Yusuf), 12th Feb 2008
 *
 *******************************************************************************/

#ifndef QIJSEARCHWIDGET_H
#define QIJSEARCHWIDGET_H

#include "ui_QijSearchWidget.h"

class QTextEdit;

#if QT_VERSION >= 0x040400
class QPlainTextEdit;
#else
#define QPlainTextEdit void
#endif

class QijSearchWidget : public QWidget, private Ui::SearchWidgetBase
{
Q_OBJECT

public:
  QijSearchWidget( QTextEdit *, QWidget *parent = 0 );
  QijSearchWidget( QPlainTextEdit *, QWidget *parent = 0 );
  QPlainTextEdit * plainTextEdit() { return _plainTextEdit; }
  QTextEdit * textEdit() { return _textEdit; }
  bool expertEnabled() { return _expertEnabled; }

public slots:
  void setTextEdit( QTextEdit * );
  void setTextEdit( QPlainTextEdit * );
  void find();
  void findAgain();
  void setExpertEnabled( bool );
  void clearSearchText();

private:
  enum direction { Forward, Backward, Stay };
  void findInTextEdit( const QString &, direction d = Stay );

  bool matchCase, wholeWords, isRegexSearch;
  bool _expertEnabled;
  direction dir;
  QTextEdit *_textEdit;
  QPlainTextEdit *_plainTextEdit;
  QString currentCursorText;

private slots:
  void on_tbClose_clicked();
  void on_leFindText_textChanged( const QString & );
  void on_leFindText_returnPressed();
  void on_tbSearchDown_clicked( bool );
  void on_tbSearchUp_clicked( bool );
  void on_tbSelectAll_clicked( bool );
  void on_chMatchCase_toggled( bool );
  void on_chWholeWords_toggled( bool );
  void on_chRegExp_toggled( bool );

 signals:
  void searchMessage( const QString & );
};

#endif
