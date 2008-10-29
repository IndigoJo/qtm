// RichTextEdit.cc - Rich Text Editor widget for QTM (and other apps, perhaps)
// derived from the KRichTextEdit widget of KDE 4.1
//
// Copyright 2007 Laurent Montel
// Copyright 2007 Thomas McGuire
// Copyright 2008 Stephen Kelly
// Copyright 2008 Matthew J Smith <indigojo@blogistan.co.uk>

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License (version 2), as
// published by the Free Software Foundation.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "RichTextEdit.h"

// This include is for a class derived from KDE.  We don't include their link
// dialog, because we have our own.
#include "nestedlisthelper.h"

#include <QTextDocumentFragment>
#include <QMouseEvent>

class RichTextEditPrivate : public QObject
{
public:
  RichTextEditPrivate( RichTextEdit *parent )
    :q( parent ),
     mMode( RichTextEdit::Plain ) {
    nestedListHelper = new NestedListHelper( q );
  }

  ~RichTextEditPrivate() {
    delete nestedListHelper;
  }

  // Normal functions

  // If the text under the cursor is a link, the cursor's selection is set
  // to the complete link text.  Otherwise, it selects the current word if
  // there's no selection.
  void selectLinkText() const;

  void init();

  // Switches to RT mode and emits the mode changed signal if the mode
  // really changed
  void activateRichText();

  // Applies formatting to current word if there is no selection
  void mergeFormatOnWordOrSelection( const QTextCharFormat &format );

  void setTextCursor( QTextCursor &cursor );

  // Data members

  RichTextEdit *q;
  RichTextEdit::Mode mMode;

  NestedListHelper *nestedListHelper;

};

void RichTextEditPrivate::activateRichText()
{
  if( mMode = RichTextEdit::Plain ) {
    q->setAcceptRichText( true );
    mMode = RichTextEdit::Rich;
    emit q->textModeChanged( mMode );
  }
}

void RichTextEditPrivate::setTextCursor( QTextCursor &cursor )
{
  q->setTextCursor( cursor );
}

void RichTextEditPrivate::mergeFormatOnWordOrSelection( const QTextCharFormat &format )
{
  QTextCursor cursor = q->textCursor();
  cursor.beginEditBlock();
  if( !cursor.hasSelection() )
    cursor.select( QTextCursor::WordUnderCursor );
  cursor.mergeCharFormat( format );
  q->mergeCurrentCharFormat( format );
  cursor.endEditBlock();
}

// Now, the methods for RichTextEdit itself

RichTextEdit::RichTextEdit( QWidget *parent )
  : QTextEdit( parent ), d( new RichTextEditPrivate( this ) )
{
  // d->init();
}

RichTextEdit::~RichTextEdit()
{
  delete d;
}

void RichTextEdit::setListStyle( int _styleIndex )
{
  d->nestedListHelper->handleOnBulletType( -_styleIndex );
  setFocus();
  d->activateRichText();
}

void RichTextEdit::indentListMore()
{
  d->nestedListHelper->handleOnIndentMore();
  d->activateRichText();
}

void RichTextEdit::indentListLess()
{
  d->nestedListHelper->handleOnIndentLess();
}

void RichTextEdit::insertHorizontalRule( QString &id )
{
  QTextCursor cursor = textCursor();
  QTextBlockFormat bf = cursor.blockFormat();
  QTextCharFormat cf = cursor.charFormat();

  cursor.beginEditBlock();
  cursor.insertHtml( "<hr />" );
  cursor.insertBlock( bf, cf );
  setTextCursor( cursor );
  d->activateRichText();
  cursor.endEditBlock();
}

void RichTextEdit::alignLeft()
{
  setAlignment( Qt::AlignLeft );
  setFocus();
  d->activateRichText();
}

void RichTextEdit::alignCenter()
{
  setAlignment( Qt::AlignHCenter );
  setFocus();
  d->activateRichText();
}

void RichTextEdit::alignRight()
{
  setAlignment( Qt::AlignRight );
  setFocus();
  d->activateRichText();
}

void RichTextEdit::alignJustify()
{
  setAlignment( Qt::AlignJustify );
  setFocus();
  d->activateRichText();
}

void RichTextEdit::setTextBold( bool bold )
{
  QTextCharFormat fmt;
  fmt.setFontWeight( bold ? QFont::Bold : QFont::Normal );
  d->mergeFormatOnWordOrSelection(fmt);
  setFocus();
  d->activateRichText();
}

void RichTextEdit::setTextItalic( bool ital )
{
  QTextCharFormat fmt;
  fmt.setFontItalic( ital );
  d->mergeFormatOnWordOrSelection(fmt);
  setFocus();
  d->activateRichText();
}

