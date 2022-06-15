// Socket-Kommunikation sind nach dem Linux system calls
// https://linux.die.net/man/2/
// Lok-Adresse 3

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

#include "lib.h"

#define Z21_LOK_ID = 3

std::vector<uint8_t> bitreverse(std::vector<uint8_t> t) {
  auto a = t;
  std::reverse(a.begin(), a.end());
  return a;
};

uint16_t bytemerge(uint8_t lsb, uint8_t msb) {
  return ((uint16_t)msb << 8) | lsb;
};

uint32_t bytemerge(uint8_t lsb, uint8_t lmb, uint8_t mmb, uint8_t msb) {
  return static_cast<uint32_t>((msb << 24) + (mmb << 16) + (lmb << 8) + lsb);
};

std::vector<uint8_t> bytesplit(uint16_t t) {
  std::vector<uint8_t> r;
  r.push_back(static_cast<uint8_t>((t & 0xFF00) >> 8));
  r.push_back(static_cast<uint8_t>((t & 0x00FF) >> 0));
  return r;
};

std::vector<uint8_t> bytesplit(u_int32_t t) {
  std::vector<uint8_t> r;
  r.push_back(static_cast<uint8_t>((t & 0xFF000000) >> 24));
  r.push_back(static_cast<uint8_t>((t & 0x00FF0000) >> 16));
  r.push_back(static_cast<uint8_t>((t & 0x0000FF00) >> 8));
  r.push_back(static_cast<uint8_t>((t & 0x000000FF) >> 0));
  return r;
};

std::chrono::time_point<std::chrono::system_clock> castNow() {
  auto now = std::chrono::system_clock::now();
  return std::chrono::time_point_cast<std::chrono::milliseconds>(now);
};

