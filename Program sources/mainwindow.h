#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <fstream>

using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    bool TRIMstatus();

    string SelectedMethod();

    ULONGLONG *GetClusters(PCHAR lpFileName, ULONG ClusterSize, ULONG *ClCount, ULONG *FileSize);

    BOOL DelFile(PCHAR lpSrcName, string Type);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
