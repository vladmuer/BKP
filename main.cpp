#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Fusion"));
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, QColor(230, 230, 230));
    darkPalette.setColor(QPalette::Base, QColor(33, 33, 33));
    darkPalette.setColor(QPalette::Text, QColor(230, 230, 230));
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, QColor(230, 230, 230));
    darkPalette.setColor(QPalette::Highlight, QColor(230, 230, 230));
    darkPalette.setColor(QPalette::HighlightedText, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::PlaceholderText, QColor(230, 230, 230));
    a.setPalette(darkPalette);
    MainWindow w;
    w.show();
    return a.exec();
}
