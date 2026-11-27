// Proves that the real AppWindow (appwindow.ui compiled with "uic -tr tr2i18n")
// is translated through KI18n: LANGUAGE=pt_BR shows the original Portuguese
// labels, LANGUAGE=en or the C locale shows the English source strings, and the
// kcm_* identifiers handed to kcmshell6 never change. Run by run-i18n-tests.sh,
// which sets PAINEL_CONTROLE_EXPECTED_LANGUAGE and PAINEL_CONTROLE_LOCALEDIR.

#include "appwindow.h"
#include "cataloglayoutdirection.h"
#include "systeminfo.h"

#include <KLocalizedString>

#include <QApplication>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QTabBar>
#include <QTabWidget>
#include <QTest>

#include <clocale>

class TestableAppWindow final : public AppWindow
{
public:
    using AppWindow::AppWindow;

    int activationCount = 0;
    QString activatedTitle;
    QStringList activatedKcms;

protected:
    void callKCMs(QString title, QString, QStringList kcms) override
    {
        ++activationCount;
        activatedTitle = title;
        activatedKcms = kcms;
    }
};

class I18nTest final : public QObject
{
    Q_OBJECT

private slots:
    void windowUsesExpectedLanguage();
    void chassisDetectionRecognizesPortableTypes();
    void notebookDisplayShortcutIsAvailable();
    void sidebarUsesTraditionalSetupOrder();
    void sidebarSupportsKeyboardActivation();
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
    auto *tabBar = tabs->findChild<QTabBar *>();
    QVERIFY(tabBar);
    QVERIFY(tabBar->isHidden());

    const int expectedTab = SystemInfo::detectDeviceType() == SystemInfo::DeviceType::Laptop ? 1 : 0;
    QCOMPARE(tabs->currentIndex(), expectedTab);
    auto *heading = window.findChild<QLabel *>(QStringLiteral("heading"));
    QVERIFY(heading);
    QCOMPARE(heading->text(), tabs->tabText(expectedTab));
    // The sidebar is built from the hotspot buttons of the detected device.
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

void I18nTest::chassisDetectionRecognizesPortableTypes()
{
    for (const int type : {8, 9, 10, 11, 14, 30, 31, 32}) {
        QVERIFY2(SystemInfo::isLaptopChassisType(type), qPrintable(QString::number(type)));
    }

    for (const int type : {3, 4, 5, 6, 7, 13, 35, 36}) {
        QVERIFY2(!SystemInfo::isLaptopChassisType(type), qPrintable(QString::number(type)));
    }
}

void I18nTest::notebookDisplayShortcutIsAvailable()
{
    AppWindow window;
    auto *tabs = window.findChild<QTabWidget *>(QStringLiteral("tabWidget"));
    QVERIFY(tabs);
    tabs->setCurrentIndex(1);

    auto *notebook = window.findChild<QWidget *>(QStringLiteral("tabNotebook"));
    QVERIFY(notebook);
    auto *displayButton = notebook->findChild<QPushButton *>(QStringLiteral("pushButton"));
    QVERIFY(displayButton);
    const QStringList displayKcms = displayButton->property("kcms").toStringList();
    QVERIFY(displayKcms.contains(QStringLiteral("kcm_kscreen")));

    auto *sidebar = window.findChild<QListWidget *>(QStringLiteral("sidebar"));
    QVERIFY(sidebar);
    QVERIFY(sidebar->count() > 0);

    const bool pt = qEnvironmentVariable("PAINEL_CONTROLE_EXPECTED_LANGUAGE") == QLatin1String("pt_BR");
    QCOMPARE(sidebar->item(0)->text(), pt ? QStringLiteral("Tela") : QStringLiteral("Display"));
    QVERIFY(sidebar->item(0)->data(42).toStringList().contains(QStringLiteral("kcm_kscreen")));
}

void I18nTest::sidebarUsesTraditionalSetupOrder()
{
    AppWindow window;
    auto *tabs = window.findChild<QTabWidget *>(QStringLiteral("tabWidget"));
    auto *sidebar = window.findChild<QListWidget *>(QStringLiteral("sidebar"));
    QVERIFY(tabs);
    QVERIFY(sidebar);

    const QStringList expectedKcms = {
        QStringLiteral("kcm_kscreen"),
        QStringLiteral("kcm_pulseaudio"),
        QStringLiteral("kcm_networkmanagement"),
        QStringLiteral("kcm_keyboard"),
        QStringLiteral("kcm_printer_manager"),
        QStringLiteral("kcm_bluetooth")
    };

    // Verify both device maps; the ordering is based on stable KCM IDs,
    // so it must be identical in every translated session.
    for (int tabIndex = 0; tabIndex < tabs->count(); ++tabIndex) {
        tabs->setCurrentIndex(tabIndex);
        QVERIFY2(sidebar->count() >= expectedKcms.size(),
                 qPrintable(QStringLiteral("Not enough sidebar entries on tab %1").arg(tabIndex)));

        for (qsizetype row = 0; row < expectedKcms.size(); ++row) {
            const QStringList kcms = sidebar->item(static_cast<int>(row))->data(42).toStringList();
            QVERIFY2(kcms.contains(expectedKcms.at(row)),
                     qPrintable(QStringLiteral("Unexpected sidebar item at row %1 on tab %2")
                                    .arg(row).arg(tabIndex)));
        }
    }
}

void I18nTest::sidebarSupportsKeyboardActivation()
{
    TestableAppWindow window;
    window.show();

    auto *sidebar = window.findChild<QListWidget *>(QStringLiteral("sidebar"));
    QVERIFY(sidebar);
    QVERIFY(sidebar->count() > 0);

    QListWidgetItem *item = sidebar->item(0);
    QVERIFY(item);
    sidebar->setCurrentItem(item);
    sidebar->setFocus(Qt::OtherFocusReason);

    // Both the main Enter key and the numeric keypad Enter must activate the
    // selected item through QListWidget::itemActivated.
    QTest::keyClick(sidebar, Qt::Key_Return);
    QCOMPARE(window.activationCount, 1);
    QCOMPARE(window.activatedTitle, item->text());
    QCOMPARE(window.activatedKcms, item->data(42).toStringList());

    QTest::keyClick(sidebar, Qt::Key_Enter);
    QCOMPARE(window.activationCount, 2);
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
