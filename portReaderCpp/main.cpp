/**
 * @file /example1/main.cpp
 * @author Philippe Lucidarme
 * @date December 2019
 * @brief File containing example of serial port communication
 *
 * This example send the ASCII table through the serial device
 *
 * @see https://lucidar.me
 */


// Serial library
#include "serialib.h"
#include <unistd.h>
#include <stdio.h>
#include <string>
#include<vector>

#include <iostream>
#include <fstream>

using namespace std;


#if defined (_WIN32) || defined(_WIN64)
    //for serial ports above "COM9", we must use this extended syntax of "\\.\COMx".
    //also works for COM0 to COM9.
    //https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea?redirectedfrom=MSDN#communications-resources
    #define SERIAL_PORT "COM3"
#endif
#if defined (__linux__) || defined(__APPLE__)
    #define SERIAL_PORT "/dev/ttyACM0"
#endif


#define dataBuffSize 19000
/*!
 * \brief main  Simple example that send ASCII characters to the serial device
 * \return      0 : success
 *              <0 : an error occured
 */

serialib serial;
vector<char> vData;

unsigned char * readLine(){
    bool carriageReturn = false;
    bool newLine = false;

    static unsigned char lineBuffer[100] = {0};
    unsigned char lineBufferIdx = 1;

    char serialByte;

    //read until \r\n

    while(!newLine){
        serial.readChar(&serialByte,1);

        if( serialByte == 13){
            carriageReturn = true;

            lineBuffer[lineBufferIdx++] = serialByte;
            continue;
        }

        if (serialByte == 10 && carriageReturn){
            carriageReturn = false;
            newLine = true;
 
            continue;
        }

        carriageReturn = false;

        lineBuffer[lineBufferIdx++] = serialByte;

        if(lineBufferIdx > 99){
            printf("%s\n", "ERROR, readline buffer OVF");
            while(true);
        }
    }
    lineBuffer[0] = lineBufferIdx;
    lineBuffer[lineBufferIdx] = '\0';

    return lineBuffer;
}

unsigned char * displayLine(){
    unsigned char * line = readLine();
    unsigned char * lineReturned = line;

    char idx = *line;
    char idx2 = 0;

    // while(idx2++ < idx){
    // while(*line != '\0'){
    printf("%s", line);
    // }

    printf("\n");

    return lineReturned;
}

void waitNewPic(){
    unsigned char * line = readLine();

    while(*line != 12)
        line = readLine();

    printf("%s\n", "get new pic");
}

void collectPic(){
    char data[8000];
    unsigned int dataIdx = 0;

    // waitNewPic();

    unsigned char * line = readLine();

    while(*line != 12 && *line != 20 && dataIdx < 7990){
        while(*line != '\0')
            data[dataIdx++] = *line++;

        line = readLine();
        // printf("%s\n", line);
    }

    // printf("%s\n", data);
    for(unsigned char c: data)
        printf("%d,", c);
}

void printPic(){
    unsigned char * line;

    while(true){
        line = displayLine();

        if(*line == 20)
            break;
    }    
}

bool compareArrays(unsigned char * a, unsigned char * b){
    bool flag = true;
    int k = 7;
    
    while(k-- && flag){
        if(*a++ != *b++)
            flag = false;
    }

    return flag;
}


int main( /*int argc, char *argv[]*/)
{
    // Serial object


    // Connection to serial port
    char errorOpening = serial.openDevice("\\\\.\\COM3", 1000000);


    // If connection fails, return the error code otherwise, display a success message
    if (errorOpening!=1) {
        printf("%s\n", "port opening error");

        while(1);
    }
    printf ("Successful connection to %s\n",SERIAL_PORT);

    unsigned char picFinishLine[] = {112,105,99,32,105,110,32,};


    ofstream myfile;
    //binary mode prevents automate adding \n when writing to file

    while(true){
        bool crCharflag = false;

        char data[dataBuffSize+1];

        serial.flushReceiver();
        serial.flushReceiver();

        // waitNewPic();

        char buf[dataBuffSize] = {'\0'};

        int ret = serial.readBytes(&buf, dataBuffSize);

        int x = 0;

        unsigned char line[34] = {0};
        char lineIdx = 0;
        bool newPic = false;
        bool newPicHold = false;
        int picIdx = 0;

        myfile.open("C:/Users/aro/Documents/python/camera_jpeg_conv/picCpp.txt", ios::binary);

    // albo read char az do 13 10, a pozniej read buffer o rozmiarze pakietu
        while(x++ < dataBuffSize-1){
            unsigned char c = buf[x];

            if(c == 13 && buf[x+1] == 10)
            {
                if(newPicHold){
                    // for(int i = 0; i<34; i++)
                    //     myfile << line[i];
                    
                    // myfile << '\n';
                    // printf("\n");
                }
                
                //offset for indexes to not print them
                x += 3;

                lineIdx = 0;

                newPic = compareArrays(picFinishLine, line);

                if(newPic && newPicHold)
                    break;

                if(newPic && !newPicHold)
                    newPicHold = true;

                /*
                if(newPic){

                    // printf("%s\n", line);
                    for(char v: line)
                        printf("%c", v );
                    printf("\n");
                    // newPic = false;
                }
                */

                continue;
            }

            line[lineIdx++] = c;

            if(newPicHold){
                // printf("%d,", c);
                myfile << c;
            }
        }

        myfile.close();

    }

/*
    for (int c=0;c<dataBuffSize;c++)
    {
        char d;
        serial.readChar(&d,1);

        data[c] = d;
        
    }

    char packetByteCnt = 0;

    for(unsigned char c : data){
        packetByteCnt++;

        if(c == '\r'){
            crCharflag = true;
            // printf("%d,", c);
            continue;
        }

        if(c == '\n' && crCharflag){
            crCharflag = false;
            // printf("%s,\n", "nl");

            continue;
        }

        crCharflag = false;

        printf("%d,", c);
    }
    */

// read line, kinda doesnt work, bytes messy
    // for(int i = 0; i <100; i++){
    //     int bytesRead = serial.readStringNoTimeOut(packet, "\r\n", 70);

    //     // printf("%s\n", packet);
    //     for(unsigned char c : packet){
    //         if(c == ','){
    //             printf(",");
    //             continue;
    //         }
    //         printf("%d,", c);
    //     }
    //     printf("\n%d\t%d\n",bytesRead,serial.available());
    // }

//read buffer, sort it later, readBytes doesnt work
    // serial.readBytes(&data, 4095, 100,1);

    // // printf("%s\n", data);
    // for(int i = 0; i < 1000; i++){
    //     printf("%c,", data[i]);
    // }



    // Close the serial device

    serial.closeDevice();

    printf("\n");

    return 0 ;
}



// todos
// add ::readline() to lib
// wait with read until start of new image
// bump baudrate up
// opencv in cpp
// add this to github
// get img size from mcu
// ack you get all packets
// add img idx to packet
