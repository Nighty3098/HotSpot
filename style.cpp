#include "mainwindow.h"
#include <QMainWindow>
#include <QLineEdit>
#include <QScrollBar>

void MainWindow::setStyle() {
    this->setStyleSheet("background-color: #0B0E14;");

    this->searchWidget->setStyleSheet("background-color: #0B0E14; border: none;");

    this->centralWidget->setStyleSheet("background-color: #0B0E14; color: #BFBDB6; padding: 0px; margin: 0px;");

    this->searchEdit->setStyleSheet(
        "QLineEdit {"
        "   background-color: #1F2430;"
        "   border: none;"
        "   border-radius: 15px;"
        "   color: #BFBDB6;"
        "   font-size: 0px;"
        "   padding: 10px 18px;"
        "   selection-background-color: #59C2FF;"
        "}"
        "QLineEdit:focus {"
        "   border: none;"
        "}"
        "QLineEdit::placeholder {"
        "   color: #707A8C;"
        "}"
        );

    this->appsList->setStyleSheet(
        "QListWidget {"
        "   background-color: #0B0E14;"
        "   border: none;"
        "   color: #BFBDB6;"
        "   font-size: 11px;"
        "   padding: 0px;"
        "   outline: none;"
        "}"
        "QListWidget::item {"
        "   color: #BFBDB6;"
        "   background-color: #0B0E14;"
        "   margin: 4px;"
        "   padding: 5px 10px;"
        "   min-height: 20px;"
        "   max-height: px;"
        "   border-radius: 12px;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: #59C2FF;"
        "   color: #0B0E14;"
        "   font-weight: bold;"
        "   border: 2px solid #59C2FF;"
        "}"
        );


    this->appsList->verticalScrollBar()->setStyleSheet(
        "QScrollBar:vertical {"
        "    background-color: transparent;"
        "    width: 8px;"
        "    margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background-color: #BFBDB6;"
        "    min-height: 20px;"
        "    border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background-color: #BFBDB6;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "    background: transparent;"
        "}"
        );
}
