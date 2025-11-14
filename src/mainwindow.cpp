#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QProcess>
#include <QScreen>
#include <QScrollBar>
#include <QSettings>
#include <QShortcut>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  globalSettings = new QSettings("Nighty3098", "HotSpot", this);

  setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
#ifdef Q_OS_WIN
  setWindowFlag(Qt::WindowDoesNotShowInTaskbar);
#endif

  setFocusPolicy(Qt::StrongFocus);

  const int window_width = 650;
  const int window_height = 500;
  const int icon_size = 26;

  QScreen *screen = QGuiApplication::primaryScreen();
  QRect screenGeometry = screen->availableGeometry();

  setGeometry(screenGeometry.center().x() - window_width / 2,
              screenGeometry.center().y() - window_height / 2, window_width,
              window_height);

  setupShortcuts();
  setupThemeSystem();

  ui->searchEdit->installEventFilter(this);
  ui->appsList->installEventFilter(this);
  ui->searchEdit->setFocus();
  ui->appsList->setIconSize(QSize(icon_size, icon_size));

  connect(ui->appsList, &QListWidget::itemClicked, this,
          &MainWindow::onItemClicked);

  allApps = getDesktopApplications();

  ApplicationEntry themeSelector;
  themeSelector.name = "HotSpot: Select Theme";
  themeSelector.exec = "theme_selector";
  themeSelector.icon = "preferences-desktop-theme";
  themeSelector.about = "Change application theme";
  allApps.prepend(themeSelector);

  filterApps("");
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::setupShortcuts() {
  auto *closeShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
  connect(closeShortcut, &QShortcut::activated, this, [this]() {
    if (inThemeSelectionMode) {
      switchToAppSelection();
    } else {
      close();
    }
  });

  auto *launchShortcut1 = new QShortcut(QKeySequence(Qt::Key_Return), this);
  auto *launchShortcut2 = new QShortcut(QKeySequence(Qt::Key_Enter), this);
  connect(launchShortcut1, &QShortcut::activated, this, [this]() {
    if (auto currentItem = ui->appsList->currentItem()) {
      onItemClicked(currentItem);
    }
  });
  connect(launchShortcut2, &QShortcut::activated, this, [this]() {
    if (auto currentItem = ui->appsList->currentItem()) {
      onItemClicked(currentItem);
    }
  });

  connect(ui->searchEdit, &QLineEdit::textChanged, this,
          &MainWindow::filterApps);
}

void MainWindow::setupThemeSystem() {
  loadAvailableThemes();

  QString savedTheme =
      globalSettings->value("theme/current", "gruvbox_dark").toString();
  applyTheme(savedTheme);
}

void MainWindow::loadAvailableThemes() {
  availableThemes.clear();

  QDir resourceDir(":/themes/");
  QStringList themeFiles =
      resourceDir.entryList(QStringList() << "*.qss", QDir::Files);

  for (const QString &themeFile : themeFiles) {
    ThemeEntry theme;
    theme.name = QFileInfo(themeFile).baseName();
    theme.displayName = theme.name.replace('_', ' ').replace("qss", "");
    theme.path = ":/themes/" + themeFile;
    availableThemes.append(theme);
    qDebug() << "Found theme:" << theme.name << "at" << theme.path;
  }

  QDir fsDir("themes/");
  if (fsDir.exists()) {
    QStringList fsThemeFiles =
        fsDir.entryList(QStringList() << "*.qss", QDir::Files);
    for (const QString &themeFile : fsThemeFiles) {
      ThemeEntry theme;
      theme.name = QFileInfo(themeFile).baseName();
      theme.displayName = theme.name.replace('_', ' ').replace("qss", "");
      theme.path = fsDir.absoluteFilePath(themeFile);
      availableThemes.append(theme);
      qDebug() << "Found filesystem theme:" << theme.name << "at" << theme.path;
    }
  }

  if (availableThemes.isEmpty()) {
    qDebug() << "No themes found!";
  }
}

void MainWindow::applyTheme(const QString &themeName) {
  ThemeEntry selectedTheme;
  bool themeFound = false;

  for (const ThemeEntry &theme : availableThemes) {
    if (theme.name == themeName) {
      selectedTheme = theme;
      themeFound = true;
      break;
    }
  }

  if (!themeFound && !availableThemes.isEmpty()) {
    selectedTheme = availableThemes.first();
    themeFound = true;
  }

  if (!themeFound) {
    qDebug() << "No themes available to apply";
    return;
  }

  QFile file(selectedTheme.path);

  if (file.open(QFile::ReadOnly)) {
    QString styleSheet = QString::fromUtf8(file.readAll());
    qApp->setStyleSheet(styleSheet);
    file.close();

    globalSettings->setValue("theme/current", selectedTheme.name);
    globalSettings->sync();

    qDebug() << "Theme applied:" << selectedTheme.name;

    if (inThemeSelectionMode) {
      showThemeSelection();
    }
  } else {
    qDebug() << "Failed to load theme file:" << selectedTheme.path
             << file.errorString();
  }
}

void MainWindow::onItemClicked(QListWidgetItem *item) {
  if (!item)
    return;

  if (inThemeSelectionMode) {
    QString themeName = item->data(Qt::UserRole + 1).toString();
    applyTheme(themeName);
    switchToAppSelection();
  } else {
    QString execCommand = item->data(Qt::UserRole + 1).toString();

    if (execCommand == "theme_selector") {
      switchToThemeSelection();
    } else {
      launchApp(item);
    }
  }
}