/*udp_c::udp_c(int port, const std::string &adresse)
    : in_port(port), in_adresse(adresse) {
  char dport[16];
  snprintf(dport, sizeof(dport), "%d", in_port);
  dport[sizeof(dport) / sizeof(dport[0]) - 1] = '\0';
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;
  struct sockaddr_in srcaddr;
  memset(&srcaddr, 0, sizeof(srcaddr));
  srcaddr.sin_family = AF_INET;
  srcaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  srcaddr.sin_port = htons(static_cast<uint16_t>(21105));
  int r(getaddrinfo(adresse.c_str(), dport, &hints, &in_adressenInfo));
  if (r != 0 || in_adressenInfo == NULL) {
    throw std::runtime_error(
        ("invalid address or port: \"" + adresse + ":" + dport + "\"").c_str());
  }
  in_verbindung = socket(in_adressenInfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC,
                         IPPROTO_UDP);
  if (bind(in_verbindung, (struct sockaddr *)&srcaddr, sizeof(srcaddr)) < 0) {
    exit(1);
  }
  if (in_verbindung == -1) {
    // freeaddrinfo(in_adressenInfo);
    throw std::runtime_error(
        ("could not create socket for: \"" + adresse + ":" + dport + "\"")
            .c_str());
  }
};

int udp_c::senden(const char *nachricht, size_t groesse) {
  return sendto(in_verbindung, nachricht, groesse, 0, in_adressenInfo->ai_addr,
                in_adressenInfo->ai_addrlen);
};

udp_c::~udp_c() {
  // freeaddrinfo(in_adressenInfo);
  close(in_verbindung);
};

int udp_c::port() const { return in_port; };

int udp_c::verbindung() const { return in_verbindung; };

std::string udp_c::adresse() const { return in_adresse; };

int udp_c::recv(char *msg, size_t max_size) {
  return ::recv(in_verbindung, msg, max_size, 0);
};

int udp_c::timed_recv(char *msg, size_t max_size, int max_wait_ms) {
  fd_set s;
  FD_ZERO(&s);
  FD_SET(in_verbindung, &s);
  struct timeval timeout;
  timeout.tv_sec = max_wait_ms / 1000;
  timeout.tv_usec = (max_wait_ms % 1000) * 1000;
  int retval = select(in_verbindung + 1, &s, &s, &s, &timeout);
  if (retval == -1) {
    // select() set errno accordingly
    return -1;
  };
  if (retval > 0) {
    // our socket has data
    return ::recv(in_verbindung, msg, max_size, 0);
  };

  // our socket has no data
  errno = EAGAIN;
  return -1;
};

udp_s::udp_s(int port, const std::string &adresse)
    : in_port(port), in_adresse(adresse) {
  char decimal_port[16];
  snprintf(decimal_port, sizeof(decimal_port), "%d", in_port);
  decimal_port[sizeof(decimal_port) / sizeof(decimal_port[0]) - 1] = '\0';
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;
  int r(getaddrinfo(adresse.c_str(), decimal_port, &hints, &in_adressinfo));
  if (r != 0 || in_adressinfo == NULL) {
    throw std::runtime_error(("invalid address or port for UDP socket: \"" +
                              adresse + ":" + decimal_port + "\"")
                                 .c_str());
  }
  in_verbindung =
      socket(in_adressinfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
  if (in_verbindung == -1) {
    freeaddrinfo(in_adressinfo);
    throw std::runtime_error(("could not create UDP socket for: \"" + adresse +
                              ":" + decimal_port + "\"")
                                 .c_str());
  }
  r = bind(in_verbindung, in_adressinfo->ai_addr, in_adressinfo->ai_addrlen);
  if (r != 0) {
    freeaddrinfo(in_adressinfo);
    close(in_verbindung);
    throw std::runtime_error(("could not bind UDP socket with: \"" + adresse +
                              ":" + decimal_port + "\"")
                                 .c_str());
  }
};

udp_s::udp_s(int socket) {
  in_verbindung = socket;
};

udp_s::~udp_s() {
  freeaddrinfo(in_adressinfo);
  close(in_verbindung);
};

int udp_s::verbindung() const { return in_verbindung; };

int udp_s::port() const { return in_port; };

std::string udp_s::adresse() const { return in_adresse; };

int udp_s::recv(char *msg, size_t max_size) {
  return ::recv(in_verbindung, msg, max_size, 0);
};

int udp_s::timed_recv(char *msg, size_t max_size, int max_wait_ms) {
  fd_set s;
  FD_ZERO(&s);
  FD_SET(in_verbindung, &s);
  struct timeval timeout;
  timeout.tv_sec = max_wait_ms / 1000;
  timeout.tv_usec = (max_wait_ms % 1000) * 1000;
  int retval = select(in_verbindung + 1, &s, &s, &s, &timeout);
  if (retval == -1) {
    // select() set errno accordingly
    return -1;
  };
  if (retval > 0) {
    // our socket has data
    return ::recv(in_verbindung, msg, max_size, 0);
  };

  // our socket has no data
  errno = EAGAIN;
  return -1;
};*/

// z21 Datensatz

void calculateChecksum(z21Datensatz &datenSatz) {
  uint8_t p = 0;
  for (int i = 0; i < ((std::vector<uint8_t>)datenSatz.data).size(); i++) {
    p = p ^ datenSatz.data[i];
  }
  datenSatz.data.push_back(p);
}

std::vector<uint8_t> calculateSingleStateSize(singleState s) {
  uint16_t z = 2 + 2 + s.content.data.size();
  std::vector<uint8_t> t = bytesplit(z);
  /*for (int i = 0; i < t.size(); i++) {
    std::cout << std::to_string(t[i]) << std::endl;
  }*/
  std::reverse(t.begin(), t.end());
  return t;
}

