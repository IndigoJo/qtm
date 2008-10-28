/*******************************************************************************

    QTM - Qt-based blog manager
    Copyright (C) 2006 Matthew J Smith

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License (version 2), as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*******************************************************************************/

// EditingWindow.h - Headers for QTM main window class.

#ifndef CATKIN_H
#define CATKIN_H

#include "ui_EditingWindowBase.h"
#include "ui_SideWidget.h"
#include "ui_aboutbox.h"
//#include "ui_account_form.h"
#include "ui_ImageEntry.h"
#include "ui_LinkEntry.h"
#include "ui_password-form.h"
#include "AccountsDialog.h"
#include "PrefsDialog.h"
#include "QijSearchWidget.h"
// #include "ui_ListDialog.h"
#ifdef USE_SYSTRAYICON
#include "SysTrayIcon.h"
#endif
#include "Application.h"

#include <QHash>
#include <QtXml>

class QWidget;
class QStackedWidget;
// class QSplitter;
class QTextBrowser;
class QPushButton;
class QAction;
class QResizeEvent;
class QComboBox;
//class QHttp;
class SafeHttp;
class QHttpResponseHeader;
class QTextEdit;
class RichTextEdit;
class QHBoxLayout;
class QRegExpValidator;
class QHostInfo;
class QSystemTrayIcon;
class QMenu;

using namespace Ui;

#if QT_VERSION >= 0x040400 && !defined DONT_USE_PTE
class QPlainTextEdit;
#define TEXTEDIT QPlainTextEdit
#else
#define TEXTEDIT QTextEdit
#endif

class EditingWindow : public QMainWindow
{
  Q_OBJECT

 public:
  EditingWindow( bool noRefreshBlogs = false, QWidget *widget = 0 );
  /*  Catkin( QList<QString>, QList<QString>, int,
      QWidget *parent = 0 );*/
  EditingWindow( QString, QWidget *parent = 0 );
  ~EditingWindow();
#if QT_VERSION >= 0x040200
#ifdef USE_SYSTRAYICON
  void setSTI( SysTrayIcon * );
#endif
#endif
  static bool caseInsensitiveLessThan( const QString &, const QString & );
  bool handleArguments();
  QString postTitle();

 private:
  Ui::MainWindow ui;
  Ui::CategoryWidget cw;
  void doUiSetup();
  void checkForEmptySettings();
  void setInitialAccount();
  void positionWidget( QWidget *, QWidget * );
  void readSettings();
  void writeSettings();
  void callRefreshCategories();
  //void blogger_newPost( QByteArray );
  //void blogger_editPost( QByteArray );
  void blogger_getUsersBlogs( QByteArray );
  void metaWeblog_newPost( QByteArray );
  void metaWeblog_editPost( QByteArray );
  void metaWeblog_newMediaObject( QByteArray );
  void mt_publishPost( QByteArray );
  void mt_getCategoryList( QByteArray );
  void mt_setPostCategories( QByteArray );
  //void submitBloggerEdit();
  void submitMTEdit();
  bool saveCheck();
  bool saveCheck( bool );
  QDomElement XmlValue( QDomDocument &, QString, QString,
			QString memberType = "param" );
  QDomElement XmlMember( QDomDocument &, QString, QString, QString );
  QDomElement XmlMethodName( QDomDocument &, QString );
  QDomElement XmlRpcArray( QDomDocument &, QString, QList<QString> );
  void setNetworkActionsEnabled( bool );
  //void newBloggerPost();
  QString & getHTMLList( QString, QString & );
  void saveAutoLinkDictionary();
  void loadAutoLinkDictionary();
  QByteArray toBase64( QByteArray & );
  void addToConsole( const QString & );
  void handleEnableCategories();
  void updateRecentFileList( const QString &, const QString & );
  void updateRecentFileMenu();
  void saveAccountsDom();

  int currentAccount, currentBlog, loadedEntryBlog;
  QString loadedAccountId;
  QString applicationVersion;
  QString server;
  QString location;
  QString login;
  QString password;
  QString port;
  QString localStorageDirectory;
  QString localStorageFileExtn;
  QString remoteFileLocation;
  QString lastAccountID;
  bool categoriesEnabled, entryBlogged, useNewWindows, savePassword,
      postAsSave, noPassword, initialChangeBlog, allowComments, allowTB, postDateTime,
    copyTitle, allowRegexSearch, useTwoNewlines;
  QHash<QString, bool *> accountAttributes;
  QHash<QString, QString *> accountStrings;
  bool entryEverSaved, cleanSave, noAutoSave, noAlphaCats;
  QList<QString> usersBlogs;
  QList<QString> categoryList;
  QStringList usersBlogURIs, usersBlogNames, usersBlogIDs;
  QStringList categoryNames, categoryIDs;
  QStringList qtmaccounts_xml;
  // QHash<QString, QString> catList;
  QHash<QString, QString> autoLinkDictionary;
  QHash<QString, QString> autoLinkTitleDictionary;
  QHash<QString, int> autoLinkTargetDictionary;
  QByteArray responseData;
  QString entryNumber, dateTime;
  QString filename;
  int primaryCat;
  QString otherCatsList;
  QRegExpValidator *tagValidator;
  QString editorFontString, previewFontString, consoleFontString;
  bool useBloggerTitleFormatting;
  int bloggerTitleFormat;
  QStringList bloggerTitleFormatStrings;
  QLabel *dirtyIndicator;
  int STI2ClickFunction;
  QDomDocument accountsDom;
  QDomElement accountsElement, currentAccountElement,
    currentBlogElement, currentCategoryElement;
  QString currentAccountId, currentBlogid;
  QString userAgentString;
  Application *qtm;
  /*  typedef struct _RF {
    QString filename;
    QString title;
    } recentFile; */
  QAction *recentFileActions[10];
  QList<Application::recentFile> recentFiles;
#ifdef USE_SYSTRAYICON
  SysTrayIcon *sti;
#endif

