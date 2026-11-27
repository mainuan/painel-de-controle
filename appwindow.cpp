#include "appwindow.h"
// ui_appwindow.h is generated with "uic -tr tr2i18n": KLocalizedString first.
#include <KLocalizedString>
#include "ui_appwindow.h"
#include "systeminfo.h"
#include <QProcess>
#include <QFont>
#include <QFrame>
#include <QAbstractItemView>
#include <QTabBar>
#include <algorithm>

static QString withoutMnemonic(const QString &text);

AppWindow::AppWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::AppWindow){
    ui->setupUi(this);

    // Follow the active KDE/Qt theme instead of imposing a separate palette.
    ui->sidebar->setIconSize(QSize(22, 22));
    ui->sidebar->setSpacing(2);
    ui->sidebar->setUniformItemSizes(true);
    ui->sidebar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->sidebar->setFrameShape(QFrame::NoFrame);
    ui->sidebar->setSelectionMode(QAbstractItemView::SingleSelection);

    QFont headingFont = ui->heading->font();
    headingFont.setBold(true);
    if (headingFont.pointSize() > 0) {
        headingFont.setPointSize(headingFont.pointSize() + 1);
    }
    ui->heading->setFont(headingFont);

    for (QPushButton* button : ui->tabWidget->findChildren<QPushButton*>()) {
        connect(button, &QPushButton::clicked,this,&AppWindow::hotspotClick,Qt::UniqueConnection);
        button->setFlat(true);
        button->setFocusPolicy(Qt::StrongFocus);
        button->setToolTip(withoutMnemonic(button->text()));
        button->setStyleSheet(
            "QPushButton { border: 1px solid transparent; border-radius: 7px; "
            "color: transparent; background: transparent; padding: 0; }"
            "QPushButton:hover, QPushButton:focus { "
            "border: 2px solid palette(highlight); background: transparent; }"
            "QPushButton:pressed { background: palette(alternate-base); }"
        );
        button->setIconSize(QSize(0, 0));
        // Keep all transparent hotspots above the device illustration.
        button->raise();
    }

    // The device view is selected from SMBIOS chassis information, with a
    // battery-presence fallback on hardware whose firmware omits that field.
    if (auto *tabBar = ui->tabWidget->findChild<QTabBar *>()) {
        tabBar->hide();
    }
    const int deviceIndex = SystemInfo::detectDeviceType() == SystemInfo::DeviceType::Laptop ? 1 : 0;
    if (ui->tabWidget->currentIndex() == deviceIndex) {
        // setCurrentIndex() emits currentChanged only when the index changes.
        // Initialize the default desktop page explicitly in that case.
        on_tabWidget_currentChanged(deviceIndex);
    } else {
        // The connected currentChanged slot initializes the newly selected page.
        ui->tabWidget->setCurrentIndex(deviceIndex);
    }
}

// Button labels carry Qt mnemonics: "&&" is a literal ampersand and a single
// "&" marks the shortcut letter. List items and window captions show text
// verbatim, so they need the plain form ("Date && Time" -> "Date & Time").
static QString withoutMnemonic(const QString &text) {
    QString plain;
    plain.reserve(text.size());
    for (qsizetype i = 0; i < text.size(); ++i) {
        if (text.at(i) == QLatin1Char('&')) {
            if (i + 1 < text.size() && text.at(i + 1) == QLatin1Char('&')) {
                plain += QLatin1Char('&');
                ++i;
            }
            continue;
        }
        plain += text.at(i);
    }
    return plain;
}

AppWindow::~AppWindow(){
    delete ui;
}

void AppWindow::callKCMs(QString title, QString iconName, QStringList kcms) {
    kcms.prepend(title);
    kcms.prepend("--caption");

    kcms.prepend(iconName);
    kcms.prepend("--icon");

    QProcess p;
    p.waitForFinished(-1);

    this->hide();

    p.execute(
        kcms.last() == "guvcview" ? "guvcview" :"kcmshell6",
        kcms.last() == "guvcview" ? QStringList() : kcms
    );

    this->show();
    ui->sidebar->clearSelection();
    ui->sidebar->clearFocus();
}

void AppWindow::hotspotClick() {
    QPushButton *sender = qobject_cast<QPushButton*>(QObject::sender());

    callKCMs(withoutMnemonic(sender->text()),sender->icon().name(),sender->property("kcms").toStringList());
}

void AppWindow::on_sidebar_itemActivated(QListWidgetItem *item) {
    callKCMs(item->text(),item->icon().name(),item->data(KCMS).toStringList());
}

void AppWindow::on_tabWidget_currentChanged(int index) {
    QWidget *tab = index == 0 ? ui->tabDesktop : ui->tabNotebook;

    ui->sidebar->clear();
    const auto buttons = tab->findChildren<QPushButton*>();
    for (QPushButton* button : buttons) {
        QListWidgetItem *item = new QListWidgetItem(button->icon(), withoutMnemonic(button->text()));
        item->setSizeHint(QSize(0, 40));
        const QStringList kcms = button->property("kcms").toStringList();
        item->setData(KCMS, kcms);
        item->setToolTip(withoutMnemonic(button->text()));
        ui->sidebar->addItem(item);

    }

    // Start with an alphabetical order, then stably promote the six most
    // commonly needed setup categories. Use KCM IDs, never translated labels.
    ui->sidebar->sortItems();

    const auto priority = [this](const QListWidgetItem *item) {
        const QStringList kcms = item->data(KCMS).toStringList();
        if (kcms.contains(QStringLiteral("kcm_kscreen"))) {
            return 0; // Display / monitor
        }
        if (kcms.contains(QStringLiteral("kcm_pulseaudio"))) {
            return 1; // Microphone / speakers
        }
        if (kcms.contains(QStringLiteral("kcm_networkmanagement"))) {
            return 2; // Network / Wi-Fi / Internet
        }
        if (kcms.contains(QStringLiteral("kcm_keyboard"))) {
            return 3; // Keyboard
        }
        if (kcms.contains(QStringLiteral("kcm_printer_manager"))) {
            return 4; // Printers
        }
        if (kcms.contains(QStringLiteral("kcm_bluetooth"))) {
            return 5; // Bluetooth
        }
        return 6; // Other settings retain their alphabetical order.
    };

    QList<QListWidgetItem *> orderedItems;
    orderedItems.reserve(ui->sidebar->count());
    while (ui->sidebar->count() > 0) {
        orderedItems.append(ui->sidebar->takeItem(0));
    }
    std::stable_sort(orderedItems.begin(), orderedItems.end(),
                     [&priority](const QListWidgetItem *left, const QListWidgetItem *right) {
        return priority(left) < priority(right);
    });
    for (QListWidgetItem *item : orderedItems) {
        ui->sidebar->addItem(item);
    }

    ui->heading->setText(ui->tabWidget->tabText(index));
    ui->sidebar->setMinimumHeight(0);
}


void AppWindow::on_callSystemSettings_clicked() {
    this->hide();

    QProcess p;
    p.waitForFinished(-1);
    p.execute("systemsettings",QStringList());

    this->show();
}