std::vector<uint8_t> formPacket(singleState s) {
  if (s.content.checksum == true)
    calculateChecksum(s.content);
  std::cout << s.content.data.size() << std::endl;
  std::vector<uint8_t> h1 = calculateSingleStateSize(s);
  std::vector<uint8_t> h2;
  h2.push_back(s.content.dataHeader);
  h2.push_back(0);
  std::vector<uint8_t> all;
  all.insert(all.end(), h1.begin(), h1.end());
  all.insert(all.end(), h2.begin(), h2.end());
  all.insert(all.end(), s.content.data.begin(), s.content.data.end());
  return all;
  /*uint8_t *m = &all[0];
  for (int i = 0; i < sizeof(m); i++) {
    std::cout << std::to_string(m[i]) << std::endl;
  }
  char *mc = reinterpret_cast<char *>(m);
  return mc;*/
}
// 2.0 SYSTEM/STATUS/VERSION

singleState z21_lan_get_serial_number() {
  singleState s;
  s.p = 1;

  z21Datensatz d;
  d.isReq = true;
  d.dataHeader = 0x10;

  s.startDate = castNow();
  s.content = d;

  return s;
};

singleState z21_lan_logoff() {
  singleState s;
  s.p = 5;

  z21Datensatz d;
  d.isReq = true;
  d.dataHeader = 0x30;

  s.startDate = castNow();
  s.content = d;

  return s;
};

singleState z21_lan_x_get_version() {
  singleState s;
  s.p = 1;

  z21Datensatz d;
  d.isReq = true;
  d.dataHeader = 0x40;

  uint8_t t[3];
  // x-header
  t[0] = 0x21;
  // db0
  t[1] = 0x21;
  // xor-byte
  t[2] = 0x00;

  d.data.insert(d.data.end(), t, t + sizeof t);

  s.startDate = castNow();
  s.content = d;

  return s;
};

singleState z21_lan_x_get_status() {
  singleState s;
  s.p = 1;

  z21Datensatz d;
  d.isReq = true;
  d.dataHeader = 0x40;

  uint8_t t[3];
  // x-header
  t[0] = 0x21;
  // db0
  t[1] = 0x24;
  // xor-byte
  t[2] = 0x05;

  d.data.insert(d.data.end(), t, t + sizeof t);

  s.startDate = castNow();
  s.content = d;

  return s;
};

singleState z21_lan_x_set_track_power_off() {
  singleState s;
  s.p = 1;

  z21Datensatz d;
  d.isReq = true;
  d.dataHeader = 0x40;

  uint8_t t[3];
  // x-header
  t[0] = 0x21;
  // db0
  t[1] = 0x80;
  // xor-byte
  t[2] = 0xa1;

  d.data.insert(d.data.end(), t, t + sizeof t);

  s.startDate = castNow();
  s.content = d;

  return s;
};

singleState z21_lan_x_set_track_power_on() {
  singleState s;
  s.p = 1;

  z21Datensatz d;
  d.isReq = true;
  d.dataHeader = 0x40;

  uint8_t t[3];
  // x-header
  t[0] = 0x21;
  // db0
  t[1] = 0x81;
  // xor-byte
  t[2] = 0xa0;

  d.data.insert(d.data.end(), t, t + sizeof t);

  s.startDate = castNow();
  s.content = d;

  return s;
};

singleState z21_lan_x_set_stop() {
  singleState s;
  s.p = 1;

  z21Datensatz d;
  d.isReq = true;
  d.dataHeader = 0x40;

  uint8_t t[2];
  // x-header
  t[0] = 0x80;
  // xor-byte
  t[1] = 0x80;

  d.data.insert(d.data.end(), t, t + sizeof t);

  s.startDate = castNow();
  s.content = d;

  return s;
};

singleState z21_lan_x_get_firmware_version() {
  singleState s;
  s.p = 1;

  z21Datensatz d;
  d.isReq = true;
  d.dataHeader = 0x40;

  uint8_t t[3];
  // x-header
  t[0] = 0xF1;
  // db0
  t[1] = 0x0A;
  // xor-byte
  t[2] = 0xFB;

  d.data.insert(d.data.end(), t, t + sizeof t);

  s.startDate = castNow();
  s.content = d;

  return s;
};