 private slots:
  void about();
  void choose( const QString fname = QString() );
  void openRecentFile();
  void save();
  void save( const QString &, bool exp = false );
  void exportEntry();
  void saveAs( bool exp = false );
  void stopThisJob();
  void handleDone( bool );
  void handleResponseHeader( const QHttpResponseHeader & );
  // void changeCurrentAccount( int );
  void changeCurrentBlog( int );
  void changeAccount( int );
  void extractAccountDetails();
  void changeBlog( int );
  void handleConsole( bool );
  void makeBold();
  void makeItalic();
  void makeUnderline();
  void makeBlockquote();
  void makePara();
  void insertLink( bool isAutoLink = false );
  void insertLinkFromClipboard();
  void insertSelfLink();
  void insertAutoLink();
  void insertImage();
  void insertImageFromClipboard();
  void insertMore();
  void toggleRichText( bool );
  void cut();
  void copy();
  void paste();
  void pasteAsMarkedParagraphs();
  void pasteAsBlockquote();
  void pasteAsMarkdownBlockquote();
  void pasteAsUnorderedList();
  void pasteAsOrderedList();
  void undo();
  void redo();
  void makeUnorderedList();
  void makeOrderedList();
  void doPreview( bool );
  void showHighlightedURL( const QString & );
  //void blogThis();
  void newMTPost(); // formerly blogThis()
  void updatePostCategories();
  void saveBlogs();
  void setPostCategories();
  void publishPost();
  void setLoadedPostCategories();
  void doWhatsThis();
  void doViewBasicSettings();
  void doViewCategories();
  void changeOtherCatsHeading();
  void doViewExcerpt();
  void doViewTechTags();
  void doViewTBPings();
  void addTechTag();
  void addClipTag();
  void addTechTagFromLineEdit();
  void addTechTagFromAddButton();
  void removeTechTag();
  void addTBPing();
  void addClipTBPing();
  void addTBPingFromLineEdit();
  void addTBPingFromAddButton();
  void removeTBPing();
  void handleInitialLookup( const QHostInfo & );
  void handleHostLookupFailed();
  void doFont();
  void doPreviewFont();
  void doConsoleFont();
  void configureQuickpostTemplates();
  void uploadFile();
  void copyURL();
  void handleFind();
  void populateAccountList();
  void populateBlogList();

 public slots:
  void refreshCategories();
  void getAccounts();
  void getPreferences();
  void refreshBlogList();
  void changeCaptionAfterTitleChanged();
  //bool load( const QString &, QList<QString>, QList<QString> );
  /*bool load( const QString &, QStringList, QStringList, QStringList,
    QStringList, QStringList ); */
  bool load( const QString &, QDomDocument & );
  bool load( const QString &, bool fromSTI = false );
  void dirtify();
  void setDirtySignals( bool );
  void doInitialChangeBlog();
  void newDoc();
  void doQuit();
  void setPublishStatus( int );
  void setPostTitle( const QString & );
  void setPostClean();
  void showStatusBarMessage( const QString & );
  void setRecentFiles( const QList<Application::recentFile> & );

 protected:
  QStackedWidget *mainStack;
  QTextBrowser *previewWindow;
  QWidget *leftWidget, *cWidget;
  QijSearchWidget *searchWidget;
  QPushButton *previewButton;
  QPushButton *blogThisButton;
  QAction *blogThisIntlAction;
  SafeHttp *http;
  TEXTEDIT *console;
  TEXTEDIT *ed;
  RichTextEdit *rted;
  QVBoxLayout *mainWindowLayoutWithSearch;
  QHBoxLayout *mainWindowLayout;
  QPushButton *pbCopyURL;
  QMenu *recentFilesMenu;
  QAction *noRecentFilesAction;

  int previewWindowID;
  int consoleID, edID;
  int previousRaisedLSWidget;
  int currentHttpBusiness;
  // virtual void resizeEvent( QResizeEvent *e );
  virtual void closeEvent( QCloseEvent * );
  virtual void showEvent( QShowEvent * );

signals:
  void httpBusinessFinished();
  void categoryRefreshFinished();
  void blogRefreshFinished();
  //void loadingDone( QWidget *, bool );
};

#endif
