#include <vector>
#include <iostream>
#include "models/calendar.h"
#include "models/user.h"

#pragma once

class Data{
public:
    std::vector<User> users;
    std::vector<Calendar> calendars;

    void displayUsers(){
        std::cout << "\nUSER LIST:\n";
        for(size_t i = 0; i < users.size(); i++){
            std::cout << users[i].username << " " << users[i].password << " " << users[i].userFd << std::endl;
        }
        std::cout << std::endl;
    }

    void displayCalendars(){
        std::cout << "\nCALENDAR LIST:\n";
        for(size_t i = 0; i < calendars.size(); i++){
            std::cout << calendars[i].name << " " << *(calendars[i].owner) << std::endl;
            std::cout << "USER LIST:\n";
            for(size_t j = 0; j < calendars[i].userList.size(); j++){
                std::cout << "-> " << *(calendars[i].userList[j]) << std::endl;
            }
        }
        std::cout << std::endl;
    }

    // USER FUNCTIONS
    std::string loginUser(std::string data){
        std::cout << "\nloginUser function" << std::endl;
        size_t sepIndex1 = data.find(MESSAGE_SEPARATOR);
        size_t sepIndex2 = data.find(MESSAGE_SEPARATOR, sepIndex1 + 1);
        if(sepIndex1 == std::string::npos || sepIndex2 == std::string::npos){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Server error";
        }

        std::string username = data.substr(0, sepIndex1);
        std::string password = data.substr(sepIndex1 + 1, sepIndex2 - sepIndex1 - 1);
        int fd = atoi(data.substr(sepIndex2 + 1).c_str());
        std::cout << "Separated data: " << username << " " << password << " " << fd << std::endl;
        for(size_t i = 0; i < this->users.size(); i++){
            if(users[i] == username){
                if(users[i].password == password){
                    if(users[i].logIn(fd) == SUCCESS){
                        std::cout << "Logged user: " << users[i];
                        return SUCCESS_CODE;
                    }
                    else{
                        return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "User is currently logged in";
                    }
                } else{
                    return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Wrong password";
                }
            }
        }
        return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "User not exists";
    }

    std::string registerUser(std::string data){
        std::cout << "\nregisterUser function" << std::endl;
        size_t sepIndex1 = data.find(MESSAGE_SEPARATOR);
        size_t sepIndex2 = data.find(MESSAGE_SEPARATOR, sepIndex1 + 1);
        if(sepIndex1 == std::string::npos || sepIndex2 == std::string::npos){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Server error";
        }

        std::string username = data.substr(0, sepIndex1);
        std::string password = data.substr(sepIndex1 + 1, sepIndex2 - sepIndex1 - 1);
        int fd = atoi(data.substr(sepIndex2 + 1).c_str());        
        std::cout << "Separated data: " << username << " " << password << " " << fd << std::endl;

        std::vector<User>::iterator userPos = std::find(users.begin(), users.end(), username);
        if(userPos != users.end()){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Such nick currently exits";
        }

        User newUser = User(username, password, fd);
        users.push_back(newUser);

        std::cout << "Created user: " << newUser << std::endl;
        return SUCCESS_CODE;
    }

    std::string logoutUser(std::string data){
        std::cout << "\nlogoutUser function" << std::endl;
        if(data.substr(0, 1) == MESSAGE_SEPARATOR){
            return SUCCESS_CODE;
        }

        size_t sepIndex1 = data.find(MESSAGE_SEPARATOR);
        if(sepIndex1 == std::string::npos){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Server error";
        }

        std::string username = data.substr(0, sepIndex1);
        int fd = atoi(data.substr(sepIndex1 + 1).c_str());
        std::cout << "Separated data: " << username << " " << fd << std::endl;
        std::vector<User>::iterator userIndex = std::find(users.begin(), users.end(), username);
        if(userIndex == users.end()){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "User is not exists";
        }
        if(userIndex->userFd != fd){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Wrong descriptor number";
            std::cout << userIndex->userFd << " " << fd << std::endl;
        }
        if(userIndex->logOut() == FAILURE){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "User is currently logged out";
        }
        return SUCCESS_CODE;
    }

