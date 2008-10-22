project(QTM)

set( QTM_VERSION 0.7 )

IF(APPLE)
    set( PROGNAME QTM )
    set( MACOSX_BUNDLE_SHORT_VERSION_STRING ${QTM_VERSION} )
    set( MACOSX_BUNDLE_VERSION ${QTM_VERSION} )
    set( MACOSX_BUNDLE_LONG_VERSION_STRING ${QTM_VERSION} )
    set( MACOSX_BUNDLE_ICON_FILE QTM.icns )
    IF( MAC_UNIVERSAL )
        set( CMAKE_OSX_ARCHITECTURES ppc;i386 ) #Comment out if not universal binary
    ENDIF( MAC_UNIVERSAL )
ELSE(NOT APPLE)
    set( PROGNAME qtm )
ENDIF(APPLE)

find_package (Qt4 REQUIRED)

if( QT_VERSION_MINOR ) # this requires CMake 2.6
  if( QT_VERSION_MINOR LESS 2 )
    set( DONT_USE_STI TRUE )
    set( USE_DBUS FALSE )
    set( USE_STI FALSE )
  endif( QT_VERSION_MINOR LESS 2 )
endif( QT_VERSION_MINOR )

if( DONT_USE_DBUS OR WIN32 ) # Because you don't get D-Bus on Windows
  if( NOT USE_DBUS )         # But let's let the user specify it, in case someone ports it  
    set( USE_DBUS FALSE )
  endif( NOT USE_DBUS )
else( DONT_USE_DBUS )
  if( NOT APPLE )		# This doesn't preclude the user from specifying D-Bus
    set( USE_DBUS TRUE )	# on the command line.
  endif( NOT APPLE )
endif( DONT_USE_DBUS OR WIN32 )

if( DONT_USE_STI )
  set( USE_STI FALSE )
  set( USE_DBUS FALSE )
else( DONT_USE_STI )
  set( USE_STI TRUE )
endif( DONT_USE_STI )

if( USE_DBUS )
  set( USE_STI TRUE )
  set( DBUSADAPTOR_CC DBusAdaptor.cc )
  set( DBUSADAPTOR_H DBusAdaptor.h )
else( USE_DBUS )
  add_definitions( -DDONT_USE_DBUS )
endif( USE_DBUS )

if( USE_STI )
  add_definitions( -DUSE_SYSTRAYICON )
  set( SYSTRAYICON_H SysTrayIcon.h )
  set( SYSTRAYICON_CC SysTrayIcon.cc )
  set( QUICKPOSTTEMPLATEFORM_UI QuickpostTemplateForm.ui )
  set( QUICKPOSTTEMPLATEDIALOG_H QuickpostTemplateDialog.h )
  set( QUICKPOSTTEMPLATEDIALOG_CC QuickpostTemplateDialog.cc )
  set( QUICKPOSTTEMPLATE_H QuickpostTemplate.h )
  set( QUICKPOSTTEMPLATE_CC QuickpostTemplate.cc )
  if( APPLE )
    set( SUPERMENU_CC SuperMenu.cc )
    set( SUPERMENU_H SuperMenu.h )
    add_definitions( -DSUPERMENU )
  endif( APPLE )
endif( USE_STI )

# Packager can specify not to use QPlainTextEdit, even if Qt 4.4.0 is present
# Note, the author has found that it does not scroll in between going below the
# bottom of the widget and pressing Enter.

if( NOT USE_PTE )
  set( DONT_USE_PTE TRUE )
endif( NOT USE_PTE )

if( DONT_USE_PTE )
  add_definitions( -DDONT_USE_PTE )
endif( DONT_USE_PTE )

if( UNIX AND NOT APPLE )

