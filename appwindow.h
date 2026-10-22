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
    void on_sidebar_itemPressed(QListWidgetItem *item);

    void on_tabWidget_currentChanged(int index);

private:
    Ui::AppWindow *ui;
    void callKCMs(QString title, QString iconName, QStringList kcms);

    const int KCMS  = 42;
};
#endif // APPWINDOW_H