void RichTextEdit::setTextUnderline( bool ul )
{
  QTextCharFormat fmt;
  fmt.setFontUnderline( ul );
  d->mergeFormatOnWordOrSelection(fmt);
  setFocus();
  d->activateRichText();
}

void RichTextEdit::setTextStrikeout( bool st )
{
  QTextCharFormat fmt;
  fmt.setFontStrikeOut( st );
  d->mergeFormatOnWordOrSelection(fmt);
  setFocus();
  d->activateRichText();
}

void RichTextEdit::setTextForegroundColor( const QColor &color )
{
  QTextCharFormat fmt;
  fmt.setForeground( color );
  d->mergeFormatOnWordOrSelection(fmt);
  setFocus();
  d->activateRichText();
}

void RichTextEdit::setTextBackgroundColor( const QColor &color )
{
  QTextCharFormat fmt;
  fmt.setBackground( color );
  d->mergeFormatOnWordOrSelection(fmt);
  setFocus();
  d->activateRichText();
}

void RichTextEdit::setFontFamily( const QString &ff )
{
  QTextCharFormat fmt;
  fmt.setFontFamily( ff );
  d->mergeFormatOnWordOrSelection(fmt);
  setFocus();
  d->activateRichText();
}

void RichTextEdit::setFontSize( int size )
{
  QTextCharFormat fmt;
  fmt.setFontPointSize( size );
  d->mergeFormatOnWordOrSelection(fmt);
  setFocus();
  d->activateRichText();
}

void RichTextEdit::switchToPlainText()
{
  if( d->mMode == Rich ) {
    d->mMode = Plain;
    document()->setPlainText( document()->toPlainText() );
    setAcceptRichText( false );
    emit textModeChanged( d->mMode );
  }
}

void RichTextEdit::enableRichTextMode()
{
  d->activateRichText();
}

RichTextEdit::Mode RichTextEdit::textMode() const
{
  return d->mMode;
}

QString RichTextEdit::textOrHtml() const
{
  if( textMode() == Rich )
    return toCleanHtml();
  else
    return toPlainText();
}

QString RichTextEdit::currentLinkText() const
{
  QTextCursor cursor = textCursor();
  selectLinkText( &cursor );
  return cursor.selectedText();
}

void RichTextEdit::selectLinkText() const
{
  QTextCursor cursor = textCursor();
  selectLinkText( &cursor );
  d->setTextCursor( cursor );
}

void RichTextEdit::selectLinkText( QTextCursor *cursor ) const
{
  // If the cursor is on a link, select the text of the link.
  if( cursor->charFormat().isAnchor() ) {
    QString aHref = cursor->charFormat().anchorHref();

    // Move cursor to start of link
    while( cursor->charFormat().anchorHref() == aHref ) {
      if( cursor->atStart() )
	break;
      cursor->setPosition( cursor->position() - 1 );
    }
    if( cursor->charFormat().anchorHref() != aHref )
      cursor->setPosition( cursor->position() + 1, QTextCursor::KeepAnchor );

    // Move selection to the end of the link
    while( cursor->charFormat().anchorHref() == aHref ) {
      if( cursor->atEnd() )
	break;
      cursor->setPosition( cursor->position() + 1, QTextCursor::KeepAnchor );
    }
    if( cursor->charFormat().anchorHref() != aHref )
      cursor->setPosition( cursor->position() - 1, QTextCursor::KeepAnchor );
  } else if( cursor->hasSelection() ) {
    // Nothing to to. Using the currently selected text as the link text.
  } else {

    // Select current word
    cursor->movePosition( QTextCursor::StartOfWord );
    cursor->movePosition( QTextCursor::EndOfWord, QTextCursor::KeepAnchor );
  }
}

QString RichTextEdit::currentLinkUrl() const
{
  return textCursor().charFormat().anchorHref();
}

