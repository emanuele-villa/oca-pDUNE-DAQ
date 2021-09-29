#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "comandiprincipali.h"
#include "slowcontrol.h"
#include <QString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_principali_clicked()
{
    QString indirizzo = ui->textEdit->toPlainText();
    QByteArray ba = indirizzo.toLatin1();
    char *address = ba.data();
    ui->label->setText(address);
    comandiPrincipali *schermata = new comandiPrincipali(nullptr, address);
    //schermata->indirizzo = address;
    schermata->setModal(true);
    schermata->exec();
}


void MainWindow::on_pushButton_2_clicked()
{
    slowControl schermata;
    schermata.setModal(true);
    schermata.exec();
}