    // CALENDAR FUNCTIONS
    std::string insertCalendar(std::string data){
        std::cout << "\ninsertCalendar function" << std::endl;
        size_t sepIndex1 = data.find(MESSAGE_SEPARATOR);
        size_t sepIndex2 = data.find(MESSAGE_SEPARATOR, sepIndex1 + 1);
        if(sepIndex1 == std::string::npos){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Server error";
        }

        std::string calendarName  = data.substr(0, sepIndex1);
        std::string calendarOwner;
        if(sepIndex2 == std::string::npos){
            calendarOwner = data.substr(sepIndex1 + 1);
        } else{
            calendarOwner = data.substr(sepIndex1 + 1, sepIndex2 - sepIndex1 - 1);
        }        
        std::cout << "Separated data: " << calendarName << " " << calendarOwner << std::endl;

        std::vector<Calendar>::iterator calendarPos = std::find(calendars.begin(), calendars.end(), calendarName);
        if(calendarPos != calendars.end()){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Such calendar name exists";
        }

        std::vector<User>::iterator userPos = std::find(users.begin(), users.end(), calendarOwner);
        if(userPos == users.end()){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "User not exists";
        }

        calendars.push_back(Calendar(calendarName, &(userPos->username)));
        // int index = -1;
        // for(int i = calendars.size() - 1; i >= 0; i--){
        //     if(calendars[i].name == calendarName){
        //         index = i;
        //         break;
        //     }
        // }
        // if(index == -1) return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Server error";
        ////////////////////////////
        calendarPos = std::find(calendars.begin(), calendars.end(), calendarName);
        if(calendarPos == calendars.end()){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Server error";
        }

        //userPos->userCalendars.push_back(&(calendars[index]));

        if(sepIndex2 != std::string::npos){
            std::cout << "Adding users from list..." << std::endl;
            data = data.substr(sepIndex2 + 1);
            std::string username;
            std::string notFoundUsers = "";

            sepIndex1 = data.find(MESSAGE_SEPARATOR);
            while(sepIndex1 != std::string::npos){
                username = data.substr(0, sepIndex1);
                std::cout << "Username: " << username << std::endl;
                userPos = std::find(users.begin(), users.end(), username);

                if(userPos == users.end()){
                    notFoundUsers += username + ", ";
                } else{
                    (*calendarPos).userList.push_back(&(userPos->username));
                }
                data = data.substr(sepIndex1 + 1);
                sepIndex1 = data.find(MESSAGE_SEPARATOR);
            }
            std::cout << "Username: " << data << std::endl;
            userPos = std::find(users.begin(), users.end(), data);

            if(userPos == users.end()){
                notFoundUsers += data + ", ";
            } else{
                (*calendarPos).userList.push_back(&(userPos->username));
            }

            if(notFoundUsers.empty()) return SUCCESS_CODE;

            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Failed to add users: " + notFoundUsers.substr(0, notFoundUsers.size() - 2);
        }
        return SUCCESS_CODE;
    }

    std::string modifyCalendarName(std::string data){
        size_t sepIndex1 = data.find(MESSAGE_SEPARATOR);
        size_t sepIndex2 = data.find(MESSAGE_SEPARATOR, sepIndex1 + 1);
        if(sepIndex1 == std::string::npos || sepIndex2 == std::string::npos){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Server error";
        }

        std::string oldCalendarName = data.substr(0, sepIndex1);
        std::string newCalendarName = data.substr(sepIndex1 + 1, sepIndex2 - sepIndex1 - 1);
        std::string username        = data.substr(sepIndex2 + 1);

        std::cout << "Separated data: " << oldCalendarName << " " << newCalendarName << " " << username << std::endl;
        std::vector<Calendar>::iterator calendarPos = std::find(calendars.begin(), calendars.end(), oldCalendarName);
        if(calendarPos == calendars.end()){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Calendar not exists";
        }
        if(*(calendarPos->owner) != username){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "You don't have permission to perform this operation";
        }
        calendarPos->changeName(newCalendarName);
        return SUCCESS_CODE;
    }

    std::string deleteCalendarUser(std::string data){
        size_t sepIndex1 = data.find(MESSAGE_SEPARATOR);
        size_t sepIndex2 = data.find(MESSAGE_SEPARATOR, sepIndex1 + 1);
        if(sepIndex1 == std::string::npos || sepIndex2 == std::string::npos){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Server error";
        }

        std::string calendarName  = data.substr(0, sepIndex1);
        std::string userToDelete  = data.substr(sepIndex1 + 1, sepIndex2 - sepIndex1 - 1);
        std::string calendarOwner = data.substr(sepIndex2 + 1);

        std::cout << "Separated data: " << calendarName << " " << userToDelete << " " << calendarOwner << std::endl;
        std::vector<Calendar>::iterator calendarPos = std::find(calendars.begin(), calendars.end(), calendarName);
        if(calendarPos == calendars.end()){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Calendar not exists";
        }
        if(*(calendarPos->owner) != calendarOwner){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Operation not permitted";
        }
        for(size_t i = 0; i < calendarPos->userList.size(); i++){
            if(*(calendarPos->userList[i]) == userToDelete){
                calendarPos->userList.erase(calendarPos->userList.begin() + i);
                return SUCCESS_CODE;
            }
        }
        return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Failed to delete user";
    }

