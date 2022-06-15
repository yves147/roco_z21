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

#include "flags.h"
#include "z21_types.h"

#include "states.h"

std::vector<uint8_t> bitreverse(std::vector<uint8_t> t);
uint16_t bytemerge(uint8_t lsb, uint8_t msb);
uint32_t bytemerge(uint8_t lsb, uint8_t lmb, uint8_t mmb, uint8_t msb);
std::vector<uint8_t> bytesplit(uint16_t t);
std::vector<uint8_t> bytesplit(u_int32_t t);
std::chrono::time_point<std::chrono::system_clock> castNow();
void calculateChecksum(z21Datensatz &datenSatz);
std::vector<uint8_t> calculateSingleStateSize(singleState s);
std::vector<uint8_t> formPacket(singleState s);
singleState z21_lan_get_serial_number();
singleState z21_lan_logoff();
singleState z21_lan_x_get_version();
singleState z21_lan_x_get_status();
singleState z21_lan_x_set_track_power_off();
singleState z21_lan_x_set_track_power_on();
singleState z21_lan_x_set_stop();
singleState z21_lan_x_get_firmware_version();
singleState z21_lan_set_broadcastflags(std::vector<uint32_t> flags);
singleState z21_get_broadcastflags();
singleState z21_lan_systemstate_getdata();
singleState z21_lan_get_hwinfo();
singleState z21_lan_get_code();
singleState z21_lan_x_get_loco_info(uint16_t lok);
singleState z21_lan_x_set_loco_drive(uint16_t lok, uint8_t levelCount,
                                     uint8_t forward, uint8_t speedLevel);
singleState z21_lan_x_set_loco_drive_hold(uint16_t lok, uint8_t levelCount,
                                          bool forward);
singleState z21_lan_x_set_loco_drive_emergency_hold(uint16_t lok,
                                                    uint8_t levelCount,
                                                    bool forward);
singleState z21_lan_x_set_loco_function(uint16_t lok, uint8_t action,
                                        uint8_t fIndex);
singleState z21_lan_x_set_turnout(uint16_t switchId, uint8_t turnway,
                                  uint8_t queue);
singleState z21_lan_can_detector_broadcast();

struct singleSerialNumberResponse : singleBasicResponseState {
  uint32_t serialNumber;
};
singleSerialNumberResponse
z21_response_lan_get_serial_number(std::vector<uint8_t> data);

struct singleVersionResponse : singleBasicResponseState {
  uint8_t xbus_ver;
  uint8_t cmdst_id;
};
singleVersionResponse z21_response_lan_x_version(std::vector<uint8_t> data);

struct singleStatusChangedResponse : singleBasicResponseState {
  uint8_t status;
};
singleStatusChangedResponse
z21_response_status_changed(singleBasicResponseState b,
                            std::vector<uint8_t> data);

struct singleSystemStateDataChangeResponse : singleBasicResponseState {
  uint8_t status;
};
singleSystemStateDataChangeResponse
z21_response_systemstate_changed(std::vector<uint8_t> data);

struct singleLocoInfo : singleBasicResponseState {
  uint16_t LokID;
  bool isUsed;
  uint8_t LevelCount;
  bool isForward;
  uint8_t rawSpeed;
  std::vector<uint8_t> rawFunctions;
};

singleLocoInfo z21_lan_x_loco_info(std::vector<uint8_t> data);

struct singleTurnoutInfo : singleBasicResponseState {
  uint16_t SwitchID;
  uint8_t SwitchState;
};

singleTurnoutInfo z21_lan_x_turnout_info(std::vector<uint8_t> data);

struct singleCANDetector : singleBasicResponseState {
  uint16_t NId;
  uint16_t Addr;
  uint8_t Port;
  uint8_t Typ;
  uint16_t v1;
  uint16_t v2;
};
singleCANDetector z21_can_detector(std::vector<uint8_t> data);

struct singleLocoData : singleBasicResponseState {
  uint8_t Id;
};
singleLocoData z21_loco_data(std::vector<uint8_t> data);

singleBasicResponseState z21_response(std::vector<uint8_t> data);

ssize_t  z21_sendto(int __fd, std::vector<uint8_t> msg, size_t __n,
               __CONST_SOCKADDR_ARG __addr, socklen_t __addr_len);