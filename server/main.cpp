
#include "server.h"


Server* server;

void signal_handler(int signal){
    delete server;
    exit(0);
}

void sigpipe_handler(int signal){
    std::cout << "SIGPIPE OCCURED" << std::endl;
}

int main(int argc, char** argv){
    if(argc < 3){
        std::cout << "Not enough arguments" << std::endl;
        std::cout << "To run program: path_to_program_file host_name host_port" << std::endl;
        exit(0);
    }    
    std::signal(SIGINT, signal_handler);
    std::signal(SIGPIPE, sigpipe_handler);
    server = new Server(atoi(argv[2]), argv[1]);
    server->loop();
    delete server;
}