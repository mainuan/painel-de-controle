#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui {
    class AppWindow;
}
QT_END_NAMESPACE

class AppWindow : public QMainWindow {
    Q_OBJECT

public:
    AppWindow(QWidget *parent = nullptr);
    ~AppWindow();

private slots:
    void hotspotClick();
    void on_sidebar_itemActivated(QListWidgetItem *item);

    void on_tabWidget_currentChanged(int index);

    void on_callSystemSettings_clicked();

protected:
    // Virtual to allow interaction tests without launching external KCM processes.
    virtual void callKCMs(QString title, QString iconName, QStringList kcms);

private:
    Ui::AppWindow *ui;
    const int KCMS  = 42;
};
#endif // APPWINDOW_H
