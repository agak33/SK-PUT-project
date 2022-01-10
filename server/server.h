#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <csignal>

#include "models/user.h"
#include "models/calendar.h"
#include "serverFunctions.h"
#include "data.h"
#include "threadData.h"

#pragma once

#define BUFFER_SIZE 1024

class Server{
public:
    int QUEUE_SIZE;
    int APPLICATION_PORT;
    int MAX_CONNECTED_CLIENTS;
    std::string APPLICATION_HOST;

    Data data;
    std::vector<std::thread> threads;
    std::vector<ThreadData*> threadData;

    typedef std::string (Data::*function)(std::string);
    std::map<std::string, function> functionMap;

    int serverSocket;
    sockaddr_in server_addr = {0};

    Server( const int& app_port, const std::string& app_host, 
            const int& queue_size = 5, const int& max_connected_clients = 50){

        this->APPLICATION_PORT      = app_port;
        this->APPLICATION_HOST      = app_host;
        this->QUEUE_SIZE            = queue_size;
        this->MAX_CONNECTED_CLIENTS = max_connected_clients;

        server_addr.sin_family      = AF_INET;
        server_addr.sin_port        = htons(APPLICATION_PORT);
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        valueCheck(serverSocket, "Socket error", "");
        valueCheck(bind(serverSocket, (sockaddr*)&server_addr, sizeof(server_addr)), "Bind error", "");
        valueCheck(listen(serverSocket, QUEUE_SIZE), "Listen error", "");

        functionMap[LOGIN_PREFIX]                   = &Data::loginUser;
        functionMap[REGISTER_PREFIX]                = &Data::registerUser;
        functionMap[LOGOUT_PREFIX]                  = &Data::logoutUser;
        functionMap[CLOSING_APP_PREFIX]             = &Data::logoutUser;

        functionMap[CALENDAR_INSERT_PREFIX]         = &Data::insertCalendar;
        functionMap[CALENDAR_MODIFY_PREFIX]         = &Data::modifyCalendarName;
        functionMap[CALENDAR_DELETE_PREFIX]         = &Data::deleteCalendar;

        functionMap[CALENDAR_INSERT_USER_PREFIX]    = &Data::insertCalendarUser;
        functionMap[CALENDAR_DELETE_USER_PREFIX]    = &Data::deleteCalendarUser;
        functionMap[CALENDAR_GET_NAMES]             = &Data::getCalendars;
        functionMap[CALENDAR_GET_CALENDAR_INFO]     = &Data::getCalendarInfo;

        functionMap[EVENT_INSERT_PREFIX]            = &Data::insertEvent;
        functionMap[EVENT_MODIFY_PREFIX]            = &Data::modifyEvent;
        functionMap[EVENT_DELETE_PREFIX]            = &Data::deleteEvent;

        std::cout << "Starting server at " << this->APPLICATION_PORT << " port" << std::endl;
    }

    ~Server(){
        for(size_t i = 0; i < threadData.size(); i++){
            threadData[i]->disconnect = true;
        }
        std::cout << "Waiting for threads..." << std::endl;
        for(size_t i = 0; i < threads.size(); i++){
            threads[i].join();
        }
        std::cout << "Deleting data..." << std::endl;
        for(size_t i = 0; i < threadData.size(); i++){
            delete threadData[i];
        }

        close(this->serverSocket);
        std::cout << "Server socket closed" << std::endl;
    }

    void threadFunction(ThreadData* thData){
        char buff[BUFFER_SIZE];
        std::string message, prefix;
        int pollResult;
        while(!thData->disconnect){
            pollResult = poll(thData->descriptors, thData->descriptorsNum, thData->timeout);
            if(pollResult == -1){
                std::cout << "Poll error" << std::endl;
                exit(SIGINT);
            } else{
                for(int i = 0; i < thData->descriptorsNum; i++){
                    if(thData->descriptors[i].revents & POLLIN){
                        std::cout << "reading data from client..." << std::endl;
                        memset(buff, 0, BUFFER_SIZE);
                        read(thData->descriptors[i].fd, buff, BUFFER_SIZE);                       

                        message = std::string(buff);
                        prefix = message.substr(0, message.find(DATA_SEPARATOR));

                        if(prefix == LOGIN_PREFIX || prefix == LOGOUT_PREFIX || prefix == REGISTER_PREFIX || prefix == CLOSING_APP_PREFIX){
                            message += MESSAGE_SEPARATOR + std::to_string(thData->descriptors[i].fd);
                        }

                        std::map<std::string, function>::iterator func = functionMap.find(prefix);
                        if(func == functionMap.end()){
                            std::cout << "Unknown message prefix: " << prefix << std::endl;
                            memset(buff, 0, BUFFER_SIZE);
                            message = FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Server error occured.";
                            sprintf(buff, "%s", message.c_str());
                        }
                        else{
                            message = message.substr(prefix.size() + 1);
                            std::cout << "MESSAGE TO FUNCTION: " << message << std::endl;
                            message = (data.*func->second)(message);
                            std::cout << "MESSAGE FROM FUNCTION: " << message << std::endl;
                            memset(buff, 0, BUFFER_SIZE);
                            sprintf(buff, "%s", message.c_str());
                        }
                        if(prefix != CLOSING_APP_PREFIX) write(thData->descriptors[i].fd, buff, BUFFER_SIZE);
                        else if(prefix == CLOSING_APP_PREFIX) thData->removeDescriptor(thData->descriptors[i].fd);
                    }
                }
                //data.displayCalendars();
                //data.displayUsers();
            }

            if(thData->descriptorsNum == 0){
                break;
            }
        }
        std::cout << "Disconnected thread..." << std::endl;
        std::cout << "Deleting thread data..." << std::endl;

        std::vector<ThreadData*>::iterator thDataPos = std::find(threadData.begin(), threadData.end(), thData);
        threadData.erase(thDataPos);
        delete thData;

        std::cout << "Thread disconnected" << std::endl;
    }

    int threadWithFreeSlots(){
        for(size_t i = 0; i < threadData.size(); i++){
            if(threadData[i]->freeSlots()){
                return i;
            }
        }
        return -1;
    }

    void handleConnection(int clientFd){
        int threadIndex = this->threadWithFreeSlots();
        if(threadIndex == -1){
            ThreadData* thData = new ThreadData(clientFd);
            threadData.push_back(thData);
            threads.push_back(std::thread(&Server::threadFunction, this, thData));
            std::cout << "New thread created" << std::endl;
        } else{
            threadData[threadIndex]->newDescriptor(clientFd);
            std::cout << "New client added to thread" << std::endl;
        }
    }

    void loop(){
        std::cout << "Waiting for new connections" << std::endl;
        while(1){
            int clientFd = accept(this->serverSocket, NULL, NULL);
            valueCheck(clientFd, "Accept error", "New client connected");

            this->handleConnection(clientFd);
        }
    }

};