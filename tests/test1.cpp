#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>

#include <chrono>
#include <unistd.h>

#include "udpc.h"
#include "z21_types.h"

std::vector<uint8_t> bitsplit(u_int16_t t) {
  std::vector<uint8_t> r;
  r.push_back(static_cast<uint8_t>((t & 0xFF00) >> 8));
  r.push_back(static_cast<uint8_t>((t & 0x00FF) >> 0));
  return r;
};

int main(int argc, char *argv[]) {
  uint16_t lok = 310;
  uint8_t t[5];
  t[0] = 0xE3;
  t[1] = 0xF0;
  std::vector<uint8_t> bytes = bitsplit(lok);
  if (lok >= 128)
    bytes[0] = bytes[0] | 0xC0;
  t[3] = bytes[1];
  t[4] = bytes[0] ^ bytes[1];
  for (int i = 0; i < sizeof t; i++) {
    std::cout << std::to_string(bytes[i]) << std::endl;
  }
}