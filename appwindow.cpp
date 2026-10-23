#include "appwindow.h"
#include "ui_appwindow.h"
#include <QProcess>

AppWindow::AppWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::AppWindow){
    ui->setupUi(this);

    for (QPushButton* button : ui->tabWidget->findChildren<QPushButton*>()) {
        connect(button, &QPushButton::clicked,this,&AppWindow::hotspotClick,Qt::UniqueConnection);
        button->setStyleSheet(
            "QPushButton{"
            "  border: 0px;"
            "  color: transparent;"
            "  background-color: transparent;"
            "}"
            "QPushButton:hover {"
            "  background-color: rgba(85, 170, 255, 70);"
            "}"
            );
        button->setIconSize(QSize(0,0));
    }
    ui->tabWidget->setCurrentIndex(0);
    on_tabWidget_currentChanged(0);
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

    callKCMs(sender->text(),sender->icon().name(),sender->property("kcms").toStringList());
}

void AppWindow::on_sidebar_itemPressed(QListWidgetItem *item) {
    callKCMs(item->text(),item->icon().name(),item->data(KCMS).toStringList());
}

void AppWindow::on_tabWidget_currentChanged(int index) {
    QWidget *tab = index == 0 ? ui->tabDesktop : ui->tabNotebook;

    ui->sidebar->clear();
    for (QPushButton* button : tab->findChildren<QPushButton*>()) {
        QListWidgetItem *item = new QListWidgetItem(button->icon(),button->text(),ui->sidebar);
        item->setSizeHint(QSize(48,48));
        item->setData(KCMS,button->property("kcms").toStringList());
    }
    ui->sidebar->setMinimumHeight(ui->sidebar->count()*48+4);

    if (ui->sidebar->minimumHeight() > 506) {
        ui->sidebar->setMinimumHeight(506);
    }

    ui->sidebar->sortItems();
}


void AppWindow::on_callSystemSettings_clicked() {
    this->hide();

    QProcess p;
    p.waitForFinished(-1);
    p.execute("systemsettings",QStringList());

    this->show();
}

