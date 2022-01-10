#include <iostream>
#include <vector>
#include <algorithm>
#include "event.h"
#include "../serverFunctions.h"

#pragma once

class Calendar{
public:
    std::string name;
    std::vector<Event> events;
    std::string* owner;
    std::vector<std::string*> userList;

    Calendar(const std::string& name, std::string* owner){
        this->name  = name;
        this->owner = owner;
    }

    bool operator==(const std::string& name){
        return this->name == name;
    }

    friend std::ostream& operator<<(std::ostream& os, const Calendar& calendar){
        os << "Calendar: " << calendar.name << " Owner: " << *(calendar.owner) << " " << calendar.owner << std::endl;
        return os;
    }

    result addEvent(Event event){
        events.push_back(event);
        return SUCCESS;
    }

    result deleteEvent(const Event& event){
        std::vector<Event>::iterator eventPosition = std::find(events.begin(), events.end(), event);
        if(eventPosition != events.end()){
            events.erase(eventPosition);
            return SUCCESS;
        }
        return FAILURE;
    }

    result editEvent(const Event& oldEvent, Event newEvent){
        std::vector<Event>::iterator eventPosition = std::find(events.begin(), events.end(), oldEvent);
        if(eventPosition != events.end()){
            *eventPosition = newEvent;
            return SUCCESS;
        }
        return FAILURE;
    }

    result addUser(std::string* name){
        if(*name != *(this->owner)){
            std::vector<std::string*>::iterator userPosition = std::find(userList.begin(), userList.end(), owner);
            if(userPosition != userList.end()){
                userList.push_back(name);
                return SUCCESS;
            }
        }
        return FAILURE;
    }

    result removeUser(const std::string* name){
        std::vector<std::string*>::iterator userPosition = std::find(userList.begin(), userList.end(), owner);
        if(userPosition != userList.end()){
            userList.erase(userPosition);
            return SUCCESS;
        }
        return FAILURE;
    }

    result changeName(const std::string& name){
        this->name = name;
        return SUCCESS;
    }
};