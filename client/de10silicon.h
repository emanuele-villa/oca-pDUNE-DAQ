#ifndef DE10_SILICON_H
#define DE10_SILICON_H
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <QWidget>
#include "mainwindow.h"
//#include "schermataprincipale.h"
#include <QLineSeries>
#include <QtCharts>
#include <QChartView>

#define de10_silicon_socket_null -1
#define de10_silicon_dampe_string_buffer 512
class de10_silicon : public QObject{
    Q_OBJECT;
signals:
    void textChanged(QString);
    void data(QVector<double>);
public:
    ~de10_silicon();
    de10_silicon(char *address, int port);
    int client_send(const char *buffer);
    int client_receive();
    int client_receive_int();
    int client_socket;

private:
    int client_connect(char *address, int port);
    //int client_receive();
public:
    uint32_t daq_mode = 0;
    uint32_t daq_cal_mode = 0;
    uint32_t daq_IntTri_en = 0;
    uint32_t daq_TestUnit_en = 0;
    void changeText(const QString& new_text){

        emit textChanged(new_text);
    }
    void sendData(QVector<double> event){

        emit data(event);
    }
    int readReg(int regAddr);
    int Init();
    int SetDelay();
    int SetMode();
    int GetEventNumber();
    char* PrintAllEventNumber(int log=1,int JLV1num=0);
    int EventReset();
    int GetEvent();
    int OverWriteDelay();
    int Calibrate();
    int WriteCalibPar();
    int SaveCalibrations();
    int intTriggerPeriod();
    int selectTrigger();
    int configureTestUnit();
};
#endif
