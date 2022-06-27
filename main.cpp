#include <iostream>
#include <stdexcept>
#include <stdio.h>

#include <vector>

#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

#include "lib.hpp"

#define Z21_ADDRESS "192.168.0.111"
//#define Z21_LOCAL_ADDRESS "192.168.0.101"
//#define Z21_ADDRESS "127.0.0.1"
//#define Z21_ADDRESS "127.0.0.1"
#define Z21_PORT 21105
#define BUFFER_SIZE 1024

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

  // udp_c c(Z21_PORT, Z21_ADDRESS);
  //  udp_s o(Z21_PORT, Z21_LOCAL_ADDRESS);
  /*std::vector<uint32_t> flags;
  flags.push_back(static_cast<uint32_t>(AUTOMATIC));
  flags.push_back(static_cast<uint32_t>(CAN_DETECTOR));
  singleState s = z21_lan_set_broadcastflags(flags);*/
  // singleState s = z21_lan_x_set_loco_drive_emergency_hold(3, 0, 1);
  singleState s = z21_lan_x_set_loco_drive(3, 0, 1, 15);
  // singleState s = z21_lan_x_set_loco_function(3, 0, 2);
  // singleState s = z21_lan_x_set_turnout(3, 1, 0);

  // singleState s = z21_lan_x_get_status();
  std::vector<uint8_t> msg = formPacket(s);
  char buf[msg.size()];
  for (int i = 0; i < msg.size(); i++) {
    std::cout << std::to_string(msg[i]) << std::endl;
    buf[i] = (char)msg[i];
  }

  //std::cout << std::to_string(sizeof(buf)) << " " << std::to_string(msg.size()) << std::endl;

  sendto(sockfd, buf, sizeof(buf), MSG_CONFIRM, (struct sockaddr *)&servaddr,
         sizeof(servaddr));
  std::cout << "sent" << std::endl;

  while (1) {

    char buffer[BUFFER_SIZE];
    socklen_t qlen;

    int q = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE, MSG_WAITALL,
                     (struct sockaddr *)&servaddr, &qlen);
    buffer[q] = '\0';
    std::vector<uint8_t> m;
    for (int i = 0; i < BUFFER_SIZE; i++) {
      m.push_back((u_int8_t)(buffer[i]));
    }
    z21_response(m);
  }

  close(sockfd);
  return 0;
};