#include <iostream>
#include "calendar.h"
#include "../serverFunctions.h"

#pragma once

enum status{LOGGED_IN, LOGGED_OUT};

class User{
public:
    std::string username;
    std::string password;

    status userStatus;
    int userFd;

    std::vector<Calendar*> userCalendars;

    // used while creating a new user with read a file
    User(const std::string& username, const std::string& password){
        this->username   = username;
        this->password   = password;
        this->userStatus = LOGGED_OUT;
    }

    // used while user is logging or registering
    User(const std::string& username, const std::string& password, const int userFd){
        this->username   = username;
        this->password   = password;
        this->userStatus = LOGGED_IN;
        this->userFd     = userFd;
    }

    bool operator==(const std::string& username){
        return this->username == username;
    }
    bool operator==(const User& user){
        return this->username == user.username;
    }

    friend std::ostream& operator<<(std::ostream& os, const User& user){
        os << "User: " << user.username;
        return os;
    }

    // used while user exists in data structure, but it's logged out
    result logIn(const int& userFd){
        if(this->userStatus == LOGGED_IN){
            return FAILURE;
        }
        this->userStatus = LOGGED_IN;
        this->userFd = userFd;
        return SUCCESS;
    }

    // used while user logged out or closed the main window
    result logOut(){
        if(this->userStatus == LOGGED_OUT){
            return FAILURE;
        }
        this->userStatus = LOGGED_OUT;
        return SUCCESS;
    }

};