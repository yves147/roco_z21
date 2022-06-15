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

uint16_t bytemerge(uint8_t n1, uint8_t n2) { return ((uint16_t)n2 << 8) | n1; };

int main(int argc, char *argv[]) {
  uint16_t result =
      bytemerge(static_cast<uint8_t>(54), static_cast<uint8_t>(1));
  std::cout << std::to_string(result) << std::endl;
}