void MainWindow::switchToThemeSelection() {
  inThemeSelectionMode = true;
  showThemeSelection();
}

void MainWindow::switchToAppSelection() {
  inThemeSelectionMode = false;
  showAppSelection();
}

void MainWindow::showThemeSelection() {
  ui->appsList->clear();
  ui->searchEdit->setPlaceholderText("Search themes...");
  ui->searchEdit->clear();

  QString currentTheme =
      globalSettings->value("theme/current", "gruvbox_dark").toString();

  for (const ThemeEntry &theme : availableThemes) {
    QListWidgetItem *item = createThemeItem(theme);

    if (theme.name == currentTheme) {
      item->setText("✓ " + item->text());
    }

    ui->appsList->addItem(item);
  }

  if (ui->appsList->count() > 0) {
    ui->appsList->setCurrentRow(0);
  }
}

void MainWindow::showAppSelection() {
  ui->appsList->clear();
  ui->searchEdit->setPlaceholderText("Search applications...");
  ui->searchEdit->clear();
  filterApps("");
}

void MainWindow::filterApps(const QString &searchText) {
  if (inThemeSelectionMode) {
    ui->appsList->clear();

    QString currentTheme =
        globalSettings->value("theme/current", "gruvbox_dark").toString();

    for (const ThemeEntry &theme : availableThemes) {
      if (searchText.isEmpty() ||
          theme.displayName.contains(searchText, Qt::CaseInsensitive) ||
          theme.name.contains(searchText, Qt::CaseInsensitive)) {

        QListWidgetItem *item = createThemeItem(theme);

        if (theme.name == currentTheme) {
          item->setText("✓ " + item->text());
        }

        ui->appsList->addItem(item);
      }
    }
  } else {
    ui->appsList->clear();

    for (const ApplicationEntry &app : allApps) {
      if (searchText.isEmpty() ||
          app.name.contains(searchText, Qt::CaseInsensitive) ||
          app.about.contains(searchText, Qt::CaseInsensitive)) {
        ui->appsList->addItem(createAppItem(app));
      }
    }
  }

  if (ui->appsList->count() > 0) {
    ui->appsList->setCurrentRow(0);
  }
}

QList<ApplicationEntry> MainWindow::getDesktopApplications() {
  QList<ApplicationEntry> apps;

  QSet<QString> uniqueNames;
  QSet<QString> uniqueExecs;

  QStringList paths = {"/usr/share/applications",
                       "/usr/local/share/applications",
                       QDir::homePath() + "/.local/share/applications"};

  for (const QString &path : paths) {
    QDir dir(path);
    QStringList desktopFiles =
        dir.entryList(QStringList() << "*.desktop", QDir::Files);

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

  std::sort(apps.begin(), apps.end(),
            [](const ApplicationEntry &a, const ApplicationEntry &b) {
              return a.name.toLower() < b.name.toLower();
            });

  return apps;
}

QListWidgetItem *MainWindow::createAppItem(const ApplicationEntry &app) {
  QListWidgetItem *item = new QListWidgetItem();

  item->setText(app.name);
  item->setData(Qt::UserRole, app.about);
  item->setData(Qt::UserRole + 1, app.exec);

  QIcon icon = QIcon::fromTheme(app.icon);
  if (icon.isNull()) {
    icon = QIcon(app.icon);
  }
  item->setIcon(icon);

  return item;
}

QListWidgetItem *MainWindow::createThemeItem(const ThemeEntry &theme) {
  QListWidgetItem *item = new QListWidgetItem();

  item->setText(theme.displayName);
  item->setData(Qt::UserRole, "Theme: " + theme.name);
  item->setData(Qt::UserRole + 1, theme.name);

  QIcon icon = QIcon::fromTheme("preferences-desktop-theme");
  if (icon.isNull()) {
    icon = QIcon::fromTheme("preferences-color");
  }
  item->setIcon(icon);

  return item;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
  if (obj == ui->searchEdit) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
      if (keyEvent->key() == Qt::Key_Down || keyEvent->key() == Qt::Key_Up) {
        ui->appsList->setFocus();
        QKeyEvent *newEvent = new QKeyEvent(keyEvent->type(), keyEvent->key(),
                                            keyEvent->modifiers());
        QApplication::postEvent(ui->appsList, newEvent);
        return true;
      } else if (keyEvent->key() == Qt::Key_Escape && inThemeSelectionMode) {
        switchToAppSelection();
        return true;
      }
    }
  } else if (obj == ui->appsList) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
      if (!keyEvent->text().isEmpty() && !keyEvent->text().at(0).isSpace()) {
        ui->searchEdit->setFocus();
        QApplication::sendEvent(ui->searchEdit, event);
        return true;
      }
    }
  }
  return QMainWindow::eventFilter(obj, event);
}

void MainWindow::launchApp(QListWidgetItem *item) {
  if (!item)
    return;

  QString execCommand = item->data(Qt::UserRole + 1).toString();
  if (execCommand.isEmpty() || execCommand == "theme_selector")
    return;

  QStringList execParts = execCommand.split(' ', Qt::SkipEmptyParts);
  if (execParts.isEmpty())
    return;

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
