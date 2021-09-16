#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setLabel(char[10]);



private slots:
    void on_pushButton_clicked();

private:
    //acces widget i'll add in main window
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
