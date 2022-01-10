#include <iostream>

#pragma once

class Event{
public:
    std::string name;
    std::string date;
    std::string hourBeginning;
    std::string* owner;
    
    Event(const std::string& name, const std::string& date, const std::string& hourBeginning, std::string* owner){
        this->name = name;
        this->date = date;
        this->hourBeginning = hourBeginning;
        this->owner = owner;
    }

    friend std::ostream& operator<<(std::ostream& os, const Event& event){
        os << "Event: " << event.name << " " << event.date << " " << event.hourBeginning;
        return os;
    }

    bool operator==(const Event& event){
        return  this->name == event.name && 
                this->date == event.date &&
                this->hourBeginning == event.hourBeginning &&
                this->owner == event.owner;
    }
};