singleState z21_lan_set_broadcastflags(std::vector<uint32_t> flags) {
  singleState s;
  s.p = 1;

  z21Datensatz d;
  d.isReq = true;
  d.dataHeader = 0x50;

  uint32_t c = 0;
  for (int i = 0; i < flags.size(); i++) {
    c = c | flags[i];
    // std::cout << "t" << std::hex << flags[i] << c << std::endl;
  }

  d.data = bytesplit(c);

  s.startDate = castNow();
  s.content = d;

  return s;
};

singleState z21_get_broadcastflags() {
  singleState s;
  s.p = 1;

  z21Datensatz d;
  d.isReq = true;
  d.dataHeader = 0x51;

  s.startDate = castNow();
  s.content = d;

  return s;
};

singleState z21_lan_systemstate_getdata() {
  singleState s;
  s.p = 1;

  z21Datensatz d;
  d.isReq = true;
  d.dataHeader = 0x85;

  s.startDate = castNow();
  s.content = d;

  return s;
};

singleState z21_lan_get_hwinfo() {
  singleState s;
  s.p = 1;

  z21Datensatz d;
  d.isReq = true;
  d.dataHeader = 0x1A;

  s.startDate = castNow();
  s.content = d;

  return s;
};

singleState z21_lan_get_code() {
  singleState s;
  s.p = 1;

  z21Datensatz d;
  d.isReq = true;
  d.dataHeader = 0x85;

  s.startDate = castNow();
  s.content = d;

  return s;
};

// 3.0 EINSTELLUNGEN

// 4.0 FAHREN
singleState z21_lan_x_get_loco_info(uint16_t lok) {
  singleState s;
  s.p = 1;

  z21Datensatz d;
  d.isReq = true;
  d.checksum = true;
  d.dataHeader = 0x40;

  uint8_t t[4];
  t[0] = 0xE3;
  t[1] = 0xF0;
  std::vector<uint8_t> bytes = bytesplit(lok);
  if (lok >= 128)
    t[2] = bytes[0] | 0xC0;
  t[3] = bytes[1];

  d.data.insert(d.data.end(), t, t + sizeof t);

  s.startDate = castNow();
  s.content = d;

  return s;
};

singleState z21_lan_x_set_loco_drive(uint16_t lok, uint8_t levelCount,
                                     uint8_t forward, uint8_t speedLevel) {
  singleState s;
  s.p = 1;

  z21Datensatz d;
  d.isReq = true;
  d.checksum = true;
  d.dataHeader = 0x40;

  uint8_t t[5];
  // x-header
  t[0] = 0xE4;
  // db0
  switch (levelCount) {
  case 0:
    t[1] = 0x10;
    break;
  case 1:
    t[1] = 0x12;
    break;
  case 2:
    t[1] = 0x13;
  }
  // db1
  std::vector<uint8_t> bytes = bytesplit(lok);
  /*if (lok >= 128)
    bytes[2] = bytes[0] | 0xC0;*/
  // db2
  t[3] = bytes[1];
  // db3
  uint8_t c = 0;
  if (levelCount == 0) {
    c = forward * 128 + speedLevel;
  } else if (levelCount == 2) {
    c = forward * 128 + 16 * (speedLevel % 2) +
        (int)round((float)speedLevel / 2);
  } else if (levelCount == 3) {
    c = forward * 128 + speedLevel;
  };
  t[4] = c;
  // xor autogenerated

  d.data.insert(d.data.end(), t, t + sizeof t);

  s.startDate = castNow();
  s.content = d;

  return s;
};

singleState z21_lan_x_set_loco_drive_hold(uint16_t lok, uint8_t levelCount,
                                          bool forward) {
  return z21_lan_x_set_loco_drive(lok, levelCount, forward, 0);
};

