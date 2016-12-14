#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTime>
#include <QtCore>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->lcdNumber->setDigitCount(10);
    ui->lcdNumber->setMode(QLCDNumber::Dec);
    QTime time = QTime::currentTime();
    ui->lcdNumber->display(time.toString("hh:mm:ss"));
    //showFullScreen();
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(timerUpDate()));
    timer->start(1000);
    ui->comboBox->addItem("COM1");
    ui->comboBox->addItem("COM2");
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::timerUpDate()
{
    QDateTime time = QDateTime::currentDateTime();
    ui->lcdNumber->display(time.toString("hh:mm:ss"));
}
void MainWindow::on_pushButton_clicked()
{
    ui->textBrowser->setText("");
}

void MainWindow::on_pushButton_3_clicked()
{
    this->close();
}


void MainWindow::on_pushButton_4_clicked()
{
    ui->textBrowser->append(ui->textEdit_2->toPlainText());
    ui->textEdit_2->setText("");
}
