INCLUDEPATH += $$PWD

QT += webenginewidgets

HEADERS += \
    $$PWD/browser.h \
    $$PWD/browserwindow.h \
    $$PWD/cookiedialog.h \
    $$PWD/downloadmanagerwidget.h \
    $$PWD/downloadwidget.h \
    $$PWD/tabwidget.h \
    $$PWD/webpage.h \
    $$PWD/webpopupwindow.h \
    $$PWD/webview.h

SOURCES += \
    $$PWD/browser.cpp \
    $$PWD/browserwindow.cpp \
    $$PWD/cookiedialog.cpp \
    $$PWD/downloadmanagerwidget.cpp \
    $$PWD/downloadwidget.cpp \
    $$PWD/tabwidget.cpp \
    $$PWD/webpage.cpp \
    $$PWD/webpopupwindow.cpp \
    $$PWD/webview.cpp

FORMS += \
    $$PWD/certificateerrordialog.ui \
    $$PWD/passworddialog.ui \
    $$PWD/downloadmanagerwidget.ui \
    $$PWD/downloadwidget.ui

RESOURCES += $$PWD/data/browser.qrc