singleState z21_lan_x_set_loco_drive_emergency_hold(uint16_t lok,
                                                    uint8_t levelCount,
                                                    bool forward) {
  return z21_lan_x_set_loco_drive(lok, levelCount, forward, 1);
};

singleState z21_lan_x_set_loco_function(uint16_t lok, uint8_t action,
                                        uint8_t fIndex) {
  singleState s;
  s.p = 1;

  z21Datensatz d;
  d.isReq = true;
  d.checksum = true;
  d.dataHeader = 0x40;

  uint8_t t[5];
  // x-header
  t[0] = 0xE4;
  // db0
  t[1] = 0xF8;
  // db1
  std::vector<uint8_t> bytes = bytesplit(lok);
  if (lok >= 128)
    bytes[2] = bytes[0] | 0xC0;
  // db2
  t[3] = bytes[1];
  // db3
  uint8_t c = 0;
  if (action > 2) {
    c += 128;
  }
  if ((action + 1) % 2 == 0) {
    c += 64;
  }
  c += fIndex;
  t[4] = c;
  // xor autogenerated

  d.data.insert(d.data.end(), t, t + sizeof t);

  s.startDate = castNow();
  s.content = d;

  return s;
}

singleState z21_lan_x_set_turnout(uint16_t switchId, uint8_t turnway,
                                  uint8_t queue) {
  if (queue != 0)
    throw std::runtime_error("not implemented");
  singleState s;
  s.p = 1;

  z21Datensatz d;
  d.isReq = true;
  d.checksum = true;
  d.dataHeader = 0x40;

  uint8_t t[4];
  // x-header
  t[0] = 0x53;
  // db0
  std::vector<uint8_t> bytes = bytesplit(switchId);
  t[1] = bytes[0];
  // db1
  t[2] = bytes[1];
  // db3
  uint8_t c = 128 + 8;
  if (turnway == 1) {
    c += 1;
  }
  if (queue == 1) {
    c += 32;
  };
  t[3] = c;
  // xor autogenerated

  d.data.insert(d.data.end(), t, t + sizeof t);

  s.startDate = castNow();
  s.content = d;

  return s;
}

singleState z21_lan_can_detector_broadcast() {
  singleState s;
  s.p = 1;

  z21Datensatz d;
  d.isReq = true;
  d.dataHeader = 0xC4;

  uint8_t t[3];
  t[0] = 0x00;
  t[1] = 0x00;
  t[2] = 0xD0;

  d.data.insert(d.data.end(), t, t + sizeof t);

  s.startDate = castNow();
  s.content = d;

  return s;
}

singleSerialNumberResponse
z21_response_lan_get_serial_number(std::vector<uint8_t> data) {
  // TODO: implement prestate her
  singleSerialNumberResponse serialNumberResponse;
  serialNumberResponse.stateName = "LAN_GET_SERIAL_NUMBER";
  serialNumberResponse.serialNumber =
      bytemerge(data[3], data[2], data[1], data[0]);

  return serialNumberResponse;
};

singleVersionResponse z21_response_lan_x_version(std::vector<uint8_t> data) {
  // TODO: implement prestate her
  singleVersionResponse versionResponse;
  versionResponse.stateName = "LAN_X_GET_VERSION";
  versionResponse.xbus_ver = data[2];
  versionResponse.cmdst_id = data[3];

  return versionResponse;
};

singleStatusChangedResponse
z21_response_status_changed(std::vector<uint8_t> data) {
  // TODO: implement prestate here
  singleStatusChangedResponse statusResponse;
  statusResponse.stateName = "LAN_X_STATUS_CHANGED";
  statusResponse.status = data[2];

  return statusResponse;
};

