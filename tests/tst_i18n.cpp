// Proves that the real AppWindow (appwindow.ui compiled with "uic -tr tr2i18n")
// is translated through KI18n: LANGUAGE=pt_BR shows the original Portuguese
// labels, LANGUAGE=en or the C locale shows the English source strings, and the
// kcm_* identifiers handed to kcmshell6 never change. Run by run-i18n-tests.sh,
// which sets PAINEL_CONTROLE_EXPECTED_LANGUAGE and PAINEL_CONTROLE_LOCALEDIR.

#include "appwindow.h"
#include "cataloglayoutdirection.h"

#include <KLocalizedString>

#include <QApplication>
#include <QListWidget>
#include <QPushButton>
#include <QTabWidget>
#include <QTest>

#include <clocale>

class I18nTest final : public QObject
{
    Q_OBJECT

private slots:
    void windowUsesExpectedLanguage();
    void kcmIdentifiersAreNotTranslated();
    void rightToLeftCatalogMirrorsLayout();
};

void I18nTest::windowUsesExpectedLanguage()
{
    const bool pt = qEnvironmentVariable("PAINEL_CONTROLE_EXPECTED_LANGUAGE") == QLatin1String("pt_BR");
    // gettext (and therefore KI18n) ignores LANGUAGE while LC_MESSAGES is C.
    const QByteArray messagesLocale = std::setlocale(LC_MESSAGES, nullptr);
    if (pt && (messagesLocale == "C" || messagesLocale == "POSIX" || messagesLocale.startsWith("C."))) {
        QSKIP("LC_MESSAGES is the C locale; run with LANG=en_US.UTF-8 (or another installed locale)");
    }

    AppWindow window;
    QCOMPARE(window.windowTitle(), pt ? QStringLiteral("Painel de controle") : QStringLiteral("Control Panel"));

    auto *advanced = window.findChild<QPushButton *>(QStringLiteral("callSystemSettings"));
    QVERIFY(advanced);
    QCOMPARE(advanced->text(), pt ? QStringLiteral("Modo avançado") : QStringLiteral("Advanced Mode"));

    auto *tabs = window.findChild<QTabWidget *>(QStringLiteral("tabWidget"));
    QVERIFY(tabs);
    QCOMPARE(tabs->tabText(0), QStringLiteral("Desktop"));
    QCOMPARE(tabs->tabText(1), pt ? QStringLiteral("Notebook") : QStringLiteral("Laptop"));

    // The sidebar is built from the hotspot buttons of the current tab.
    auto *sidebar = window.findChild<QListWidget *>(QStringLiteral("sidebar"));
    QVERIFY(sidebar);
    QStringList items;
    for (int row = 0; row < sidebar->count(); ++row) {
        items << sidebar->item(row)->text();
    }
    const QStringList expected = pt
        ? QStringList{QStringLiteral("Rede, WiFi e Internet"), QStringLiteral("Impressoras"),
                      QStringLiteral("Data & Hora"), QStringLiteral("Microfones e autofalantes"),
                      QStringLiteral("Armazenamento removível"), QStringLiteral("Câmeras digitais")}
        : QStringList{QStringLiteral("Network, Wi-Fi and Internet"), QStringLiteral("Printers"),
                      QStringLiteral("Date & Time"), QStringLiteral("Microphones and Speakers"),
                      QStringLiteral("Removable Storage"), QStringLiteral("Digital Cameras")};
    for (const QString &text : expected) {
        QVERIFY2(items.contains(text), qPrintable(text + QLatin1String(" not in: ") + items.join(QLatin1String(" | "))));
    }
}

void I18nTest::kcmIdentifiersAreNotTranslated()
{
    AppWindow window;
    auto *mouse = window.findChild<QPushButton *>(QStringLiteral("pushButton_26"));
    QVERIFY(mouse);
    QCOMPARE(mouse->property("kcms").toStringList(), QStringList{QStringLiteral("kcm_mouse")});
}

void I18nTest::rightToLeftCatalogMirrorsLayout()
{
    auto *application = qApp;
    // No qtbase_ar translator in this process (LANGUAGE is pt_BR, en or unset):
    // Qt by itself keeps the layout left to right.
    QVERIFY(QCoreApplication::translate("QGuiApplication", "QT_LAYOUT_DIRECTION") != QStringLiteral("RTL"));
    QCOMPARE(QGuiApplication::layoutDirection(), Qt::LeftToRight);

    // The application's own Arabic catalog is used: mirror (same call as main()).
    KLocalizedString::setLanguages({QStringLiteral("ar"), QStringLiteral("en")});
    QVERIFY(KLocalizedString::isApplicationTranslatedInto(QStringLiteral("ar")));
    QCOMPARE(CatalogLayoutDirection::effectiveLanguage(), QStringLiteral("ar"));
    CatalogLayoutDirection::apply(*application);
    QCOMPARE(QGuiApplication::layoutDirection(), Qt::RightToLeft);
    AppWindow window;
    QVERIFY(window.isRightToLeft());

    // Yiddish is written right to left, but there is no painel-controle catalog
    // for it: the interface falls back to English and stays left to right.
    application->setLayoutDirection(Qt::LeftToRight);
    KLocalizedString::setLanguages({QStringLiteral("yi"), QStringLiteral("en")});
    QCOMPARE(QLocale(QStringLiteral("yi")).textDirection(), Qt::RightToLeft);
    QVERIFY(!KLocalizedString::isApplicationTranslatedInto(QStringLiteral("yi")));
    QCOMPARE(CatalogLayoutDirection::effectiveLanguage(), QString());
    CatalogLayoutDirection::apply(*application);
    QCOMPARE(QGuiApplication::layoutDirection(), Qt::LeftToRight);

    KLocalizedString::clearLanguages();
    application->setLayoutDirection(Qt::LayoutDirectionAuto);
}

int main(int argc, char *argv[])
{
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("painel-controle"));
    const QString localeDirectory = qEnvironmentVariable("PAINEL_CONTROLE_LOCALEDIR");
    if (!localeDirectory.isEmpty()) {
        KLocalizedString::addDomainLocaleDir(QByteArrayLiteral("painel-controle"), localeDirectory);
    }
    QApplication application(argc, argv);
    I18nTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "tst_i18n.moc"
