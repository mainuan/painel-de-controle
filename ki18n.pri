# KF6 KI18n for qmake projects.
#
# KF6 ships neither qmake modules (QT += KI18n) nor pkg-config files, so the
# headers and the library are referenced explicitly. /usr/include/KF6 is the
# location on Debian/Kubuntu, Arch and Fedora; override it with
#   qmake KF6_INCLUDEDIR=/other/prefix/include/KF6
isEmpty(KF6_INCLUDEDIR): KF6_INCLUDEDIR = /usr/include/KF6
!exists($$KF6_INCLUDEDIR/KI18n/KLocalizedString) {
    error("KF6 KI18n headers not found in $$KF6_INCLUDEDIR/KI18n (install libkf6i18n-dev or pass KF6_INCLUDEDIR=...)")
}
INCLUDEPATH += $$KF6_INCLUDEDIR/KI18n
LIBS += -lKF6I18n

# Generated ui_*.h call tr2i18n() (KI18n) instead of QObject::tr().
QMAKE_UIC_FLAGS += -tr tr2i18n