#Generate desktop entry file

  if( NOT DESKTOP_ENTRY )
    set( DESKTOP_ENTRY qtm.desktop )
  endif( NOT DESKTOP_ENTRY )

  add_custom_command( OUTPUT ${DESKTOP_ENTRY}
    COMMAND touch ${QTM_BINARY_DIR}/${DESKTOP_ENTRY}
    COMMAND sh ${QTM_SOURCE_DIR}/qtm-desktop.sh ${CMAKE_INSTALL_PREFIX} >${QTM_BINARY_DIR}/${DESKTOP_ENTRY}
    DEPENDS qtm-desktop.sh
    COMMENT "Generating desktop entry file"
    )
  add_custom_target( DESKTOP_ENTRY_FILE ALL
    DEPENDS ${DESKTOP_ENTRY}
    )

#Generate manpage

  if( NOT MANPAGE_DIRECTORY )
    set( MANPAGE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/share/man/man1 )
  endif( NOT MANPAGE_DIRECTORY)

  add_custom_command( OUTPUT qtm.1.gz
    COMMAND touch qtm.1
    COMMAND sh ${QTM_SOURCE_DIR}/qtm-manpage.sh ${QTM_VERSION} >${QTM_BINARY_DIR}/qtm.1
    COMMAND gzip -9 ${QTM_BINARY_DIR}/qtm.1
    DEPENDS qtm-manpage.sh
    COMMENT "Generating manpage"
    )
  add_custom_target( MANPAGE_FILE ALL
    DEPENDS qtm.1.gz
    )

  set_directory_properties( ADDITIONAL_MAKE_CLEAN_FILES qtm.1 qtm.1.gz )
  
  set( APP_ICON images/qtm-logo1.png )
endif( UNIX AND NOT APPLE )

if( QDEBUG )
  set( CMAKE_VERBOSE_MAKEFILE ON )
  add_definitions( -g -O1 )
else( NOT QDEBUG )
  add_definitions( -DNO_DEBUG_OUTPUT )
  #set( CMAKE_SKIP_RPATH ON )
endif( QDEBUG )

if( NOT DESKTOP_ENTRY_PATH )
  set( DESKTOP_ENTRY_PATH ${CMAKE_INSTALL_PREFIX}/share/applications )
endif( NOT DESKTOP_ENTRY_PATH )

if( NO_SSL )
  add_definitions( -DDONT_USE_SSL )
endif( NO_SSL )

if( USE_DBUS )
  cmake_minimum_required(VERSION 2.4.5)
else( USE_DBUS )
  cmake_minimum_required(VERSION 2.4.0)
endif( USE_DBUS )

set(QTM_SRCS
    AccountsDialog.cc
    Application.cc
    EditingWindow.cc
    locationlineedit.cc
    main.cc
    PrefsDialog.cc
    QijSearchWidget.cc
    ${QUICKPOSTTEMPLATE_CC}
    ${QUICKPOSTTEMPLATEDIALOG_CC}
    SafeHttp.cc
    ${SUPERMENU_CC}
    ${SYSTRAYICON_CC}
    ${DBUSADAPTOR_CC}
    XmlRpcHandler.cc
)

set(QTM_MOC_HDRS
    AccountsDialog.h
    Application.h
    EditingWindow.h
    locationlineedit.h
    PrefsDialog.h
    QijSearchWidget.h
    ${QUICKPOSTTEMPLATE_H}
    ${QUICKPOSTTEMPLATEDIALOG_H}
    SafeHttp.h
    ${SUPERMENU_H}
    ${SYSTRAYICON_H}
    ${DBUSADAPTOR_H}
)

set(QTM_UIS
    aboutbox.ui
    AccountsForm.ui
    EditingWindowBase.ui
    PrefsForm.ui
    PrefsForm41.ui
    ImageEntry.ui
    LinkEntry.ui
    password-form.ui
    QijSearchWidget.ui
    ${QUICKPOSTTEMPLATEFORM_UI}
    SideWidget.ui
)

set(QTM_RCS
    qtm.qrc
    QijSearchWidget.qrc
)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0005 OLD)
endif(COMMAND cmake_policy)

add_definitions( -Wall )

