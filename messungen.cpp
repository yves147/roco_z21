#include <iostream>
#include <numeric>
#include <stdexcept>
#include <stdio.h>
#include <thread>

#include <vector>

#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

#include "lib.hpp"

#define TEST_OUTPUT = "data/messungen-14.txt"
#define Z21_ADDRESS "192.168.0.111"
//#define Z21_LOCAL_ADDRESS "192.168.0.101"
//#define Z21_ADDRESS "127.0.0.1"
//#define Z21_ADDRESS "127.0.0.1"
#define Z21_PORT 21105
#define BUFFER_SIZE 1024

void sendThread(int __fd, std::vector<uint8_t> msg, size_t __n,
                __CONST_SOCKADDR_ARG __addr, socklen_t __addr_len) {
  z21_sendto(__fd, msg, __n, __addr, __addr_len);
}

int main(int argc, char *argv[]) {

  int sockfd;
  struct sockaddr_in servaddr, srcaddr;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    throw std::runtime_error("create failed");
    exit(1);
  }

  memset(&servaddr, 0, sizeof(servaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(Z21_ADDRESS);
  servaddr.sin_port = htons(21105);

  memset(&srcaddr, 0, sizeof(srcaddr));

  srcaddr.sin_family = AF_INET;
  srcaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  srcaddr.sin_port = htons(Z21_PORT);

  if (bind(sockfd, (struct sockaddr *)&srcaddr, sizeof(srcaddr)) < 0) {
    perror("bind");
    exit(1);
  }

  /*std::vector<uint32_t> flags;
  flags.push_back(static_cast<uint32_t>(AUTOMATIC));
  flags.push_back(static_cast<uint32_t>(RBUS));
  flags.push_back(static_cast<uint32_t>(RAILCOM));
  singleState s1 = z21_lan_set_broadcastflags(flags);

  auto driveMsg1 = formPacket(s1);
  z21_sendto(sockfd, driveMsg1, sizeof(driveMsg1), (struct sockaddr *)&servaddr,
             sizeof(servaddr));*/
  
  singleState s2 = z21_lan_x_set_loco_drive(3, 0, 1, 0);
  //singleState s2 = z21_lan_x_set_turnout(4, 1, 0);
  auto driveMsg2 = formPacket(s2);
  z21_sendto(sockfd, driveMsg2, sizeof(driveMsg2), (struct sockaddr *)&servaddr,
             sizeof(servaddr));

  // singleState s = z21_lan_x_set_loco_function(3, 0, 2);

  /*std::chrono::time_point<std::chrono::system_clock> bremsStart;
  bool laeuft = false;
  int t = 0;

  while (true) {

    char buffer[BUFFER_SIZE];
    socklen_t qlen;

    int q = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE, MSG_WAITALL,
                     (struct sockaddr *)&servaddr, &qlen);
    buffer[q] = '\0';
    std::vector<uint8_t> m;
    for (int i = 0; i < BUFFER_SIZE; i++) {
      m.push_back((u_int8_t)(buffer[i]));
    };
    singleBasicResponseState s = z21_response(m);
    if (s.stateName == "LOCO_DATA") {
      singleLocoData sLD = z21_loco_data(s.raw);
      std::cout << std::to_string(sLD.Id) << std::endl;
      auto d = castNow();
      if (sLD.Id == 46) {
        bremsStart = d;
        laeuft = true;
        singleState s2 = z21_lan_x_set_loco_drive(3, 0, 1, 14);
        auto driveMsg2 = formPacket(s2);
        z21_sendto(sockfd, driveMsg2, sizeof(driveMsg2),
                   (struct sockaddr *)&servaddr, sizeof(servaddr));
      } else if (laeuft == true) {
        t++;
        std::cout << "count " << std::to_string(t) << std::endl;
      }
    }

    close(sockfd);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (bind(sockfd, (struct sockaddr *)&srcaddr, sizeof(srcaddr)) < 0) {
      perror("bind");
      exit(1);
    }
  }

  close(sockfd);*/

  return 0;
};