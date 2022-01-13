#include <vector>
#include "event.h"

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
        os << "Calendar: " << calendar.name << " Owner: " << calendar.owner << std::endl;
        return os;
    }

    void changeName(const std::string& name){
        this->name = name;
    }
};