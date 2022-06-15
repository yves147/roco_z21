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

struct z21Datensatz {
  bool isReq = true;
  bool checksum = false;
  uint16_t dataLength;
  uint16_t dataHeader;
  std::vector<uint8_t> data;
};

struct z21Daten {
  std::string datatype;
  std::vector<uint8_t> raw_data;
};