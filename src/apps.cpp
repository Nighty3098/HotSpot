#include "mainwindow.h"
#include <QSettings>
#include <QDir>
#include <QProcess>
#include <QStyledItemDelegate>
#include <Qt>

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

void MainWindow::launchApp(QListWidgetItem *item) {
    QString exec = item->data(Qt::UserRole+1).toString();
    if (!exec.isEmpty()) {
        QProcess::startDetached(exec.split(' ').first());
        this->close();
    }
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
