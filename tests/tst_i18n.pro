# i18n test: builds the real AppWindow and checks its labels per language.
# Use tests/run-i18n-tests.sh, which also builds the catalogs.
QT += core gui widgets testlib
CONFIG += c++17 testcase no_testcase_installs
TARGET = tst_i18n

include(../ki18n.pri)

INCLUDEPATH += $$PWD/..
SOURCES += tst_i18n.cpp ../appwindow.cpp ../systeminfo.cpp
HEADERS += ../appwindow.h ../systeminfo.h ../cataloglayoutdirection.h
FORMS += ../appwindow.ui
RESOURCES += ../resource.qrc
