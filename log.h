#include <algorithm>
#include <arpa/inet.h>
#include <bitset>
#include <cmath>
#include <iostream>
#include <iterator>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <vector>

#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>

#include <chrono>
#include <unistd.h>

#include "states.h"

struct LogEvent {
    uint8_t EventLevel;
    std::string EventText;
    std::chrono::time_point<std::chrono::system_clock> EventTPoint;
};

struct InMessageLogEvent : LogEvent {
    singleResponseState istate;
};

struct OutMessageLogEvent : LogEvent {
    singleState ostate;
};

//TODO: (mini) Logger schreiben
class Logger {
    public:
    
};