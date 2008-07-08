// Highlighter.cc - Syntax highlighter for HTML elements in QTM documents
//
// Copyright 2008 Matthew J Smith
// Distributed under the GNU General Public License, version 2 or later

#include "Highlighter.h"

Highlighter::Highlighter( QObject *parent )
  : QSyntaxHighlighter( parent )
{

}

Highlighter::~Highlighter()
{

}

void Highlighter::highlightBlock( const QString &block )
{
// Formatting for HTML tags
  htmlTagFormat.setForeground( Qt::darkRed );

// Formatting for HTML entities, i.e. &eacute; and so on
  htmlEntityFormat.setForeground( Qt::darkMagenta );
}
