QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

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

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

# Compile po/<lang>.po (languages listed in po/LINGUAS) into
# <build>/locale/<lang>/LC_MESSAGES/painel-controle.mo and install them to
# $$PREFIX/share/locale (default PREFIX=/usr, where KI18n looks them up).
I18N_DOMAIN = painel-controle
isEmpty(PREFIX): PREFIX = /usr
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
