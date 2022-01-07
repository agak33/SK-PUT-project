#include <vector>
#include <iostream>
#include "models/calendar.h"
#include "models/user.h"

#pragma once

class Data{
    std::vector<User> users;
    std::vector<Calendar> calendars;
public:
    std::string loginUser(std::string data){
        return "0";
    }

    std::string registerUser(std::string data){
        return "0";
    }

    std::string logoutUser(std::string data){
        return "";
    }

    std::string insertCalendar(std::string data){
        return "";
    }

    std::string modifyCalendar(std::string data){
        return "";
    }

    std::string deleteCalendar(std::string data){
        return "";
    }

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