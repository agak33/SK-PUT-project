#include "../serverFunctions.h"

#pragma once

class User{
public:
    std::string username;
    std::string password;
    int userFd;

    User() {};

    User(const std::string& username, const std::string& password){
        this->username   = username;
        this->password   = password;
        this->userFd     = -1;
    }

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

    result logIn(const int& userFd){
        if(this->userFd != -1){
            return FAILURE;
        }
        this->userFd = userFd;
        return SUCCESS;
    }

    result logOut(){
        if(this->userFd == -1){
            return FAILURE;
        }
        this->userFd = -1;
        return SUCCESS;
    }

};