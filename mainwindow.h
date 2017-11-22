#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <string.h>
#include <QMainWindow>
#include<WinSock2.h>
#include<Windows.h>

#include <QTextCodec>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTime>
#include <QSqlError>
#include <QtDebug>
#include <QSqlDriver>
#include <QSqlRecord>
#pragma comment(lib,"ws2_32.lib")
#define BUFLEN 50
#define MAXCONN 5
static DWORD WINAPI clientProc(LPARAM lparam);
static DWORD WINAPI AcceptProc(LPARAM lparam);
namespace Ui {
class MainWindow;
}
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    SOCKET  SerSocket;
    void initsock();
    void initDataBase();
    static int ConNum;
signals:
    void valuechanged(QString str);
private slots:
    void on_pushButton_clicked();
    void uichange(QString str);
private:
    Ui::MainWindow *ui;
    WSADATA wsaData;
    sockaddr_in srvAddr,clientAddr;
};
struct par
{
    MainWindow *m;
    SOCKET s;
};
struct par2
{
    MainWindow *m;
    SOCKET s;
    sockaddr_in clientAddr;
};
#endif // MAINWINDOW_H
