
#include <thread>

class RunningThread{
    public:
    bool running;
    std::thread::id threadId;

    RunningThread(std::thread::id threadId){
        this->threadId = threadId;
        this->running = true;
    }

    bool operator==(const bool& running){
        return this->running == running;
    }
};