void KRichTextEdit::updateLink( const QString &linkUrl, const QString &linkText )
{
  QTextCursor cursor = textCursor();
  cursor.beginEditBlock();
  QTextCharFormat format = cursor.charFormat();

  selectLinkText();
  if( !cursor.hasSelection() ) {
    cursor.select( QTextCursor::WordUnderCursor );
  }

  if( !linkUrl.isEmpty() ) {
    format.setAnchor( true );
    format.setAnchorHref( linkUrl );
  } else {
    format = cursor.block().charFormat();
    format.setAnchor( false );
    format.setAnchorHref( QString() );
  }

  QString _linkText;

  int lowPos = qMin( cursor.selectionStart(), cursor.selectionEnd() );
  if( !linkText.isEmpty() ) {
    _linkText = linkText;
  } else {
    _linkText = linkUrl;
  }
  // Can't simply insertHtml("<a href=\"%1\">%2</a>").arg(linkUrl).arg(_linkText);
  // That would remove all existing text formatting on the selection (bold etc).
  // The href information is stored in the QTextCharFormat, but qt bugs must
  // still be worked around below.
  cursor.insertText( _linkText, format );


  // Workaround for qt bug 203510:
  // Link formatting does not get applied immediately. Removing and reinserting
  // the marked up html does format the text correctly.
  // -- Stephen Kelly, 15th March 2008
  if( !linkUrl.isEmpty() ) {
    cursor.setPosition( lowPos );
    cursor.setPosition( lowPos + _linkText.length(), QTextCursor::KeepAnchor );

    if( !cursor.currentList() ) {
      cursor.insertHtml( cursor.selection().toHtml() );
    } else {
      // Workaround for qt bug 215576:
      // If the cursor is currently on a list, inserting html will create a new block.
      // This seems to be because toHtml() does not create a <!-- StartFragment --> tag in
      // this case and text style information is stored in the list item rather than a span tag.
      // -- Stephen Kelly, 8th June 2008

      QString selectionHtml = cursor.selection().toHtml();
      QString style = selectionHtml.split( "<li style=\"" ).takeAt( 1 ).split( "\"" ).first();
      QString linkTag = "<a" + selectionHtml.split( "<a" ).takeAt( 1 ).split( '>' ).first() + '>'
	+ "<span style=\"" + style + "\">" + _linkText + "</span></a>";
      cursor.insertHtml( linkTag );
    }

    // Insert a space after the link if at the end of the block so that
    // typing some text after the link does not carry link formatting
    if( cursor.position() == cursor.block().position() + cursor.block().length() - 1 ) {
      cursor.setCharFormat( cursor.block().charFormat() );
      cursor.insertText( QString(' ') );
    }

    d->activateRichText();
  } else {
    // Remove link formatting. This is a workaround for the same qt bug (203510).
    // Just remove all formatting from the link text.
    QTextCharFormat charFormat;
    cursor.setCharFormat( charFormat );
  }

  cursor.endEditBlock();
}

void KRichTextEdit::keyPressEvent( QKeyEvent *event )
{
    bool handled = false;
    if( textCursor().currentList() ) {
        // handled is False if the key press event was not handled or not completely
        // handled by the Helper class.
        handled = d->nestedListHelper->handleBeforeKeyPressEvent( event );
    }

    if( !handled ) {
        KTextEdit::keyPressEvent( event );
    }

    if( textCursor().currentList() ) {
        d->nestedListHelper->handleAfterKeyPressEvent( event );
    }
    emit cursorPositionChanged();
}

bool KRichTextEdit::canIndentList() const
{
    return d->nestedListHelper->canIndent();
}

bool KRichTextEdit::canDedentList() const
{
    return d->nestedListHelper->canDedent();
}

QString KRichTextEdit::toCleanHtml() const
{
    static QString evilline = 
      "<p style=\" margin-top:0px; margin-bottom:0px; "
      "margin-left:0px; margin-right:0px; -qt-block-indent:0; "
      "text-indent:0px; -qt-user-state:0;\">";

    QString result;
    QStringList lines = toHtml().split( "\n" );
    Q_FOREACH( QString tempLine, lines ) {
        if( tempLine.startsWith( evilline ) ) { 
            tempLine.remove( evilline );
            if (tempLine.endsWith( "</p>" ) ) {
                tempLine.remove (QRegExp( "</p>$" ) );
                tempLine.append( "<br>\n" );
            }
            result += tempLine;
        } else {
            result += tempLine;
        }
    }

    // ### HACK to fix bug 86925: A completely empty line is ignored in HTML-mode
    int offset = 0;
    QRegExp paragraphFinder( "<p.*>(.*)</p>" );
    QRegExp paragraphEnd( "</p>" );
    paragraphFinder.setMinimal( true );

    while( offset != -1 ) {

        // Find the next paragraph
        offset = paragraphFinder.indexIn( result, offset );

        if( offset != -1 ) {

            // If the text in the paragraph is empty, add a &nbsp there.
            if( paragraphFinder.capturedTexts().size() == 2 &&
                paragraphFinder.capturedTexts()[1].isEmpty() ) {
                int end = paragraphEnd.indexIn( result, offset );
                Q_ASSERT( end != -1 && end > offset );
                result.replace( end, paragraphEnd.pattern().length(), "<br></p>" );
            }

            // Avoid finding the same match again
            offset++;
        }
    }

    return result;
}
