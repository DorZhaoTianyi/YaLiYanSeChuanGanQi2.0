#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <Q3DScatter>
#include <QScatter3DSeries>
#include <Q3DTheme>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , allData()
{
    ui->setupUi(this);
    ui->radioButton->setEnabled(false);
    connect(serial, &QSerialPort::readyRead, this, &MainWindow::data_Receive);
    buffer.clear();

    // 设置深蓝色背景
    this->setStyleSheet("background-color: #001a33;");
    ui->centralwidget->setStyleSheet("background-color: #001a33;");

    // 设置字体样式
    QFont font;
    font.setPointSize(15);
    font.setBold(true);

    // 应用字体到所有控件
    QList<QWidget*> widgets = {
        ui->name, ui->botelu, ui->tingzhiwei, ui->shujuwei, ui->jiaoyanwei,ui->save,
        ui->turnon, ui->radioButton, ui->scan, ui->M11, ui->M12, ui->M13, ui->M14,
        ui->M21, ui->M22, ui->M23, ui->M24, ui->M31, ui->M32, ui->M33, ui->M34,
        ui->label, ui->label_2, ui->label_3, ui->label_4, ui->label_5, ui->language,
    };

    // 设置字体、颜色和居中对齐
    for (auto widget : widgets) {
        widget->setFont(font);
        widget->setStyleSheet("color: white;");

        // 对QLabel控件设置居中对齐
        QLabel* label = qobject_cast<QLabel*>(widget);
        if (label) {
            label->setAlignment(Qt::AlignCenter);
        }

        // 对QPushButton控件设置居中对齐
        QPushButton* button = qobject_cast<QPushButton*>(widget);
        if (button) {
            button->setStyleSheet(button->styleSheet() + "text-align: center;");
        }

        // 对QComboBox控件设置居中对齐
        QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
        if (comboBox) {
            comboBox->setStyleSheet("QComboBox { color: white; text-align: center; }"
                                  "QComboBox QAbstractItemView { color: white; text-align: center; }");
        }
    }

    // 单独设置标签控件的居中对齐（确保所有label都居中）
    QList<QLabel*> labels = {
        ui->label, ui->label_2, ui->label_3, ui->label_4, ui->label_5
    };

    for (auto label : labels) {
        label->setAlignment(Qt::AlignCenter);
    }

    // 初始化三维坐标系
    init3DGraph();
}

// 翻译函数
void MainWindow::retranslateUI()
{
    if (isChinese) {
        // 中文翻译
        ui->label->setText("串口名称");
        ui->label_2->setText("波特率");
        ui->label_3->setText("停止位");
        ui->label_4->setText("数据位");
        ui->label_5->setText("校验位");
        ui->scan->setText("扫描串口");
        ui->turnon->setText("打开串口");
        ui->save->setText("保存数据");
        ui->language->setText("中/EN");

        // 3D图表坐标轴标题
        if (scatterGraph) {
            scatterGraph->axisX()->setTitle("X轴 - 列位置");
            scatterGraph->axisY()->setTitle("Y轴 - 压力高度");
            scatterGraph->axisZ()->setTitle("Z轴 - 行位置");
        }
    } else {
        // 英文翻译
        ui->label->setText("Port Name");
        ui->label_2->setText("Baud Rate");
        ui->label_3->setText("Parity");
        ui->label_4->setText("Data Bits");
        ui->label_5->setText("Stop Bits");
        ui->scan->setText("Scan");
        ui->turnon->setText("Open");
        ui->save->setText("Save Data");
        ui->language->setText("中/EN");

        // 3D图表坐标轴标题
        if (scatterGraph) {
            scatterGraph->axisX()->setTitle("X Axis - Column Position");
            scatterGraph->axisY()->setTitle("Y Axis - Pressure Height");
            scatterGraph->axisZ()->setTitle("Z Axis - Row Position");
        }
    }

    // 更新串口按钮文本（如果串口已打开）
    if (serial->isOpen()) {
        ui->turnon->setText(isChinese ? "关闭串口" : "Close Port");
    }

    // 更新3D图表的数据点标签
    if (scatterGraph && !scatterGraph->seriesList().isEmpty()) {
        QScatter3DSeries *series = scatterGraph->seriesList().at(0);
        series->setItemLabelFormat(isChinese ?
                                  "行: @zLabel, 列: @xLabel, 高度: @yLabel" :
                                  "Row: @zLabel, Column: @xLabel, Height: @yLabel");
    }
}

