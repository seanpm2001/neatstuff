#always install the library
win32 {
   dlltarget.path=$$[QT_INSTALL_BINS]
   INSTALLS += dlltarget
}
target.path=$$[QT_INSTALL_LIBS]
INSTALLS += target

#headers
qt_install_headers {
    INSTALL_HEADERS = $$SYNCQT.HEADER_FILES $$SYNCQT.HEADER_CLASSES
    equals(TARGET, QtCore) {
       #headers generated by configure
       INSTALL_HEADERS *= $$QT_BUILD_TREE/src/corelib/global/qconfig.h \
                          $$QT_SOURCE_TREE/src/corelib/arch/$$ARCH/arch
    }

    flat_headers.files = $$INSTALL_HEADERS
    flat_headers.path = $$[QT_INSTALL_HEADERS]/Qt
    INSTALLS += flat_headers

    targ_headers.files = $$INSTALL_HEADERS
    targ_headers.path = $$[QT_INSTALL_HEADERS]/$$TARGET
    INSTALLS += targ_headers
}

embedded:equals(TARGET, QtGui) {
    # install fonts for embedded
    INSTALLS += fonts
    fonts.path = $$[QT_INSTALL_LIBS]/fonts
    fonts.files = $$QT_SOURCE_TREE/lib/fonts/*
}