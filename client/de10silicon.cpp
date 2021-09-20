#include <sys/time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include "de10silicon.h"
#include <errno.h>
#include <QApplication>
#include <QObject>
#include <stdint.h>

de10_silicon::de10_silicon(char *address, int port){
    changeText("hello");
    printf("de10 silicon creato\n");
    client_socket = client_connect(address, port);


}

//--------------------------------------------------------------
de10_silicon::~de10_silicon(){
    client_send("quit\n");
    if (client_socket != -1) {
        shutdown(client_socket, SHUT_RDWR);
        close(client_socket);
        client_socket = -1;
    }
}



int de10_silicon::client_connect(char *address, int port) {
    struct sockaddr_in server_addr;
    struct hostent *server;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket < 0){

        perror("errore creazione socket");
        exit(EXIT_FAILURE);
    }else{

        printf("socket creato: %d\n", client_socket);
    }

    server = gethostbyname(address);
    if(server == NULL){

        fprintf(stderr, "non esiste questo host");
        exit(EXIT_FAILURE);
    }

    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(port);

    if(::connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){

        perror("errore connessione");
        exit(EXIT_FAILURE);
    }

    printf("connessione riuscita\n");
    return client_socket;
}

int de10_silicon::client_send(const char *buffer) {
    int result;
    //char backup[d_dampe_string_buffer];
    printf("invio\n");
    if ((client_socket != -1) && (buffer)) {
        //memset(backup, 0, d_dampe_string_buffer);
        //snprintf(backup, d_dampe_string_buffer, "%s", buffer);
        if ((write(client_socket, buffer, strlen(buffer)) > 0)) {
            usleep(250000);
            printf("[CLIENT] messaggio inviato correttamente\n");
            changeText("inviato");
        }else{

            fprintf(stderr, "errore invio");
        }
    }
    return true;
}

int de10_silicon::client_receive_int(){

    int n;
    int cont = 0;
    while(cont < 111 * sizeof(int)){
        int temp;
        n = read(client_socket, &temp, sizeof(temp));
        if(n < 0){

            perror("errore lettura");
        }else{

            char c[4];
            char msg[256];
            sprintf(c, "%x", temp);
            //sprintf(msg, "ho letto %d", n);
            changeText(c);
            //usleep(100000);
            bzero(c, sizeof(c));
            cont += n;
        }
    }

    changeText("fine");

    /*QVector<double> values(110);
    int n;
    int i = 0;
    while(i < 110){
        int temp;
        n = read(client_socket, &temp, sizeof(temp));
        if(n <  0){

            perror("errore lettura");
        }else{

            values[i] = temp;
            i++;
        }
    }*/

    //predisporre qui il grafico


    //sendData(values);

    /*for(int i = 0; i < 640; i++){

        printf("ricevuto: %d\n", value[i]);
    }*/
}

int de10_silicon::client_receive(){


    char msg[257];
    size_t n = 0;
    printf("in ascolto su socket %d\n", client_socket);

    n = read(client_socket, msg, sizeof(msg) - 1);

    if(n < 0){

        fprintf(stderr, "errore lettura\n");
    }else if(n == 0){

        printf("finito\n");
    }else{
        //msg[n] = '\0';
        printf("ho letto: %d\n", n);
        printf("%s\n",msg);
        changeText(msg);
        usleep(100000);
        if(msg[n - 1] == '\0'){

            bzero(msg, sizeof(msg));
            return -1;
        }
    }

    bzero(msg, sizeof(msg));
    return n;
}

//--------------------------------------------------------------
int de10_silicon::Init() {
    //client_send("trigger -off\n");
    //client_send("write -x 040700\n");
    printf("[>>> initializing dampe (reset everything)]\n");
    client_send("init");
    client_receive();
    uint32_t reg_content = 1;
    //char *c = (char *)&reg_content;
    char c[sizeof (uint32_t) * 8 +1];
    sprintf(c, "%x", reg_content);
    client_send(c);
    changeText(c);
    bzero(c, sizeof(c));
    reg_content = 0x02faf080;
    //c = (char *)&reg_content;
    sprintf(c, "%x", reg_content);
    client_send(c);
    changeText(c);
    bzero(c, sizeof(c));
    reg_content = 0x000000ff;
    //c = (char *)&reg_content;
    sprintf(c, "%x", reg_content);
    client_send(c);
    changeText(c);
    bzero(c, sizeof(c));
    reg_content = 0x0000028a;
    //c = (char *)&reg_content;
    sprintf(c, "%x", reg_content);
    client_send(c);
    changeText(c);
    bzero(c, sizeof(c));
    reg_content = 0x00040028;
    //c = (char *)&reg_content;
    sprintf(c, "%x", reg_content);
    client_send(c);
    changeText(c);
    reg_content = 0x00040002;
    //c = (char *)&reg_content;
    sprintf(c, "%x", reg_content);
    client_send(c);
    changeText(c);
    reg_content  = 0x00070145;

    //c = (char *)&reg_content;
    sprintf(c, "%x", reg_content);
    client_send(c);
    changeText(c);

    return 0;
}
int de10_silicon::SetDelay(){
    client_send("set delay");
    uint32_t delay = 5;
    char c[sizeof (uint32_t) * 8 + 1];
    sprintf(c, "%x", delay);
    client_send(c);
    client_receive();
    return 0;
}

int de10_silicon::SetMode() {

    client_send("set mode");
    uint32_t mode = 3;
    char c[sizeof (uint32_t) * 8 + 1];
    sprintf(c, "%d", mode);
    client_send(c);
    client_receive();
    return 0;
}

int de10_silicon::GetEventNumber() {
    //	printf("[>>> getting events]\n");
    client_send("get event number");
    client_receive();
    return 0;
}

char* de10_silicon::PrintAllEventNumber(int log,int JLV1num) {
    int ret=0;
    static char numbers[1023]="";
    // snprintf(numbers, 1023, "Dampe %02d: %6d", selfaddress, 0);
    // printf("[>>>>>] dampe: %s\n", numbers);
    client_send("print all event number");
    client_receive();
    return numbers;
}

int de10_silicon::EventReset() {
    // client_send("write -x 020400\n");
    // client_send("write -x 040700\n");
    // printf("[>>>>>] resetting events (reinitialize)\n");
    client_send("event reset");
    client_receive();
    return 0;
}

int de10_silicon::GetEvent(){

    client_send("get event");
    int ret = 1;
    int i = 1;
    client_receive_int();
    return 0;
}

int de10_silicon::OverWriteDelay(){

    client_send("OverWriteDelay");
    client_receive();
    return 0;
}

int de10_silicon::Calibrate(){

    client_send("Calibrate");
    client_receive();
    return 0;
}

int de10_silicon::WriteCalibPar(){

    client_send("WriteCalibPar");
    client_receive();
    return 0;
}

int de10_silicon::SaveCalibrations(){

    client_send("SaveCalibrations");
    client_receive();
    return 0;
}


int main(int argc, char *argv[]){

    QApplication a(argc, argv);
    MainWindow s;
    s.show();
    return a.exec();
    return 0;
}
