#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QProcess>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    //void paintEvent(QPaintEvent* event);//override
public slots:
    void update();
    void processupdate();
    void processupdate1();
    void cpucalc();
    void memgraph();
    void netgraph();
private slots:
    //void on_tabWidget_tabBarClicked(int index);

    void on_tabWidget_currentChanged(int index);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

private:
    Ui::MainWindow *ui;
};

class Cpuwd: public QWidget
{
    Q_OBJECT
public:
    explicit Cpuwd(QWidget *parent = 0);
    ~Cpuwd();
    //void draw();
protected:
    void paintEvent(QPaintEvent*);

};

class Netwd: public QWidget
{
    Q_OBJECT
public:
    explicit Netwd(QWidget *parent = 0);
    ~Netwd();
    //void draw();
protected:
    void paintEvent(QPaintEvent*);

};

#endif // MAINWINDOW_H
