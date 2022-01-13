#include <map>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <csignal>
#include <algorithm>

#include "data.h"
#include "threadData.h"

#pragma once

class Server{
public:
    int QUEUE_SIZE;
    int APPLICATION_PORT;
    int MAX_CONNECTED_CLIENTS;
    std::string APPLICATION_HOST;
    int MAX_THREAD_NUMBER;

    Data data;

    std::vector<std::thread> threads;
    std::vector<ThreadData*> threadData;

    typedef std::string (Data::*function)(std::string);
    std::map<std::string, function> functionMap;

    int nFoo;
    int serverSocket;
    sockaddr_in server_addr = {0};

    Server( const int& app_port, const std::string& app_host, 
            const int& queue_size = 5, const int& max_connected_clients = 3){

        this->APPLICATION_PORT      = app_port;
        this->APPLICATION_HOST      = app_host;
        this->QUEUE_SIZE            = queue_size;
        this->MAX_CONNECTED_CLIENTS = max_connected_clients;
        this->MAX_THREAD_NUMBER     = max_connected_clients / MAX_DESCRIPTORS_NUM;

        this->threads.reserve(MAX_THREAD_NUMBER);
        this->threadData.reserve(MAX_THREAD_NUMBER);

        server_addr.sin_family      = AF_INET;
        server_addr.sin_port        = htons(APPLICATION_PORT);
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        nFoo = 1;

        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        valueCheck(serverSocket, "Socket error", "");
        valueCheck(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&nFoo, sizeof(nFoo)), "Setsockopt error", "");
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

        functionMap[CALENDAR_GET_NAMES_PREFIX]      = &Data::getCalendars;
        functionMap[CALENDAR_GET_INFO_PREFIX]       = &Data::getCalendarInfo;        
        functionMap[CALENDAR_GET_EVENTS_PREFIX]     = &Data::getEventsForDay;
        functionMap[CALENDAR_GET_USERS_PREFIX]      = &Data::getCalendarUserList;

        functionMap[EVENT_INSERT_PREFIX]            = &Data::insertEvent;
        functionMap[EVENT_MODIFY_PREFIX]            = &Data::modifyEvent;
        functionMap[EVENT_DELETE_PREFIX]            = &Data::deleteEvent;

        std::cout << "Starting server at " << this->APPLICATION_HOST <<  " port " << this->APPLICATION_PORT << std::endl;
    }

    ~Server(){
        std::cout << "Server deconstructor called" << std::endl;
        for(size_t i = 0; i < threadData.size(); i++){
            threadData[i]->disconnect = true;
        }
        std::cout << "Waiting for threads to join" << std::endl;
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
        while(!thData->disconnect && thData->descriptorsNum > 0){
            if(poll(thData->descriptors, thData->descriptorsNum, thData->timeout) == -1){
                std::cout << "Poll error" << std::endl;
                exit(SIGINT);
            } else{
                for(int i = 0; i < thData->descriptorsNum; i++){
                    if(thData->descriptors[i].revents & POLLIN){
                        std::cout << "reading data from client..." << std::endl;
                        memset(buff, 0, BUFFER_SIZE);
                        if(read(thData->descriptors[i].fd, buff, BUFFER_SIZE) == 0){
                            data.logoutUser(thData->descriptors[i].fd);
                            thData->removeDescriptor(thData->descriptors[i].fd);
                        } else {
                            thData->addBuffer(buff, i);
                            if(thData->readyToRead(i)){
                                message = thData->getMessage(i);
                                prefix  = thData->getPrefix(message);
                                message = thData->getArguments(message, i);

                                std::map<std::string, function>::iterator func = functionMap.find(prefix);
                                if(func == functionMap.end()){
                                    std::cout << "Unknown message prefix: " << prefix << std::endl;
                                    message = FAILURE_CODE + std::string(DATA_SEPARATOR) + "Server error occured." + DATA_END;
                                }
                                else{
                                    std::cout << "MESSAGE TO FUNCTION: " << message << std::endl;
                                    message = (data.*func->second)(message) + DATA_END;
                                }
                                std::cout << "MESSAGE FROM FUNCTION: " << message << std::endl;

                                if(prefix != CLOSING_APP_PREFIX){
                                    while(thData->saveToBuff(buff, message) > 0){
                                        if(write(thData->descriptors[i].fd, buff, BUFFER_SIZE) == -1){
                                            data.logoutUser(thData->descriptors[i].fd);
                                            thData->removeDescriptor(thData->descriptors[i].fd);
                                            break;
                                        }
                                    } 
                                }
                                else if(prefix == CLOSING_APP_PREFIX) thData->removeDescriptor(thData->descriptors[i].fd);
                            }
                        }
                    }
                }
            }
            std::cout << thData << std::endl;
        }
        std::cout << "Disconnected thread..." << std::endl;
        std::cout << "Deleting thread data..." << std::endl;

        std::vector<ThreadData*>::iterator thDataPos = std::find_if(threadData.begin(), 
                                                                    threadData.end(), 
                                                                    [thData](ThreadData* p){return *p == *thData;});

        //std::vector<ThreadData*>::iterator thDataPos = std::find(threadData.begin(), threadData.end(), thData);
        if(thDataPos == threadData.end()) std::cout << "Thread Data not found in vector!!!!!!" << std::endl;    
        else threadData.erase(thDataPos);
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
        char response[BUFFER_SIZE];
        memset(response, 0, BUFFER_SIZE);
        if(threadIndex == -1){
            if((int)threads.size() * MAX_DESCRIPTORS_NUM < MAX_CONNECTED_CLIENTS){
                ThreadData* thData = new ThreadData(clientFd);
                threadData.push_back(thData);
                threads.push_back(std::thread(&Server::threadFunction, this, thData));

                std::cout << "New thread created" << std::endl;
                sprintf(response, "%s%c", SUCCESS_CODE, DATA_END);
            } else {
                sprintf(response, "%s%c", FAILURE_CODE, DATA_END);
            }
            
        } else{
            threadData[threadIndex]->newDescriptor(clientFd);
            std::cout << "New client added to thread" << std::endl;
            sprintf(response, "%s%c", SUCCESS_CODE, DATA_END);
        }
        //write(clientFd, response, BUFFER_SIZE);
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