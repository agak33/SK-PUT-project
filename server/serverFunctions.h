#include <iostream>
#pragma once

#define DATA_SEPARATOR          ";"

#define LOGIN_PREFIX            "L"
#define REGISTER_PREFIX         "R"
#define LOGOUT_PREFIX           "LO"
#define CLOSING_APP_PREFIX      "LC"

#define CALENDAR_INSERT_PREFIX  "CI"
#define CALENDAR_MODIFY_PREFIX  "CM"
#define CALENDAR_DELETE_PREFIX  "CD"

#define CALENDAR_INSERT_USER_PREFIX "CIU"
#define CALENDAR_DELETE_USER_PREFIX "CDU"
#define CALENDAR_GET_NAMES          "CGN"
#define CALENDAR_GET_CALENDAR_INFO  "CGI"

#define EVENT_INSERT_PREFIX     "EI"
#define EVENT_MODIFY_PREFIX     "EM"
#define EVENT_DELETE_PREFIX     "ED"

#define SUCCESS_CODE "0"
#define FAILURE_CODE "1"
#define MESSAGE_SEPARATOR ";"

enum result{SUCCESS, FAILURE};

static void valueCheck(int value, std::string messageFailure, std::string messageSuccess){
    if(value == -1){
        std::cout << messageFailure << std::endl;
        exit(1);
    }
    if(!messageSuccess.empty()) std::cout << messageSuccess << std::endl;
}
