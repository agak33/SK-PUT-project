#include <iostream>
#pragma once

#define DATA_SEPARATOR          ";"

#define LOGIN_PREFIX            "L"
#define REGISTER_PREFIX         "R"
#define LOGOUT_PREFIX           "LO"

#define CALENDAR_INSERT_PREFIX  "CI"
#define CALENDAR_MODIFY_PREFIX  "CM"
#define CALENDAR_DELETE_PREFIX  "CD"

#define EVENT_INSERT_PREFIX     "EI"
#define EVENT_MODIFY_PREFIX     "EM"
#define EVENT_DELETE_PREFIX     "ED"

enum result{SUCCESS, FAILURE};

static void valueCheck(int value, std::string messageFailure, std::string messageSuccess){
    if(value == -1){
        std::cout << messageFailure << std::endl;
        exit(1);
    }
    if(!messageSuccess.empty()) std::cout << messageSuccess << std::endl;
}