// 语言切换槽函数
void MainWindow::on_language_clicked()
{
    isChinese = !isChinese;
    retranslateUI();
}

void MainWindow::init3DGraph()
{
    // 创建3D散点图
    scatterGraph = new Q3DScatter();

    // 创建容器widget来嵌入3D图表
    container = QWidget::createWindowContainer(scatterGraph);

    // 设置容器大小
    container->setMinimumSize(400, 300);
    container->setMaximumSize(800, 600);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 将3D图表添加到QFrame中
    if (ui->frame_3d->layout()) {
        delete ui->frame_3d->layout();
    }
    QVBoxLayout *layout = new QVBoxLayout(ui->frame_3d);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(container);

    // 设置图表主题
    scatterGraph->activeTheme()->setType(Q3DTheme::ThemeQt);

    // 设置背景
    scatterGraph->activeTheme()->setBackgroundEnabled(true);
    scatterGraph->activeTheme()->setBackgroundColor(QColor(0, 26, 51));
    scatterGraph->activeTheme()->setWindowColor(QColor(0, 26, 51));
    scatterGraph->activeTheme()->setLabelBackgroundEnabled(false);

    // 设置坐标轴标签和标题颜色为白色
    scatterGraph->activeTheme()->setLabelTextColor(Qt::white);
    scatterGraph->activeTheme()->setLabelBackgroundColor(Qt::transparent);

    // 设置坐标轴标题（初始为中文）
    scatterGraph->axisX()->setTitle("X轴 - 列位置");
    scatterGraph->axisY()->setTitle("Y轴 - 压力高度");
    scatterGraph->axisZ()->setTitle("Z轴 - 行位置");

    // 设置坐标轴标题可见
    scatterGraph->axisX()->setTitleVisible(true);
    scatterGraph->axisY()->setTitleVisible(true);
    scatterGraph->axisZ()->setTitleVisible(true);

    // 设置网格颜色
    scatterGraph->activeTheme()->setGridLineColor(QColor(255, 255, 255, 128));

    // 创建数据系列
    QScatter3DSeries *series = new QScatter3DSeries;
    series->setItemLabelFormat("行: @zLabel, 列: @xLabel, 高度: @yLabel");
    series->setBaseColor(Qt::white);
    series->setItemSize(0.3f);

    #if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        series->setMesh(QAbstract3DSeries::MeshCube);
    #else
        series->setMesh(QScatter3DSeries::MeshCube);
    #endif

    scatterGraph->addSeries(series);

    // 设置初始视角
    scatterGraph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetIsometricRight);
    scatterGraph->scene()->activeCamera()->setZoomLevel(120);

    // 设置阴影质量
    scatterGraph->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);

    // 初始化显示测试点
    update3DGraph();
}

