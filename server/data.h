#include "models/calendar.h"
#include "models/user.h"

#pragma once

class Data{
private:
    std::mutex insertUserMutex;
    std::mutex loginUserMutex;

    std::mutex modifyCalendarListMutex;

    User users[MAX_CLIENT_NUMBER];
    std::vector<Calendar> calendars;
    int usersNum;

    int userIndex(const std::string& name){
        for(int i = 0; i < this->usersNum; i++){
            if(users[i].username == name){
                return i;
            }
        }
        return -1;
    }

    void insertUser(const User& user){
        users[usersNum++] = user;
    }
public:
    Data(){
        this->usersNum = 0;
    }

    void displayUsers(){
        std::cout << "\nUSER LIST:\n";
        for(int i = 0; i < usersNum; i++){
            std::cout << "-> " << users[i].username << " " << &(users[i].username) << std::endl;
        }
        std::cout << std::endl;
    }

    void displayCalendars(){
        std::cout << "\nCALENDAR LIST:\n";
        for(size_t i = 0; i < calendars.size(); i++){
            std::cout << "Calendar " << i + 1 << std::endl;
            std::cout << calendars[i].name << " " << *(calendars[i].owner) << std::endl;
            std::cout << "USER LIST:\n";
            for(size_t j = 0; j < calendars[i].userList.size(); j++){
                std::cout << "-> " << *(calendars[i].userList[j]) << std::endl;
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    // USER FUNCTIONS
    std::string loginUser(std::string data){
        //std::cout << "\nloginUser function" << std::endl;
        size_t sepIndex1 = data.find(DATA_SEPARATOR);
        size_t sepIndex2 = data.find_last_of(DATA_SEPARATOR);
        if(sepIndex1 == std::string::npos || sepIndex2 == std::string::npos){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Server error";
        }

        std::string username = data.substr(0, sepIndex1);
        std::string password = data.substr(sepIndex1 + 1, sepIndex2 - sepIndex1 - 1);
        int fd = atoi(data.substr(sepIndex2 + 1).c_str());
        //std::cout << "Separated data: " << username << " " << password << " " << fd << std::endl;
        int usrIndex = userIndex(username);

        if(usrIndex == -1){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "User doesn't exists";
        } else if (users[usrIndex].password == password){
            loginUserMutex.lock();
            if(users[usrIndex].logIn(fd) == SUCCESS){
                loginUserMutex.unlock();
                return SUCCESS_CODE;
            } else{
                loginUserMutex.unlock();
                return FAILURE_CODE + std::string(DATA_SEPARATOR) + "User is currently logged in";
            }
        }
        return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Wrong password";
    }

    std::string registerUser(std::string data){
        //std::cout << "\nregisterUser function" << std::endl;
        size_t sepIndex1 = data.find(DATA_SEPARATOR);
        size_t sepIndex2 = data.find_last_of(DATA_SEPARATOR);
        if(sepIndex1 == std::string::npos || sepIndex2 == std::string::npos){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Server error";
        }

        std::string username = data.substr(0, sepIndex1);
        std::string password = data.substr(sepIndex1 + 1, sepIndex2 - sepIndex1 - 1);
        int fd = atoi(data.substr(sepIndex2 + 1).c_str());        
        //std::cout << "Separated data: " << username << " " << password << " " << fd << std::endl;
        insertUserMutex.lock();
        int usrIndex = userIndex(username);

        if(usrIndex == -1){
            insertUser(User(username, password, fd));
            insertUserMutex.unlock();
            return SUCCESS_CODE;
        }
        insertUserMutex.unlock();
        return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Such nick currently exits";
    }

    result logoutUser(int fd){
        //std::cout << "\nlogoutUser function" << std::endl;
        for(int i = 0; i < usersNum; i++){
            if(users[i].userFd == fd){
                loginUserMutex.lock();
                if(users[i].logOut() == SUCCESS){
                    loginUserMutex.unlock();
                    return SUCCESS;
                }
                loginUserMutex.unlock();
                return FAILURE;
            }
        }
        return FAILURE;
    }

    std::string logoutUser(std::string data){
        //std::cout << "\nlogoutUser function" << std::endl;
        if(data.substr(0, 1) == DATA_SEPARATOR){
            return SUCCESS_CODE;
        }

        size_t sepIndex1 = data.find(DATA_SEPARATOR);
        if(sepIndex1 == std::string::npos){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Server error";
        }

        std::string username = data.substr(0, sepIndex1);
        int fd = atoi(data.substr(sepIndex1 + 1).c_str());
        //std::cout << "Separated data: " << username << " " << fd << std::endl;

        int usrIndex = userIndex(username);
        if(usrIndex == -1){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "User doesn't exists";
        }
        if(users[usrIndex].userFd != fd){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Failed to logout user. Wrong descriptor number";
        }
        loginUserMutex.lock();
        if(users[usrIndex].logOut() == FAILURE){
            loginUserMutex.unlock();
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "User is currently logged out";
        }
        loginUserMutex.unlock();
        return SUCCESS_CODE;
    }

    // CALENDAR FUNCTIONS
    std::string insertCalendar(std::string data){
        //std::cout << "\ninsertCalendar function" << std::endl;
        size_t sepIndex1 = data.find(DATA_SEPARATOR);
        size_t sepIndex2 = data.find(DATA_SEPARATOR, sepIndex1 + 1);
        if(sepIndex1 == std::string::npos){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Server error";
        }

        std::string calendarName  = data.substr(0, sepIndex1);
        std::string calendarOwner;
        if(sepIndex2 == std::string::npos){
            calendarOwner = data.substr(sepIndex1 + 1);
        } else{
            calendarOwner = data.substr(sepIndex1 + 1, sepIndex2 - sepIndex1 - 1);
        }
        //std::cout << "Separated data: " << calendarName << " " << calendarOwner << std::endl;

        modifyCalendarListMutex.lock();
        std::vector<Calendar>::iterator calendarPos = std::find(calendars.begin(), calendars.end(), calendarName);
        if(calendarPos != calendars.end()){
            modifyCalendarListMutex.unlock();
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Such calendar name exists";
        }

        int usrIndex = userIndex(calendarOwner);
        if(usrIndex == -1){
            modifyCalendarListMutex.unlock();
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "User doesn't exists";
        }
        calendars.push_back(Calendar(calendarName, &(users[usrIndex].username)));
        modifyCalendarListMutex.unlock();
        calendarPos = std::find(calendars.begin(), calendars.end(), calendarName);
        if(calendarPos == calendars.end()){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Server error";
        }

        if(sepIndex2 != std::string::npos){
            data = data.substr(sepIndex2 + 1);
            //std::cout << data << std::endl;
            std::string username;
            std::string notFoundUsers = "";

            sepIndex1 = data.find(DATA_SEPARATOR);
            while(sepIndex1 != std::string::npos){
                username = data.substr(0, sepIndex1);
                usrIndex = userIndex(username);
                if(usrIndex == -1 || username == calendarOwner){
                    notFoundUsers += username + ", ";
                } else{
                    (*calendarPos).userList.push_back(&(users[usrIndex].username));
                }
                data = data.substr(sepIndex1 + 1);
                sepIndex1 = data.find(DATA_SEPARATOR);
            }

            usrIndex = userIndex(data);
            if(usrIndex == -1 || data == calendarOwner){
                notFoundUsers += data + ", ";
            } else{
                (*calendarPos).userList.push_back(&(users[usrIndex].username));
            }
            if(notFoundUsers.empty()) return SUCCESS_CODE;
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Calendar has been created. Failed to add users: " + notFoundUsers.substr(0, notFoundUsers.size() - 2);
        }
        return SUCCESS_CODE;
    }

    std::string modifyCalendarName(std::string data){
        //std::cout << "\nmodifyCalendarName function" << std::endl;
        size_t sepIndex1 = data.find(DATA_SEPARATOR);
        size_t sepIndex2 = data.find(DATA_SEPARATOR, sepIndex1 + 1);
        if(sepIndex1 == std::string::npos || sepIndex2 == std::string::npos){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Server error";
        }

        std::string oldCalendarName = data.substr(0, sepIndex1);
        std::string newCalendarName = data.substr(sepIndex1 + 1, sepIndex2 - sepIndex1 - 1);
        std::string username        = data.substr(sepIndex2 + 1);

        //std::cout << "Separated data: " << oldCalendarName << " " << newCalendarName << " " << username << std::endl;
        std::vector<Calendar>::iterator calendarPos = std::find(calendars.begin(), calendars.end(), oldCalendarName);
        if(calendarPos == calendars.end()){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Calendar doesn't exists. Try to refresh calendar list.";
        }
        if(*(calendarPos->owner) != username){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "You don't have permission to perform this operation";
        }
        modifyCalendarListMutex.lock();
        std::vector<Calendar>::iterator calendarPos1 = std::find(calendars.begin(), calendars.end(), newCalendarName);
        if(calendarPos1 != calendars.end()){
            modifyCalendarListMutex.unlock();
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Such calendar name already exists";
        }
        calendarPos->changeName(newCalendarName);
        modifyCalendarListMutex.unlock();
        return SUCCESS_CODE;
    }

    std::string deleteCalendarUser(std::string data){
        //std::cout << "\ndeleteCalendarUser function" << std::endl;
        size_t sepIndex1 = data.find(DATA_SEPARATOR);
        size_t sepIndex2 = data.find(DATA_SEPARATOR, sepIndex1 + 1);
        if(sepIndex1 == std::string::npos || sepIndex2 == std::string::npos){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Server error";
        }

        std::string calendarName  = data.substr(0, sepIndex1);
        std::string userToDelete  = data.substr(sepIndex1 + 1, sepIndex2 - sepIndex1 - 1);
        std::string calendarOwner = data.substr(sepIndex2 + 1);

        //std::cout << "Separated data: " << calendarName << " " << userToDelete << " " << calendarOwner << std::endl;
        std::vector<Calendar>::iterator calendarPos = std::find(calendars.begin(), calendars.end(), calendarName);
        if(calendarPos == calendars.end()){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Calendar doesn't exists. Try to refresh calendar list.";
        }
        if(*(calendarPos->owner) != calendarOwner){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Operation not permitted";
        }
        for(size_t i = 0; i < calendarPos->userList.size(); i++){
            if(*(calendarPos->userList[i]) == userToDelete){
                calendarPos->userList.erase(calendarPos->userList.begin() + i);
                return SUCCESS_CODE;
            }
        }
        return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Failed to delete user";
    }

    std::string insertCalendarUser(std::string data){
        //std::cout << "\ninsertCalendarUser function" << std::endl;
        size_t sepIndex1 = data.find(DATA_SEPARATOR);
        size_t sepIndex2 = data.find(DATA_SEPARATOR, sepIndex1 + 1);
        if(sepIndex1 == std::string::npos || sepIndex2 == std::string::npos){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Server error";
        }

        std::string calendarName  = data.substr(0, sepIndex1);
        std::string userToInsert  = data.substr(sepIndex1 + 1, sepIndex2 - sepIndex1 - 1);
        std::string calendarOwner = data.substr(sepIndex2 + 1);

        //std::cout << "Separated data: " << calendarName << " " << userToInsert << " " << calendarOwner << std::endl;

        std::vector<Calendar>::iterator calendarPos = std::find(calendars.begin(), calendars.end(), calendarName);
        if(calendarPos == calendars.end()){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Calendar doesn't exists. Try to refresh calendar list.";
        }
        if(*(calendarPos->owner) != calendarOwner){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Operation not permitted";
        }

        int usrIndex = userIndex(userToInsert);
        if(usrIndex == -1){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "User doesn't exists";
        }

        std::vector<std::string*>::iterator userPosCal = std::find(calendarPos->userList.begin(), calendarPos->userList.end(), &(users[usrIndex].username));
        if(userPosCal != calendarPos->userList.end() || *(calendarPos->owner) == userToInsert){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "User currently has access to this calendar";
        }
        calendarPos->userList.push_back(&(users[usrIndex].username));
        return SUCCESS_CODE;
    }

    std::string deleteCalendar(std::string data){
        //std::cout << "\ndeleteCalendar function" << std::endl;
        size_t sepIndex1 = data.find(DATA_SEPARATOR);
        if(sepIndex1 == std::string::npos){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Server error";
        }

        std::string calendarName   = data.substr(0, sepIndex1);
        std::string calendarOwner  = data.substr(sepIndex1 + 1);

        //std::cout << "Separated data: " << calendarName << " " << calendarOwner << std::endl;
        std::vector<Calendar>::iterator calendarPos = std::find(calendars.begin(), calendars.end(), calendarName);
        if(calendarPos == calendars.end()){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Calendar doesn't exists. Try to refresh calendar list.";
        }
        if(*(calendarPos->owner) != calendarOwner){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Operation not permitted";
        }
        modifyCalendarListMutex.lock();
        calendars.erase(calendarPos);
        modifyCalendarListMutex.unlock();
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
        int usrIndex = userIndex(data);
        if(usrIndex == -1){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "User doesn't exists";
        }

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
        return response;
    }

    std::string getCalendarUserList(std::vector<Calendar>::iterator calendarPos){
        std::string response = SUCCESS_CODE;
        for(size_t i = 0; i < calendarPos->userList.size(); i++){
            response += DATA_SEPARATOR + *(calendarPos->userList[i]);
        }
        return response;
    }

    std::string getCalendarUserList(std::string data){
        std::vector<Calendar>::iterator calendarPos = std::find(calendars.begin(), calendars.end(), data);
        if(calendarPos == calendars.end()){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Calendar not found";
        }
        return this->getCalendarUserList(calendarPos);
    }

    std::string getCalendarInfo(std::string data){
        std::vector<Calendar>::iterator calendarPos = std::find(calendars.begin(), calendars.end(), data);
        if(calendarPos == calendars.end()){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Calendar not found";
        }

        std::string userList = this->getCalendarUserList(calendarPos);
        if(userList.size() < 3){
            return SUCCESS_CODE +   std::string(DATA_SEPARATOR) + calendarPos->name + 
                                    std::string(DATA_SEPARATOR) + *(calendarPos->owner);
        }
        return SUCCESS_CODE +   std::string(DATA_SEPARATOR) + calendarPos->name + 
                                std::string(DATA_SEPARATOR) + *(calendarPos->owner) +
                                std::string(DATA_SEPARATOR) + userList.substr(2);
        
    }

    // EVENT FUNCTIONS
    std::string insertEvent(std::string data){
        size_t sepIndex[4];
        sepIndex[0] = data.find(DATA_SEPARATOR);
        if(sepIndex[0] == std::string::npos) return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Server error";
        for(size_t i = 1; i < 4; i++){
            sepIndex[i] = data.find(DATA_SEPARATOR, sepIndex[i - 1] + 1);
            if(sepIndex[i] == std::string::npos) return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Server error";
        }

        std::string calendarName = data.substr(0, sepIndex[0]);
        std::vector<Calendar>::iterator calendarPos = std::find(calendars.begin(), calendars.end(), calendarName);
        if(calendarPos == calendars.end()){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Calendar doesn't exists. Try to refresh calendar list.";
        }

        std::string username   = data.substr(sepIndex[0] + 1, sepIndex[1] - sepIndex[0] - 1);
        std::string eventName  = data.substr(sepIndex[1] + 1, sepIndex[2] - sepIndex[1] - 1);
        std::string eventDate  = data.substr(sepIndex[2] + 1, sepIndex[3] - sepIndex[2] - 1);
        std::string eventHour  = data.substr(sepIndex[3] + 1);

        //std::cout << "Separated data: " << username << " " << eventDate << " " << eventName << " " << eventHour << std::endl;

        Event event = Event(eventName, eventDate, eventHour, &username);
        std::vector<Event>::iterator eventPos = std::find(calendarPos->events.begin(), calendarPos->events.end(), event);
        if(eventPos != calendarPos->events.end()){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Such event already exists";
        }
        int usrIndex = userIndex(username);
        if(usrIndex == -1){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "User doesn't exists";
        }
        calendarPos->events.push_back(Event(eventName, eventDate, eventHour, &(users[usrIndex].username)));
        return SUCCESS_CODE;
    }

    std::string modifyEvent(std::string data){
        size_t sepIndex[8];
        sepIndex[0] = data.find(DATA_SEPARATOR);
        if(sepIndex[0] == std::string::npos) return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Server error";
        for(size_t i = 1; i < 8; i++){
            sepIndex[i] = data.find(DATA_SEPARATOR, sepIndex[i - 1] + 1);
            if(sepIndex[i] == std::string::npos) return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Server error";
        }

        std::string calendarName = data.substr(0, sepIndex[0]);
        std::vector<Calendar>::iterator calendarPos = std::find(calendars.begin(), calendars.end(), calendarName);
        if(calendarPos == calendars.end()){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Calendar doesn't exists. Try to refresh calendar list.";
        }

        std::string username   = data.substr(sepIndex[0] + 1, sepIndex[1] - sepIndex[0] - 1);

        //std::cout << calendarName << " " << username << std::endl;

        std::string eventName  = data.substr(sepIndex[1] + 1, sepIndex[2] - sepIndex[1] - 1);
        std::string eventDate  = data.substr(sepIndex[2] + 1, sepIndex[3] - sepIndex[2] - 1);
        std::string eventHour  = data.substr(sepIndex[3] + 1, sepIndex[4] - sepIndex[3] - 1);
        std::string eventOwner = data.substr(sepIndex[4] + 1, sepIndex[5] - sepIndex[4] - 1);

        //std::cout << eventName << " " << eventDate << " " << eventHour << " " << eventOwner << std::endl;

        Event event = Event(eventName, eventDate, eventHour, &eventOwner);
        std::vector<Event>::iterator eventPos = std::find(calendarPos->events.begin(), calendarPos->events.end(), event);
        if(eventPos == calendarPos->events.end()){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Event doesn't exists. Try to refresh event list.";
        }
        if(*(eventPos->owner) != *(calendarPos->owner) && *(eventPos->owner) != eventOwner){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Operation not permitted";
        }
        eventName = data.substr(sepIndex[5] + 1, sepIndex[6] - sepIndex[5] - 1);
        eventDate = data.substr(sepIndex[6] + 1, sepIndex[7] - sepIndex[6] - 1);
        eventHour = data.substr(sepIndex[7] + 1);

        Event modifiedEvent = Event(eventName, eventDate, eventHour, &eventOwner);
        std::vector<Event>::iterator eventPos1 = std::find(calendarPos->events.begin(), calendarPos->events.end(), modifiedEvent);
        if(eventPos1 != calendarPos->events.end()){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Such event already exists";
        }
        eventPos->name = eventName;
        eventPos->date = eventDate;
        eventPos->hourBeginning = eventHour;

        return SUCCESS_CODE;
    }

    std::string deleteEvent(std::string data){
        size_t sepIndex[5];
        sepIndex[0] = data.find(DATA_SEPARATOR);
        if(sepIndex[0] == std::string::npos) return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Server error";
        for(size_t i = 1; i < 5; i++){
            sepIndex[i] = data.find(DATA_SEPARATOR, sepIndex[i - 1] + 1);
            if(sepIndex[i] == std::string::npos) return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Server error";
        }

        std::string calendarName = data.substr(0, sepIndex[0]);
        std::vector<Calendar>::iterator calendarPos = std::find(calendars.begin(), calendars.end(), calendarName);
        if(calendarPos == calendars.end()){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Calendar doesn't exists. Try to refresh calendar list.";
        }
        std::string username   = data.substr(sepIndex[0] + 1, sepIndex[1] - sepIndex[0] - 1);

        //std::cout << calendarName << " " << username << std::endl;

        std::string eventName  = data.substr(sepIndex[1] + 1, sepIndex[2] - sepIndex[1] - 1);
        std::string eventDate  = data.substr(sepIndex[2] + 1, sepIndex[3] - sepIndex[2] - 1);
        std::string eventHour  = data.substr(sepIndex[3] + 1, sepIndex[4] - sepIndex[3] - 1);
        std::string eventOwner = data.substr(sepIndex[4] + 1);

        //std::cout << eventName << " " << eventDate << " " << eventHour << " " << eventOwner << std::endl;

        Event event = Event(eventName, eventDate, eventHour, &eventOwner);
        std::vector<Event>::iterator eventPos = std::find(calendarPos->events.begin(), calendarPos->events.end(), event);
        if(eventPos == calendarPos->events.end()){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Event doesn't exists. Try to refresh event list.";
        }
        if(*(eventPos->owner) != *(calendarPos->owner) && *(eventPos->owner) != eventOwner){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Operation not permitted";
        }
        calendarPos->events.erase(eventPos);
        return SUCCESS_CODE;
    }

    std::string getEventsForDay(std::string data){
        size_t sepIndex1 = data.find(DATA_SEPARATOR);
        if(sepIndex1 == std::string::npos){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Server error";
        }

        std::string calendarName  = data.substr(0, sepIndex1);
        std::string date          = data.substr(sepIndex1 + 1);

        std::vector<Calendar>::iterator calendarPos = std::find(calendars.begin(), calendars.end(), calendarName);
        if(calendarPos == calendars.end()){
            return FAILURE_CODE + std::string(DATA_SEPARATOR) + "Calendar doesn't exists. Try to refresh calendar list.";
        }

        //std::cout << "Separated data: " << calendarName << " " << date << std::endl;

        std::string response = SUCCESS_CODE + std::string(DATA_SEPARATOR) + date;
        for(size_t i = 0; i < calendarPos->events.size(); i++){
            if(calendarPos->events[i].date == date){
                response += std::string(DATA_SEPARATOR) + calendarPos->events[i].name +
                            std::string(DATA_SEPARATOR) + calendarPos->events[i].hourBeginning +
                            std::string(DATA_SEPARATOR) + *(calendarPos->events[i].owner);
            }
        }
        return response;
    }
};