    std::string insertCalendarUser(std::string data){
        size_t sepIndex1 = data.find(MESSAGE_SEPARATOR);
        size_t sepIndex2 = data.find(MESSAGE_SEPARATOR, sepIndex1 + 1);
        size_t sepIndex3 = data.find(MESSAGE_SEPARATOR, sepIndex2 + 1);
        if(sepIndex1 == std::string::npos || sepIndex2 == std::string::npos || sepIndex3 == std::string::npos){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Server error";
        }

        std::string calendarName  = data.substr(0, sepIndex1);
        std::string userToInsert  = data.substr(sepIndex1 + 1, sepIndex2 - sepIndex1 + 1);
        std::string calendarOwner = data.substr(sepIndex2 + 1, sepIndex3 - sepIndex2 + 1);

        std::vector<Calendar>::iterator calendarPos = std::find(calendars.begin(), calendars.end(), calendarName);
        if(calendarPos == calendars.end()){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Calendar not exists";
        }
        if(*(calendarPos->owner) != calendarOwner){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Operation not permitted";
        }
        
        std::vector<User>::iterator userPos = std::find(users.begin(), users.end(), userToInsert);
        if(userPos == users.end()){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "User not exists";
        }

        std::vector<std::string*>::iterator userPosCal = std::find(calendarPos->userList.begin(), calendarPos->userList.end(), &(userPos->username));
        if(userPosCal != calendarPos->userList.end()){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "User has access to calendar";
        }
        calendarPos->userList.push_back(&(userPos->username));
        return SUCCESS_CODE;
    }

    std::string deleteCalendar(std::string data){
        size_t sepIndex1 = data.find(MESSAGE_SEPARATOR);
        if(sepIndex1 == std::string::npos){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Server error";
        }

        std::string calendarName   = data.substr(0, sepIndex1);
        std::string calendarOwner  = data.substr(sepIndex1 + 1);

        std::cout << "Separated data: " << calendarName << " " << calendarOwner << std::endl;
        std::vector<Calendar>::iterator calendarPos = std::find(calendars.begin(), calendars.end(), calendarName);
        if(calendarPos == calendars.end()){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Calendar not exists";
        }
        if(*(calendarPos->owner) != calendarOwner){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Operation not permitted";
        }

        std::vector<User>::iterator userPos = std::find(users.begin(), users.end(), *(calendarPos->owner));
        if(userPos == users.end()){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Owner not exists";
        }
        // if(userPos->deleteCalendar(calendarName) == FAILURE){
        //     std::cout << "WARNING: Failed to delete calendar owner" << std::endl;
        // }

        // for(size_t i = 0; i < calendarPos->userList.size(); i++){
        //     userPos = std::find(users.begin(), users.end(), *(calendarPos->userList[i]));
        //     if(userPos == users.end()){
        //         std::cout << "WARINING: Failed to delete calendar user" << std::endl;
        //     }
        //     if(userPos->deleteCalendar(calendarName) == FAILURE){
        //         std::cout << "WARNING: Failed to delete calendar owner" << std::endl;
        //     }
        // }
        calendars.erase(calendarPos);
        return SUCCESS_CODE;
    }

    void displayCalendarsAddresses(){
        std::cout << "\nCALENDAR ADDRESSES:\n";
        for(size_t i = 0; i < calendars.size(); i++){
            std::cout << "-> " << &(calendars[i]) << std::endl;
        }
        std::cout << std::endl;
    }

    std::string getCalendars(std::string data){
        //this->displayCalendarsAddresses();
        std::vector<User>::iterator userPos = std::find(users.begin(), users.end(), data);
        if(userPos == users.end()){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "User not exists";
        }
        // std::string response = SUCCESS_CODE;
        // for(size_t i = 0; i < userPos->userCalendars.size(); i++){
        //     std::cout << "-> " /*<< userPos->userCalendars[i]->name << "    "*/ << userPos->userCalendars[i] << std::endl;
        //     response += DATA_SEPARATOR;
        // }
        // std::cout << userPos->userCalendars.size() << " " << response << std::endl;
        // return response;

        std::string response = SUCCESS_CODE;
        for(size_t i = 0; i < calendars.size(); i++){
            if(*(calendars[i].owner) == data){
                response += DATA_SEPARATOR + calendars[i].name;
            } else{
                for(size_t j = 0; j < calendars[i].userList.size(); j++){
                    if(*(calendars[i].userList[j]) == data){
                        response += DATA_SEPARATOR + calendars[i].name;
                    }
                }
            }
        }
        std::cout << response << std::endl;
        return response;
    }

    std::string getCalendarInfo(std::string data){
        std::vector<Calendar>::iterator calendarPos = std::find(calendars.begin(), calendars.end(), data);
        if(calendarPos == calendars.end()){
            return FAILURE_CODE + std::string(MESSAGE_SEPARATOR) + "Calendar not found";
        }

        std::string response =  SUCCESS_CODE + std::string(DATA_SEPARATOR) + 
                                calendarPos->name + std::string(DATA_SEPARATOR) + *(calendarPos->owner);
        for(size_t i = 0; i < calendarPos->userList.size(); i++){
            response += DATA_SEPARATOR + *(calendarPos->userList[i]);
        }
        return response;
    }
    
    // EVENT FUNCTIONS
    std::string insertEvent(std::string data){
        return "";
    }

    std::string modifyEvent(std::string data){
        return "";
    }

    std::string deleteEvent(std::string data){
        return "";
    }
};