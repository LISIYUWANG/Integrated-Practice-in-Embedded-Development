#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include <fstream>
#include <string>
#include <QDebug>
#include <QRadioButton>
#include <QButtonGroup>
#include <QTcpSocket>
#include <QtNetwork>
#include <QMessageBox>
using namespace std;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    void saveAd();
    ~MainWindow();
    
private slots:
    void on_refreshButton_clicked();

    void on_pushButton_clicked();

    void on_addAd_clicked();

    void on_pushButton_3_clicked();

    void on_runButton_clicked();

    void on_resetButton_clicked();

private:
    Ui::MainWindow *ui;
    QButtonGroup *speed;            //分组
    QButtonGroup *method;            //分组
};

#endif // MAINWINDOW_H
