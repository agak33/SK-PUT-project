#include <iostream>
#include <mutex>
#pragma once

#define DATA_SEPARATOR ";"
#define DATA_END       '\n'

#define LOGIN_PREFIX            "L"
#define REGISTER_PREFIX         "R"
#define LOGOUT_PREFIX           "LO"
#define CLOSING_APP_PREFIX      "LC"

#define CALENDAR_INSERT_PREFIX  "CI"
#define CALENDAR_MODIFY_PREFIX  "CM"
#define CALENDAR_DELETE_PREFIX  "CD"

#define CALENDAR_INSERT_USER_PREFIX "CIU"
#define CALENDAR_DELETE_USER_PREFIX "CDU"

#define CALENDAR_GET_NAMES_PREFIX   "CGN"
#define CALENDAR_GET_INFO_PREFIX    "CGI"
#define CALENDAR_GET_EVENTS_PREFIX  "CGE"
#define CALENDAR_GET_USERS_PREFIX   "CGU"

#define EVENT_INSERT_PREFIX     "EI"
#define EVENT_MODIFY_PREFIX     "EM"
#define EVENT_DELETE_PREFIX     "ED"

#define SUCCESS_CODE "0"
#define FAILURE_CODE "1"

#define MAX_DESCRIPTORS_NUM 5
//#define MAX_CONNECTED_CLIENTS 50
#define MAX_CLIENT_NUMBER 100
#define BUFFER_SIZE 100

enum result{SUCCESS, FAILURE};

static void valueCheck(int value, std::string messageFailure, std::string messageSuccess){
    if(value == -1){
        std::cout << messageFailure << std::endl;
        exit(1);
    }
    if(!messageSuccess.empty()) std::cout << messageSuccess << std::endl;
}
