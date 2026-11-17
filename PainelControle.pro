QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Installed program name (the 1.0 package already shipped /usr/bin/painel-controle).
TARGET = painel-controle

# Translations: KDE KI18n, gettext domain "painel-controle" (docs/i18n/README.md).
include(ki18n.pri)

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    appwindow.cpp

HEADERS += \
    appwindow.h \
    cataloglayoutdirection.h

FORMS += \
    appwindow.ui

RESOURCES += \
    resource.qrc

# Installation: qmake6 PREFIX=/usr (default) && make install INSTALL_ROOT=<staging>
isEmpty(PREFIX): PREFIX = /usr

target.path = $$PREFIX/bin
INSTALLS += target

manpage.files = $$PWD/docs/painel-controle.1
manpage.path = $$PREFIX/share/man/man1
INSTALLS += manpage

# Compile po/<lang>.po (languages listed in po/LINGUAS) into
# <build>/locale/<lang>/LC_MESSAGES/painel-controle.mo and install them to
# $$PREFIX/share/locale (default PREFIX=/usr, where KI18n looks them up).
I18N_DOMAIN = painel-controle
I18N_LANGUAGES = $$cat($$PWD/po/LINGUAS, lines)
for(language, I18N_LANGUAGES) {
    I18N_PO_FILES += $$PWD/po/$${language}.po
    translation_$${language}.files = $$OUT_PWD/locale/$${language}/LC_MESSAGES/$${I18N_DOMAIN}.mo
    translation_$${language}.path = $$PREFIX/share/locale/$${language}/LC_MESSAGES
    translation_$${language}.CONFIG += no_check_exist
    INSTALLS += translation_$${language}
}
msgfmt.input = I18N_PO_FILES
msgfmt.output = locale/${QMAKE_FILE_BASE}/LC_MESSAGES/$${I18N_DOMAIN}.mo
msgfmt.commands = $$QMAKE_MKDIR locale/${QMAKE_FILE_BASE}/LC_MESSAGES && msgfmt --check -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_IN}
msgfmt.CONFIG = no_link target_predeps
QMAKE_EXTRA_COMPILERS += msgfmt

# The launcher is generated from data/painel-controle.desktop.in with every
# translation from po/ (msgfmt --desktop reads po/LINGUAS).
desktop.target = painel-controle.desktop
desktop.commands = msgfmt --desktop --template=$$PWD/data/painel-controle.desktop.in -d $$PWD/po -o painel-controle.desktop
desktop.depends = $$PWD/data/painel-controle.desktop.in $$PWD/po/LINGUAS $$I18N_PO_FILES
QMAKE_EXTRA_TARGETS += desktop
PRE_TARGETDEPS += painel-controle.desktop
QMAKE_CLEAN += painel-controle.desktop
desktopfile.files = $$OUT_PWD/painel-controle.desktop
desktopfile.path = $$PREFIX/share/applications
desktopfile.CONFIG += no_check_exist
INSTALLS += desktopfile
