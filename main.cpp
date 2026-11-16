#include "appwindow.h"

#include <KLocalizedString>

#include <QApplication>

int main(int argc, char *argv[])
{
    // gettext domain; catalogs live in $XDG_DATA_DIRS/locale
    // (/usr/share/locale/<lang>/LC_MESSAGES/painel-controle.mo when installed).
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("painel-controle"));
    // Optional: run from a build tree with PAINEL_CONTROLE_LOCALEDIR=<build>/locale.
    const QString localeDirectory = qEnvironmentVariable("PAINEL_CONTROLE_LOCALEDIR");
    if (!localeDirectory.isEmpty()) {
        KLocalizedString::addDomainLocaleDir(QByteArrayLiteral("painel-controle"), localeDirectory);
    }

    QApplication a(argc, argv);
    AppWindow w;
    w.show();
    return a.exec();
}
