#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QtDataVisualization>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using namespace QtDataVisualization;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_scan_clicked();
    void on_turnon_clicked();
    void on_save_clicked();
    void data_Receive();
    void on_language_clicked();  // 添加语言切换槽函数

private:
    Ui::MainWindow *ui;
    QSerialPort *serial = new QSerialPort(this);
    QByteArray buffer;

    // 三维图表相关成员变量
    Q3DScatter *scatterGraph;
    QWidget *container;

    // 数据存储
    QVector<QVector<int>> allData;

    // 压力值变量
    int P11, P12, P13, P14, P21, P22, P23, P24, P31, P32, P33, P34;

    // 语言状态变量
    bool isChinese = true;

    void serialSet();
    void setButtonColor(QPushButton* button, int value);
    int clampToZero(int value);

    // 初始化三维坐标系
    void init3DGraph();
    // 更新三维坐标系数据
    void update3DGraph();

    // 界面翻译函数
    void retranslateUI();
};

#endif // MAINWINDOW_H