void MainWindow::update3DGraph()
{
    if (!scatterGraph || scatterGraph->seriesList().isEmpty()) {
        qDebug() << "3D图表未初始化";
        return;
    }

    QScatter3DSeries *series = scatterGraph->seriesList().at(0);

    // 更新数据点标签格式
    series->setItemLabelFormat(isChinese ?
                              "行: @zLabel, 列: @xLabel, 高度: @yLabel" :
                              "Row: @zLabel, Column: @xLabel, Height: @yLabel");

    QScatterDataArray *dataArray = new QScatterDataArray;

    // 获取最新的数据点
    int dataIndex = allData.size() - 1;
    if (dataIndex < 0) {
        qDebug() << (isChinese ? "没有数据可用，显示默认网格" : "No data available, showing default grid");
        // 如果没有数据，显示高度为0的网格
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 4; ++col) {
                float x = col;
                float y = 0.0f;
                float z = row;
                dataArray->append(QVector3D(x, y, z));
            }
        }
    } else {
        const QVector<int> &data = allData[dataIndex];

        // 根据数据顺序建立对应关系
        QVector<QVector<float>> heightMap(3, QVector<float>(4, 0.0f));

        // 填充高度映射表
        for (int i = 0; i < data.size(); i++) {
            int row = i / 4;
            int col = i % 4;

            if (row < 3 && col < 4) {
                heightMap[row][col] = data[i] * 0.1f;
            }
        }

        // 创建连续的柱体效果
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 4; ++col) {
                float x = col;
                float z = row;
                float totalHeight = heightMap[row][col];

                float cubeSpacing = 0.1f;
                float cubeHeight = 0.08f;
                int numCubes = qMax(1, static_cast<int>(totalHeight / cubeSpacing));

                for (int cubeIndex = 0; cubeIndex < numCubes; ++cubeIndex) {
                    float y = cubeIndex * cubeSpacing + cubeHeight / 2;
                    if (y <= totalHeight) {
                        dataArray->append(QVector3D(x, y, z));
                    }
                }

                qDebug() << (isChinese ? "点[" : "Point[") << row << "," << col
                         << (isChinese ? "] - 压力值:" : "] - Pressure:") << data[row*4+col]
                         << (isChinese ? "总高度:" : "Total Height:") << totalHeight
                         << (isChinese ? "立方体数量:" : "Cubes:") << numCubes;
            }
        }
    }

    series->dataProxy()->resetArray(dataArray);
    series->setBaseColor(Qt::white);
    series->setItemSize(0.3f);

    #if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        series->setMesh(QAbstract3DSeries::MeshCube);
    #else
        series->setMesh(QScatter3DSeries::MeshCube);
    #endif

    // 动态调整Y轴范围
    float maxHeight = 0;
    if (!allData.isEmpty()) {
        const QVector<int> &latestData = allData.last();
        for (int value : latestData) {
            float height = value * 0.1f;
            if (height > maxHeight) {
                maxHeight = height;
            }
        }
    }

    // 设置坐标轴范围
    scatterGraph->axisX()->setRange(-0.5, 3.5);
    scatterGraph->axisY()->setRange(-0.5, qMax(1.0f, maxHeight + 0.5f));
    scatterGraph->axisZ()->setRange(-0.5, 2.5);

    qDebug() << (isChinese ? "3D图表更新完成，点数:" : "3D graph updated, points:")
             << dataArray->size()
             << (isChinese ? "最大高度:" : "Max height:") << maxHeight;
}

int MainWindow::clampToZero(int value)
{
    return (value >= 0 && value <= 4095) ? value : 0;
}

void MainWindow::on_scan_clicked()
{
    ui->name->clear();
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QSerialPort com;
        com.setPort(info);
        if(com.open(QIODevice::ReadWrite))
        {
            ui->name->addItem(info.portName());
            com.close();
        }
    }
}

void MainWindow::on_turnon_clicked()
{
    if(!serial->isOpen())
    {
        serialSet();
        if(serial->open(QIODevice::ReadWrite)) {
            ui->radioButton->setChecked(true);
            ui->turnon->setText(isChinese ? "关闭串口" : "Close");
            allData.clear();
            qDebug() << (isChinese ? "串口打开成功" : "Port opened successfully");

            // 清空3D图表并重新初始化
            if (scatterGraph && !scatterGraph->seriesList().isEmpty()) {
                QScatter3DSeries *series = scatterGraph->seriesList().at(0);
                series->dataProxy()->resetArray(new QScatterDataArray);
            }
            // 显示初始网格
            update3DGraph();
        } else {
            QMessageBox::critical(this, isChinese ? "错误" : "Error",
                                isChinese ? "无法打开串口" : "Failed to open port");
        }
    }
    else if(serial->isOpen())
    {
        serial->close();
        ui->radioButton->setChecked(false);
        ui->turnon->setText(isChinese ? "打开串口" : "Open");
        qDebug() << (isChinese ? "串口已关闭" : "Port closed");
    }
}