configure_file( ${QTM_SOURCE_DIR}/qtm_version.h.in ${QTM_BINARY_DIR}/qtm_version.h @ONLY IMMEDIATE )
set( GENERATED_SOURCES ${QTM_BINARY_DIR}/qtm_version.h )

SET(QT_USE_QTNETWORK TRUE)
SET(QT_USE_QTXML TRUE)
if( USE_DBUS )
  set( QT_USE_QTDBUS TRUE )
endif( USE_DBUS )

# find_package (Qt4 REQUIRED)

include (${QT_USE_FILE})

if( USE_DBUS AND NOT QT_QTDBUS_INCLUDE_DIR )
  set( QT_QTDBUS_INCLUDE_DIR ${QT_INCLUDE_DIR}/QtDBus )
  set( QTDBUS_EXTRA QtDBus )
  add_definitions( -I${QT_QTDBUS_INCLUDE_DIR} )
endif( USE_DBUS AND NOT QT_QTDBUS_INCLUDE_DIR )

QT4_WRAP_UI(QTM_UIS_H ${QTM_UIS})

QT4_WRAP_CPP(QTM_MOC_SRCS ${QTM_MOC_HDRS})

QT4_ADD_RESOURCES(QTM_RC_SRCS ${QTM_RCS})

if(MINGW)
    add_custom_command(OUTPUT qtmico.o
                       COMMAND windres.exe -iqtm.rc -oqtmico.o)
    set(QTM_SRCS ${QTM_SRCS} qtmico.o)
endif(MINGW)

IF(APPLE)
    add_executable(${PROGNAME} MACOSX_BUNDLE ${QTM_SRCS} ${QTM_UIS_H} ${QTM_MOC_SRCS} ${QTM_RC_SRCS} ${GENERATED_SOURCES})
    add_custom_command( TARGET ${PROGNAME} POST_BUILD
      COMMAND mkdir ARGS -p ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources
      COMMAND cp ARGS ${MACOSX_BUNDLE_ICON_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources
      )
ELSE(NOT APPLE)
    add_executable(${PROGNAME} WIN32 ${QTM_SRCS} ${QTM_UIS_H} ${QTM_MOC_SRCS} ${QTM_RC_SRCS} ${GENERATED_SOURCES})
    if( NOT QDEBUG )
     if( MINGW )  
      add_custom_command( TARGET ${PROGNAME} POST_BUILD
	COMMAND strip ARGS ${QTM_BINARY_DIR}/${PROGNAME}.exe
	)
     endif( MINGW )
    endif( NOT QDEBUG )
ENDIF(APPLE)


INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR} ${QTM_SOURCE_DIR})

if( USE_RPATH )
  set( CMAKE_SKIP_BUILD_RPATH TRUE )
  set( CMAKE_BUILD_WITH_INSTALL_RPATH FALSE )
  set( CMAKE_INSTALL_RPATH "${QT_LIBRARY_DIR}" )
  set( CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE )
endif( USE_RPATH )

SET_TARGET_PROPERTIES( ${PROGNAME}
	PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE )
TARGET_LINK_LIBRARIES(${PROGNAME} ${QT_LIBRARIES} ${QTDBUS_EXTRA} )
if( QDEBUG AND MINGW )
    SET_TARGET_PROPERTIES (${PROGNAME} PROPERTIES LINK_FLAGS -Wl,-subsystem,console )
endif( QDEBUG AND MINGW )

install(TARGETS ${PROGNAME} DESTINATION bin)
if(UNIX AND NOT APPLE)
  install(FILES ${QTM_BINARY_DIR}/${DESKTOP_ENTRY} DESTINATION ${DESKTOP_ENTRY_PATH})
  install( FILES ${APP_ICON} DESTINATION share/icons )
  install( FILES ${QTM_BINARY_DIR}/qtm.1.gz DESTINATION ${MANPAGE_DIRECTORY} )
endif(UNIX AND NOT APPLE)
