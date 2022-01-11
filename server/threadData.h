#include <poll.h>
#include "serverFunctions.h"

#pragma once

class ThreadData{
public:
    pollfd descriptors[MAX_DESCRIPTORS_NUM];
    int descriptorsNum;
    int timeout;
    bool disconnect;

    ThreadData(){
        this->descriptorsNum = 0;
        this->timeout = 2500;
        this->disconnect = false;
    }

    ThreadData(int clientFd, int timeout = 2500){
        this->descriptorsNum = 0;
        this->timeout = timeout;
        this->disconnect = false;
        this->newDescriptor(clientFd);
    }

    ~ThreadData(){
        for(int i = 0; i < descriptorsNum; i++){
            close(descriptors[i].fd);
            std::cout << "Descriptor " << descriptors[i].fd << " closed" << std::endl;
        }
    }

    bool operator==(const ThreadData& threadData){
        if( this->descriptorsNum == threadData.descriptorsNum && 
            this->timeout == threadData.timeout && 
            this->disconnect == threadData.disconnect){
                for(int i = 0; i < this->descriptorsNum; i++){
                    if(this->descriptors[i].fd != threadData.descriptors[i].fd){
                        return false;
                    }
                }
        }
        return true;
    }

    bool freeSlots(){
        return this->descriptorsNum < MAX_DESCRIPTORS_NUM;
    }

    result newDescriptor(int fd){
        if(this->descriptorsNum == MAX_DESCRIPTORS_NUM){
            return FAILURE;
        }
        this->descriptors[descriptorsNum].fd = fd;
        this->descriptors[descriptorsNum++].events = POLLIN;
        return SUCCESS;
    }

    result removeDescriptor(int fd){
        for(int i = 0; i < this->descriptorsNum; i++){
            if(this->descriptors[i].fd == fd){
                close(descriptors[i].fd);
                std::cout << "Descriptor " << descriptors[i].fd << " closed" << std::endl;
                for(int j = i + 1; j <= this->descriptorsNum; j++){
                    descriptors[j - 1] = descriptors[j];
                }
                --this->descriptorsNum;
                return SUCCESS;
            }
        }
        return FAILURE;
    }
};