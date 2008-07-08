// Highlighter.h - Syntax highlighter for HTML tags in QTM documents
//
// Written by Matthew J Smith
//
// Copyright 2008 Matthew J Smith
// Distributed under GNU General Public License, version 2 or later

#ifdef HTMLHIGHLIGHTER_H
#define HTMLHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QString>

class Highlighter : public QSyntaxHighlighter
{
Q_OBJECT

public:
  Highlighter::Highligher( QObject *parent = 0 );
  Highlighter::~Highlighter();

protected:
  virtual void highlightBlock( const QString & );

private:
  struct HighlightingRule {
    QRegExp pattern;
    QTextCharFormat format;
    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat htmlTagFormat;
    QTextCharFormat htmlEntityFormat;
    QTextCharFormat htmlCommentFormat;
};


#endif
