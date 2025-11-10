#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopServices>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDesktopWidget>
#include <QApplication>
#include <QListWidget>
#include <QShortcut>
#include <QLineEdit>
#include <QSettings>
#include <QDir>
#include <QProcess>
#include <QScrollBar>
#include <QKeyEvent>
#include <QIcon>
#include <QClipboard>
#include <QRegularExpression>
#include <cmath>
#include <algorithm>
#include <stack>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
#ifdef Q_OS_WIN
    setWindowFlag(Qt::WindowDoesNotShowInTaskbar);
#endif

    setFocusPolicy(Qt::StrongFocus);

    const int window_width = 650;
    const int window_height = 500;
    const int icon_size = 26;

    QRect screenGeometry = QApplication::desktop()->availableGeometry(this);
    setGeometry(screenGeometry.center().x() - window_width/2,
                screenGeometry.center().y() - window_height/2,
                window_width, window_height);

    setupShortcuts();
    setStyle();

    ui.searchEdit->installEventFilter(this);
    ui.appsList->installEventFilter(this);
    ui.searchEdit->setFocus();
    ui.appsList->setIconSize(QSize(icon_size, icon_size));


    allApps = getDesktopApplications();
    filterApps("");
}

MainWindow::~MainWindow() {}

void MainWindow::setupShortcuts()
{
    auto *closeShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(closeShortcut, &QShortcut::activated, this, &QWidget::close);

    auto *launchShortcut1 = new QShortcut(QKeySequence(Qt::Key_Return), this);
    auto *launchShortcut2 = new QShortcut(QKeySequence(Qt::Key_Enter), this);
    connect(launchShortcut1, &QShortcut::activated, this, [this]() {
        if (auto currentItem = ui.appsList->currentItem()) launchApp(currentItem);
    });
    connect(launchShortcut2, &QShortcut::activated, this, [this]() {
        if (auto currentItem = ui.appsList->currentItem()) launchApp(currentItem);
    });

    connect(ui.searchEdit, &QLineEdit::textChanged, this, &MainWindow::filterApps);
    connect(ui.appsList, &QListWidget::itemPressed, this, &MainWindow::launchApp);
}

void MainWindow::filterApps(const QString &searchText) {
    ui.appsList->clear();

    for (const ApplicationEntry &app : allApps) {
        if (searchText.isEmpty() ||
            app.name.contains(searchText, Qt::CaseInsensitive) ||
            app.about.contains(searchText, Qt::CaseInsensitive)) {
            ui.appsList->addItem(createAppItem(app));
        }
    }

    if (ui.appsList->count() > 0) {
        ui.appsList->setCurrentRow(0);
    }
}

QList<ApplicationEntry> MainWindow::getDesktopApplications() {
    QList<ApplicationEntry> apps;

    QSet<QString> uniqueNames;
    QSet<QString> uniqueExecs;

    QStringList paths = {
        "/usr/share/applications",
        "/usr/local/share/applications",
        QDir::homePath() + "/.local/share/applications"
    };

    for (const QString &path : paths) {
        QDir dir(path);
        QStringList desktopFiles = dir.entryList(QStringList() << "*.desktop", QDir::Files);

        for (const QString &fileName : desktopFiles) {
            QString filePath = dir.absoluteFilePath(fileName);

            QSettings desktopFile(filePath, QSettings::IniFormat);
            desktopFile.beginGroup("Desktop Entry");

            if (desktopFile.value("NoDisplay").toBool() ||
                desktopFile.value("Hidden").toBool()) {
                desktopFile.endGroup();
                continue;
            }

            QString name = desktopFile.value("Name").toString();
            QString exec = desktopFile.value("Exec").toString();
            QString icon = desktopFile.value("Icon").toString();
            QString about = desktopFile.value("Comment").toString();

            if (!name.isEmpty() && !exec.isEmpty()) {
                if (!uniqueNames.contains(name) && !uniqueExecs.contains(exec)) {
                    uniqueNames.insert(name);
                    uniqueExecs.insert(exec);
                    apps.append({name, exec, icon, about});
                }
            }
            desktopFile.endGroup();
        }
    }

    std::sort(apps.begin(), apps.end(), [](const ApplicationEntry &a, const ApplicationEntry &b) {
        return a.name.toLower() < b.name.toLower();
    });

    return apps;
}

