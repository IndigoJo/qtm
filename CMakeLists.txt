project(QTM)

set( QTM_VERSION 0.6.9 )

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

if( DONT_USE_DBUS )
  set( USE_DBUS FALSE )
else( DONT_USE_DBUS )
  set( USE_DBUS TRUE )

endif( DONT_USE_DBUS )

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
    catkin.cc
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
    catkin.h
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
    PrefsForm.ui
    PrefsForm41.ui
    CatkinMainWindow.ui
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
if( WIN32 AND NOT UNIX )
  file( REMOVE qtm_version.h )
  add_custom_command( OUTPUT qtm_version.h
    COMMAND qtm_version.bat ${QTM_VERSION}
    DEPENDS qtm_version.bat	 
    COMMENT "Generating version header"
  )
  add_custom_target( WINDOWS_VERSION_HEADER ALL
    DEPENDS qtm_version.h
  )
else( WIN32 AND NOT UNIX )
  file( REMOVE qtm_version.h )
  add_custom_command( OUTPUT qtm_version.h
    COMMAND sh ${QTM_SOURCE_DIR}/qtm_version.sh ${QTM_VERSION}
    DEPENDS qtm_version.sh
    COMMENT "Generating version header"
  )
  add_custom_target( VERSION_HEADER ALL
    DEPENDS qtm_version.h
  )
endif( WIN32 AND NOT UNIX )

SET(QT_USE_QTNETWORK TRUE)
SET(QT_USE_QTXML TRUE)
if( USE_DBUS )
  set( QT_USE_QTDBUS TRUE )
endif( USE_DBUS )

# find_package (Qt4 REQUIRED)

include (${QT_USE_FILE})

if( NOT QT_QTDBUS_INCLUDE_DIR )
  set( QT_QTDBUS_INCLUDE_DIR ${QT_INCLUDE_DIR}/QtDBus )
  set( QTDBUS_EXTRA QtDBus )
  add_definitions( -I${QT_QTDBUS_INCLUDE_DIR} )
endif( NOT QT_QTDBUS_INCLUDE_DIR )

QT4_WRAP_UI(QTM_UIS_H ${QTM_UIS})

QT4_WRAP_CPP(QTM_MOC_SRCS ${QTM_MOC_HDRS})

QT4_ADD_RESOURCES(QTM_RC_SRCS ${QTM_RCS})

if(MINGW)
    add_custom_command(OUTPUT qtmico.o
                       COMMAND windres.exe -iqtm.rc -oqtmico.o)
    set(QTM_SRCS ${QTM_SRCS} qtmico.o)
endif(MINGW)

IF(APPLE)
    add_executable(${PROGNAME} MACOSX_BUNDLE ${QTM_SRCS} ${QTM_UIS_H} ${QTM_MOC_SRCS} ${QTM_RC_SRCS})
    add_custom_command( TARGET ${PROGNAME} POST_BUILD
      COMMAND mkdir ARGS -p ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources
      COMMAND cp ARGS ${MACOSX_BUNDLE_ICON_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources
      )
ELSE(NOT APPLE)
    add_executable(${PROGNAME} WIN32 ${QTM_SRCS} ${QTM_UIS_H} ${QTM_MOC_SRCS} ${QTM_RC_SRCS})
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
