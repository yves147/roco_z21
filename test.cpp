#include <algorithm>
#include <arpa/inet.h>
#include <bits/stdc++.h>
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

uint16_t bytemerge(uint8_t lsb, uint8_t msb) {
  return ((uint16_t)msb << 8) | lsb;
};

int main() {
  u_int16_t t = bytemerge(1, 255);
  std::cout << std::to_string(t) << std::endl;
  int16_t a = static_cast<int16_t>(t);
  std::cout << std::to_string(a) << std::endl;
}