QListWidgetItem* MainWindow::createAppItem(const ApplicationEntry &app) {
    QListWidgetItem *item = new QListWidgetItem();

    item->setData(Qt::DisplayRole, app.name);
    item->setData(Qt::UserRole, app.about);

    QIcon icon = QIcon::fromTheme(app.icon);
    if (icon.isNull()) {
        icon = QIcon(app.icon);
    }
    item->setIcon(icon);
    item->setData(Qt::UserRole + 1, app.exec);

    return item;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui.searchEdit) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Down || keyEvent->key() == Qt::Key_Up) {
                ui.appsList->setFocus();
                QKeyEvent *newEvent = new QKeyEvent(keyEvent->type(), keyEvent->key(), keyEvent->modifiers());
                QApplication::postEvent(ui.appsList, newEvent);
                return true;
            }
        }
    } else if (obj == ui.appsList) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (!keyEvent->text().isEmpty() && !keyEvent->text().at(0).isSpace()) {
                ui.searchEdit->setFocus();
                QApplication::sendEvent(ui.searchEdit, event);
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::launchApp(QListWidgetItem *item) {
    if (!item) return;

    QString execCommand = item->data(Qt::UserRole + 1).toString();
    if (execCommand.isEmpty()) return;

    QStringList execParts = execCommand.split(' ', Qt::SkipEmptyParts);
    if (execParts.isEmpty()) return;

    QString program = execParts.takeFirst();

    QStringList arguments;
    for (const QString &arg : execParts) {
        if (!arg.startsWith('%')) {
            arguments << arg;
        }
    }

    bool started = QProcess::startDetached(program, arguments);
    if (!started) {
        qWarning("Failed to launch application: %s", qPrintable(execCommand));
    } else {
        close();
    }
}

void MainWindow::setStyle() {
    ui.centralWidget->setStyleSheet("background-color: #282828;");

    ui.centralWidget->setStyleSheet("background-color: #282828; color: #f9f5d7; padding: 0px; margin: 0px;");

    ui.searchEdit->setStyleSheet(
        "QLineEdit {"
        "   background-color: #282828;"
        "   border: none;"
        "   border-radius: 15px;"
        "   color: #f9f5d7;"
        "   font-size: 12px;"
        "   padding: 10px 18px;"
        "   min-height: 30px;"
        "   max-height: 30px;"
        "   selection-background-color: #282828;"
        "}"
        "QLineEdit:focus {"
        "   border: none;"
        "}"
        "QLineEdit::placeholder {"
        "   color: #707A8C;"
        "}"
        );

    ui.appsList->setStyleSheet(
        "QListWidget {"
        "   background-color: #282828;"
        "   border: none;"
        "   color: #f9f5d7;"
        "   font-size: 12px;"
        "   padding: 0px;"
        "   outline: none;"
        "}"
        "QListWidget::item {"
        "   color: #f9f5d7;"
        "   background-color: #282828;"
        "   margin: 4px;"
        "   font-size: 12px;"
        "   padding: 5px 10px;"
        "   min-height: 30px;"
        "   max-height: 30px;"
        "   border-radius: 12px;"
        "   border: 0px solid #282828;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: #32302f;"
        "   color: #f9f5d7;"
        "   font-weight: bold;"
        "   border: none;"
        "   border: 0px solid #a89984;"
        "}"
        );

    ui.appsList->verticalScrollBar()->setStyleSheet(
        "QScrollBar:vertical {"
        "    background-color: transparent;"
        "    width: 8px;"
        "    margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background-color: #32302f;"
        "    min-height: 20px;"
        "    border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background-color: #32302f;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "    background: transparent;"
        "}"
        );
}
