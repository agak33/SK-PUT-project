#include "models/user.h"
#include "serverFunctions.h"
#include "server.h"

#include <iostream>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

Server* server;

void signal_handler(int signal){
    delete server;
    exit(0);
}

int main(int argc, char** argv){
    if(argc < 3){
        std::cout << "You have entered not enough arguments" << std::endl;
        std::cout << "Proper execute command: path_to_file host_port host_name <queue_size> <max_connected_clients>" << std::endl;
        exit(0);
    }    
    std::signal(SIGINT, signal_handler);
    switch(argc){
        case 3:
            server = new Server(atoi(argv[1]), argv[2]);
            break;
        case 4:
            server = new Server(atoi(argv[1]), argv[2], atoi(argv[3]));
            break;
        case 5:
            server = new Server(atoi(argv[1]), argv[2], atoi(argv[3]), atoi(argv[4]));
            break;
        default:
            std::cout << "oops, something went wrong" << std::endl;
            delete server;
            exit(0);
    }
    server->loop();
    delete server;
}