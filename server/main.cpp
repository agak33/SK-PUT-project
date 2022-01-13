
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
        std::cout << "Too few arguments to run server" << std::endl;
        std::cout << "To run: path_to_program_file host_port host_name" << std::endl;
        exit(0);
    }    
    std::signal(SIGINT, signal_handler);
    std::signal(SIGPIPE, sigpipe_handler);
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