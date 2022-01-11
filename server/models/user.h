#include <iostream>
#include "calendar.h"
#include "../serverFunctions.h"

#pragma once

class User{
public:
    std::string username;
    std::string password;
    int userFd;
    //std::vector<Calendar*> userCalendars;

    User() {};
    // used while creating a new user with read a file
    User(const std::string& username, const std::string& password){
        this->username   = username;
        this->password   = password;
        this->userFd     = -1;
    }

    // used while user is registering
    User(const std::string& username, const std::string& password, const int userFd){
        this->username   = username;
        this->password   = password;
        this->userFd     = userFd;
    }

    bool operator==(const std::string& username){
        return this->username == username;
    }
    bool operator==(const User& user){
        return this->username == user.username;
    }

    friend std::ostream& operator<<(std::ostream& os, const User& user){
        os << "User: " << user.username << " " << user.password << " " <<  user.userFd << std::endl;
        return os;
    }

    // used while user exists in data structure, but it's logged out
    result logIn(const int& userFd){
        if(this->userFd != -1){
            return FAILURE;
        }
        this->userFd = userFd;
        return SUCCESS;
    }

    // used while user logged out or closed the main window
    result logOut(){
        if(this->userFd == -1){
            return FAILURE;
        }
        this->userFd = -1;
        return SUCCESS;
    }

    // result deleteCalendar(const std::string& name){
    //     for(size_t i = 0; i < this->userCalendars.size(); i++){
    //         if(this->userCalendars[i]->name == name){
    //             this->userCalendars.erase(this->userCalendars.begin() + 1);
    //             return SUCCESS;
    //         }
    //     }
    //     return FAILURE;
    // }

};