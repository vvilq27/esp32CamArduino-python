/**
 * @file /ebufferIdxample1/main.cpp
 * @author Philippe Lucidarme
 * @date December 2019
 * @brief File containing ebufferIdxample of serial port communication
 *
 * This ebufferIdxample send the ASCII table through the serial device
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
    //for serial ports above "COM9", we must use this ebufferIdxtended syntabufferIdx of "\\.\COMbufferIdx".
    //also works for COM0 to COM9.
    //https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea?redirectedfrom=MSDN#communications-resources
    #define SERIAL_PORT "COM3"
#endif
#if defined (__linubufferIdx__) || defined(__APPLE__)
    #define SERIAL_PORT "/dev/ttyACM0"
#endif


#define dataBuffSize 19000
/*!
 * \brief main  Simple ebufferIdxample that send ASCII characters to the serial device
 * \return      0 : success
 *              <0 : an error occured
 */

serialib serial;
vector<char> vData;

void waitNewLine(){
    char serialByte;
    bool crCharflag = false;

    while(true){
        serial.readChar(&serialByte,1);

        if(crCharflag == false && serialByte == 13){
            crCharflag = true;
        }

        if(crCharflag == true && serialByte == 10){
            break;
        }
    }
}

char * readLine(){
    bool carriageReturn = false;
    bool newLine = false;

    static char lineBuffer[200] = {0};
    unsigned char lineBufferIdbufferIdx = 0;

    char serialByte;

    //read until \r\n

    while(!newLine){
        serial.readChar(&serialByte,1);

        if( serialByte == 13){
            carriageReturn = true;

            lineBuffer[lineBufferIdbufferIdx++] = serialByte;
            // printf("%s\n", "13 detected");
            continue;
        }

        if (serialByte == 10 && carriageReturn){
            carriageReturn = false;
            newLine = true;
            // printf("%s\n", "10 detected");
            // printf("%s\n", lineBuffer);

            lineBuffer[lineBufferIdbufferIdx++] = serialByte;
 
            break;
        }

        carriageReturn = false;

        lineBuffer[lineBufferIdbufferIdx++] = serialByte;

        if(lineBufferIdbufferIdx > 199){
            printf("%s\n", "ERROR, readline buffer OVF");
            printf("buffer: %s\n", lineBuffer);
            printf("%d\n", lineBufferIdbufferIdx);
            while(true);
        }
    }

    return lineBuffer;
}

// unsigned char * displayLine(){
//     unsigned char * line = readLine();
//     unsigned char * lineReturned = line;

//     char idbufferIdx = *line;
//     char idbufferIdx2 = 0;

//     // while(idbufferIdx2++ < idbufferIdx){
//     // while(*line != '\0'){
//     printf("%s", line);
//     // }

//     printf("\n");

//     return lineReturned;
// }

void waitNewPic(){
    readLine();
    char * line = readLine();
    // printf("here: %s", line);
    unsigned char newLine[] = "newimg:";

    bool newPic = false;
     

    while(!newPic){
        unsigned char * newLinePtr = newLine;
        int i = 0;

        while(*line++ == *newLinePtr++ && i++ < 7){
            printf("%c  ", *line);
            printf("%c\n", *newLinePtr);

            if(*newLinePtr == ':'){
                newPic = true;
                printf("%s\n", "new image");
                break;
            }
        }

        if(!newPic)
            line = readLine();
    }

    printf("%s\n", "get new pic");
}

// void collectPic(){
//     char data[8000];
//     unsigned int dataIdbufferIdx = 0;

//     // waitNewPic();

//     unsigned char * line = readLine();

//     while(*line != 12 && *line != 20 && dataIdbufferIdx < 7990){
//         while(*line != '\0')
//             data[dataIdbufferIdx++] = *line++;

//         line = readLine();
//         // printf("%s\n", line);
//     }

//     // printf("%s\n", data);
//     for(unsigned char c: data)
//         printf("%d,", c);
// }

// void printPic(){
//     unsigned char * line;

//     while(true){
//         line = displayLine();

//         if(*line == 20)
//             break;
//     }    
// }

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

    ofstream myfile;
    //binary mode prevents automate adding \n when writing to file


    bool crCharflag = false;

    serial.flushReceiver();
    serial.flushReceiver();

    // waitNewPic();

    int picIdbufferIdx = 0;

    char bufSize = 12;
    char bufIdx = 0;

    char lineBuffer[bufSize] = {0};
    char * lineBufferPtr = lineBuffer;
    char serialByte;

    waitNewPic();
    // waitNewLine();

    while(true){
        serial.readChar(&serialByte,1);
        bufIdx++;
        
        if(bufIdx < bufSize)
            *lineBufferPtr++ = serialByte;

        else
            break;

        // if(crCharflag == false && serialByte == 13){
        //     crCharflag = true;
        // }

        // if(crCharflag == true && serialByte == 10){
        //     crCharflag = false;
        //     printf("%s\n", lineBuffer);
        //     memset(lineBuffer, 0x00, bufSize);
        //     lineBufferPtr = lineBuffer;
        //     break;
        // }
    }

    printf("%s\n", lineBuffer);

    serial.closeDevice();

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
// add img idbufferIdx to packet