void MainWindow::serialSet()
{
    serial->setPortName(ui->name->currentText());
    serial->setBaudRate(ui->botelu->currentText().toLong());
    switch (ui->tingzhiwei->currentIndex())
    {
        case 0:serial->setStopBits(QSerialPort::OneStop);break;
        case 1:serial->setStopBits(QSerialPort::OneAndHalfStop);break;
        case 2:serial->setStopBits(QSerialPort::TwoStop);break;
    }
    switch (ui->shujuwei->currentText().toLong())
    {
        case 5:serial->setDataBits(QSerialPort::Data5);break;
        case 6:serial->setDataBits(QSerialPort::Data6);break;
        case 7:serial->setDataBits(QSerialPort::Data7);break;
        case 8:serial->setDataBits(QSerialPort::Data8);break;
    }
    switch (ui->jiaoyanwei->currentIndex())
    {
        case 0:serial->setParity(QSerialPort::NoParity);break;
        case 1:serial->setParity(QSerialPort::EvenParity);break;
        case 2:serial->setParity(QSerialPort::OddParity);break;
    }
}

void MainWindow::setButtonColor(QPushButton* button, int value)
{
    value = qBound(0, value, 200);
    int blue = value * 255 / 200;
    QString style = QString(
        "QPushButton {"
        "   background-color: rgb(%1, %1, 255);"
        "   border: 1px solid gray;"
        "   border-radius: 3px;"
        "   padding: 2px;"
        "   color: %2;"
        "   text-align: center;"  // 添加居中对齐
        "}"
        "QPushButton:pressed {"
        "   background-color: rgb(%3, %3, 200);"
        "}"
        ).arg(255 - blue/2)
         .arg(value > 2048 ? "white" : "black")
         .arg(200 - blue/3);
    button->setStyleSheet(style);
}

