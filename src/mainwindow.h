#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QList>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct ApplicationEntry {
    QString name;
    QString exec;
    QString icon;
    QString about;
};

struct ThemeEntry {
    QString name;
    QString displayName;
    QString path;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void filterApps(const QString &searchText);
    void launchApp(QListWidgetItem *item);
    void switchToThemeSelection();
    void switchToAppSelection();
    void applyTheme(const QString &themeName);
    void onItemClicked(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    QSettings *globalSettings;
    QList<ApplicationEntry> allApps;
    QList<ThemeEntry> availableThemes;
    bool inThemeSelectionMode = false;

    void setupShortcuts();
    void setupThemeSystem();
    void loadAvailableThemes();
    void showThemeSelection();
    void showAppSelection();
    QList<ApplicationEntry> getDesktopApplications();
    QListWidgetItem* createAppItem(const ApplicationEntry &app);
    QListWidgetItem* createThemeItem(const ThemeEntry &theme);
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // MAINWINDOW_H
