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

class de10_silicon : public de10_silicon_base, public QObject {
  Q_OBJECT;
  
signals:
  void textChanged(QString);
  void data(QVector<double>);
  
public:
  ~de10_silicon();
  de10_silicon(const char *address, int port):de10_silicon_base(address, port);
  
public:
  void changeText(const sstd::string& new_text){
    emit textChanged(new_text);
  }
  void sendData(std::vector<double> event){
    emit data(event);
  }

};
#endif