void MainWindow::data_Receive()
{
    buffer.append(serial->readAll());

    while(buffer.size() >= 34)
    {
        if(static_cast<uint8_t>(buffer[0]) == 0x0A &&
           static_cast<uint8_t>(buffer[25]) == 0x0B)
        {
            int rawP11 = static_cast<uint16_t>(buffer[1] << 8) | static_cast<uint8_t>(buffer[2]);
            int rawP12 = static_cast<uint16_t>(buffer[3] << 8) | static_cast<uint8_t>(buffer[4]);
            int rawP13 = static_cast<uint16_t>(buffer[5] << 8) | static_cast<uint8_t>(buffer[6]);
            int rawP14 = static_cast<uint16_t>(buffer[7] << 8) | static_cast<uint8_t>(buffer[8]);
            int rawP21 = static_cast<uint16_t>(buffer[9] << 8) | static_cast<uint8_t>(buffer[10]);
            int rawP22 = static_cast<uint16_t>(buffer[11] << 8) | static_cast<uint8_t>(buffer[12]);
            int rawP23 = static_cast<uint16_t>(buffer[13] << 8) | static_cast<uint8_t>(buffer[14]);
            int rawP24 = static_cast<uint16_t>(buffer[15] << 8) | static_cast<uint8_t>(buffer[16]);
            int rawP31 = static_cast<uint16_t>(buffer[17] << 8) | static_cast<uint8_t>(buffer[18]);
            int rawP32 = static_cast<uint16_t>(buffer[19] << 8) | static_cast<uint8_t>(buffer[20]);
            int rawP33 = static_cast<uint16_t>(buffer[21] << 8) | static_cast<uint8_t>(buffer[22]);
            int rawP34 = static_cast<uint16_t>(buffer[23] << 8) | static_cast<uint8_t>(buffer[24]);

            P11 = clampToZero(rawP11);
            P12 = clampToZero(rawP12);
            P13 = clampToZero(rawP13);
            P14 = clampToZero(rawP14);
            P21 = clampToZero(rawP21);
            P22 = clampToZero(rawP22);
            P23 = clampToZero(rawP23);
            P24 = clampToZero(rawP24);
            P31 = clampToZero(rawP31);
            P32 = clampToZero(rawP32);
            P33 = clampToZero(rawP33);
            P34 = clampToZero(rawP34);

            QVector<int> currentData;
            currentData << P11 << P12 << P13 << P14
                        << P21 << P22 << P23 << P24
                        << P31 << P32 << P33 << P34;
            allData.append(currentData);

            QFont font;
            font.setPointSize(15);
            font.setBold(true);
            QString fontStyle = "color: white;";

            ui->M11->setFont(font); ui->M11->setText(QString::number(P11)); ui->M11->setStyleSheet(fontStyle); setButtonColor(ui->M11, P11);
            ui->M12->setFont(font); ui->M12->setText(QString::number(P12)); ui->M12->setStyleSheet(fontStyle); setButtonColor(ui->M12, P12);
            ui->M13->setFont(font); ui->M13->setText(QString::number(P13)); ui->M13->setStyleSheet(fontStyle); setButtonColor(ui->M13, P13);
            ui->M14->setFont(font); ui->M14->setText(QString::number(P14)); ui->M14->setStyleSheet(fontStyle); setButtonColor(ui->M14, P14);

            ui->M21->setFont(font); ui->M21->setText(QString::number(P21)); ui->M21->setStyleSheet(fontStyle); setButtonColor(ui->M21, P21);
            ui->M22->setFont(font); ui->M22->setText(QString::number(P22)); ui->M22->setStyleSheet(fontStyle); setButtonColor(ui->M22, P22);
            ui->M23->setFont(font); ui->M23->setText(QString::number(P23)); ui->M23->setStyleSheet(fontStyle); setButtonColor(ui->M23, P23);
            ui->M24->setFont(font); ui->M24->setText(QString::number(P24)); ui->M24->setStyleSheet(fontStyle); setButtonColor(ui->M24, P24);

            ui->M31->setFont(font); ui->M31->setText(QString::number(P31)); ui->M31->setStyleSheet(fontStyle); setButtonColor(ui->M31, P31);
            ui->M32->setFont(font); ui->M32->setText(QString::number(P32)); ui->M32->setStyleSheet(fontStyle); setButtonColor(ui->M32, P32);
            ui->M33->setFont(font); ui->M33->setText(QString::number(P33)); ui->M33->setStyleSheet(fontStyle); setButtonColor(ui->M33, P33);
            ui->M34->setFont(font); ui->M34->setText(QString::number(P34)); ui->M34->setStyleSheet(fontStyle); setButtonColor(ui->M34, P34);

            qDebug() << (isChinese ? "收到数据，更新3D图表" : "Data received, updating 3D graph");
            // 更新三维坐标系
            update3DGraph();

            buffer.remove(0, 26);
        }
        else
        {
            buffer.remove(0, 1);
        }
    }
}

void MainWindow::on_save_clicked()
{
    if(allData.isEmpty()) {
        QMessageBox::information(this, isChinese ? "提示" : "Info",
                               isChinese ? "没有数据可保存" : "No data to save");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
                                                  isChinese ? "保存数据" : "Save Data",
                                                  QDir::homePath() + "/data.csv",
                                                  "CSV文件 (*.csv);;所有文件 (*)");

    if(fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, isChinese ? "错误" : "Error",
                            QString(isChinese ?
                                   "无法打开文件进行写入:\n%1\n错误: %2" :
                                   "Cannot open file for writing:\n%1\nError: %2")
                            .arg(fileName)
                            .arg(file.errorString()));
        return;
    }

    QTextStream out(&file);
    out.setGenerateByteOrderMark(true);
    out << "P11,P12,P13,P14,P21,P22,P23,P24,P31,P32,P33,P34\n";

    for(const auto &data : allData) {
        for(int i = 0; i < data.size(); ++i) {
            if(i != 0) out << ",";
            out << data[i];
        }
        out << "\n";
    }

    file.close();

    QMessageBox::information(this, isChinese ? "成功" : "Success",
                           QString(isChinese ?
                                  "共保存 %1 条数据到:\n%2" :
                                  "Saved %1 records to:\n%2")
                           .arg(allData.size())
                           .arg(fileName));
}

MainWindow::~MainWindow()
{
    if (scatterGraph) {
        delete scatterGraph;
        scatterGraph = nullptr;
    }
    delete ui;
}
