#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include <QMainWindow>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>

struct ApplicationEntry {
    QString name;
    QString exec;
    QString icon;
    QString about;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void launchApp(QListWidgetItem *item);
    void filterApps(const QString &searchText);

private:
    Ui::MainWindow ui;

    void setupShortcuts();
    void setStyle();
    QList<ApplicationEntry> getDesktopApplications();
    QListWidgetItem* createAppItem(const ApplicationEntry &app);
    QListWidgetItem* createCalculatorItem(const QString &expression, const QString &result);

    // Data
    QList<ApplicationEntry> allApps;

    QStringList calculationHistory;
    void addToHistory(const QString &expression, const QString &result);
};

#endif // MAINWINDOW_H
