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
#include "dataset.h"

struct singleState {
  /*
                  0   Nicht begonnen
                  1   In Arbeit
                  2   Abgesendet
                  3   Antwort erhalten = Korrekt
                  4   Antwortzeit ausgelaufen = Fehler
                  5   Keine Antwort erforderlich
  */
  uint8_t p = 0;
  std::chrono::time_point<std::chrono::system_clock> startDate;
  std::chrono::time_point<std::chrono::system_clock> finishDate;
  z21Datensatz content;
};

struct singleBasicResponseState {
  std::string stateName;
  uint16_t dataLen;
  uint8_t header;
  std::vector<uint8_t> raw;
};