singleSystemStateDataChangeResponse
z21_response_systemstate_changed(std::vector<uint8_t> data) {
  // TODO: implement prestate here
  singleSystemStateDataChangeResponse stateResponse;
  stateResponse.stateName = "LAN_SYSTEMSTATE_DATACHANGED";
  stateResponse.status = data[2];

  return stateResponse;
};

singleLocoInfo z21_lan_x_loco_info(std::vector<uint8_t> data) {
  singleLocoInfo infoResponse;
  infoResponse.stateName = "LAN_X_LOCO_INFO";
  infoResponse.LokID = bytemerge(data[2], data[1]);
  infoResponse.isUsed = data[3] >= 8;
  infoResponse.LevelCount = data[4] & 0x07;
  infoResponse.isForward = data[5] >= 128;
  infoResponse.rawSpeed = data[5] & 0x7f;
  // TODO: index überprüfen / oder 5
  std::vector<uint8_t> rf(data.begin() + 6, data.end());
  infoResponse.rawFunctions = rf;

  return infoResponse;
};

singleTurnoutInfo z21_lan_x_turnout_info(std::vector<uint8_t> data) {
  singleTurnoutInfo turnoutResponse;
  turnoutResponse.stateName = "LAN_X_TURNOUT_INFO";
  turnoutResponse.SwitchID = bytemerge(data[2], data[1]);
  turnoutResponse.SwitchState = data[3] & 0x3; // useless bc first 6 bits = 0

  return turnoutResponse;
};

singleCANDetector z21_can_detector(std::vector<uint8_t> data) {
  // TODO: implement prestate here
  singleCANDetector stateResponse;
  stateResponse.stateName = "LAN_CAN_DETECTOR";
  stateResponse.NId = bytemerge(data[1], data[0]);
  stateResponse.Addr = bytemerge(data[3], data[2]);
  stateResponse.Port = data[4];
  stateResponse.Typ = data[5];
  stateResponse.v1 = bytemerge(data[7], data[6]);
  stateResponse.v2 = bytemerge(data[9], data[8]);

  std::cout << std::to_string(stateResponse.Addr) << " "
            << std::to_string(stateResponse.Port) << " p "
            << std::to_string(stateResponse.v1) << " v "
            << std::to_string(stateResponse.v2) << std::endl;

  return stateResponse;
};

singleLocoData z21_loco_data(std::vector<uint8_t> data) {
  singleLocoData locoData;
  locoData.stateName = "LOCO_DATA";
  locoData.Id = data[5];

  return locoData;
};

int operation2(int x, int y, std::function<int(int, int)> function) {
  return function(x, y);
}

singleBasicResponseState z21_response(std::vector<uint8_t> data) {
  singleBasicResponseState s;

  s.dataLen = data[0];
  if (s.dataLen > data.size()) {
    throw std::runtime_error("Invalid DATA LENGTH in response packet: Invalid");
  }

  s.header = data[2];

  std::vector<uint8_t> idata(data.begin() + 4, data.begin() + s.dataLen);
  s.raw = idata;

  if (s.header == 0xa1 && idata.size() == 6) {
    s.stateName = "LOCO_DATA";
  } else if (s.header == 0x10) {
    s.stateName = "LAN_GET_SERIAL_NUMBER";
  } else if (s.header == 0xC4) {
    std::cout << "CAN" << std::endl;
    s.stateName = "LAN_CAN_DETECTOR";
  }

  return s;
};

ssize_t z21_sendto(int __fd, std::vector<uint8_t> msg, size_t __n,
                   __CONST_SOCKADDR_ARG __addr, socklen_t __addr_len) {
  char buf[msg.size()];
  for (int i = 0; i < msg.size(); i++) {
    buf[i] = (char)msg[i];
  }

  // std::cout << std::to_string(sizeof(buf)) << " " <<
  // std::to_string(msg.size()) << std::endl;

  return sendto(__fd, buf, sizeof(buf), MSG_CONFIRM, __addr, __addr_len);
};