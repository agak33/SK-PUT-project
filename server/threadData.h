#include <poll.h>
#include "serverFunctions.h"

#pragma once

class ThreadData{
public:
    pollfd descriptors[MAX_DESCRIPTORS_NUM];
    std::string message[MAX_DESCRIPTORS_NUM];
    int descriptorsNum;
    int timeout;
    bool disconnect;

    ThreadData(){
        this->descriptorsNum = 0;
        this->timeout = 2500;
        this->disconnect = false;
        for(int i = 0; i < MAX_DESCRIPTORS_NUM; i++){
            message[i] = "";
        }
    }

    ThreadData(int clientFd, int timeout = 2500){
        this->descriptorsNum = 0;
        this->timeout = timeout;
        this->disconnect = false;
        this->newDescriptor(clientFd);
        for(int i = 0; i < MAX_DESCRIPTORS_NUM; i++){
            message[i] = "";
        }
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

    friend std::ostream& operator<<(std::ostream& os, const ThreadData& threadData){
        os << "cos tam" << std::endl;
        return os;
    }

    bool freeSlots(){
        return this->descriptorsNum < MAX_DESCRIPTORS_NUM;
    }

    bool readyToRead(int index){
        return message[index].back() == DATA_END;
    }

    void addBuffer(char* buffer, int index){
        message[index] += std::string(buffer);
    }

    std::string getMessage(int index){
        std::string result = message[index];
        message[index] = "";
        return result;
    }

    std::string getPrefix(const std::string& message){
        size_t index = message.find(DATA_SEPARATOR);
        if(index == std::string::npos) return message;
        return message.substr(0, index);
    }

    std::string getArguments(const std::string& message, int index){
        size_t i = message.find(DATA_SEPARATOR);
        if(i == std::string::npos) return message;

        std::string prefix = message.substr(0, i);
        if(prefix == LOGIN_PREFIX || prefix == LOGOUT_PREFIX || prefix == REGISTER_PREFIX || prefix == CLOSING_APP_PREFIX){
            return message.substr(i + 1) + DATA_SEPARATOR + std::to_string(descriptors[index].fd);
        }
        return message.substr(i + 1);
    }

    int saveToBuff(char* buffer, std::string& message){
        int bytes = std::min(BUFFER_SIZE, (int)message.size());
        memset(buffer, 0, BUFFER_SIZE);
        sprintf(buffer, "%s", message.substr(0, bytes).c_str());
        message = message.substr(bytes);
        return bytes;
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
                message[i] = "";
                std::cout << "Descriptor " << descriptors[i].fd << " closed" << std::endl;
                for(int j = i + 1; j <= this->descriptorsNum; j++){
                    descriptors[j - 1] = descriptors[j];
                    message[j - 1] = message[j];
                }
                --this->descriptorsNum;
                return SUCCESS;
            }
        }
        return FAILURE;
    }
};