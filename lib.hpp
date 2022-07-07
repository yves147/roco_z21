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

#include "json.hpp"
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

struct singleVersionResponse : singleBasicResponseState {
  uint8_t xbus_ver;
  uint8_t cmdst_id;
};

struct singleStatusChangedResponse : singleBasicResponseState {
  uint8_t status;
  uint8_t csEmergencyStop;
  uint8_t csTrackVoltageOff;
  uint8_t csShortCircuit;
  uint8_t csProgrammingModeActive;
};

struct singleSystemStateDataChangeResponse : singleBasicResponseState {
  int16_t MainCurrent;
  int16_t ProgCurrent;
  int16_t FilteredMainCurrent;
  int16_t Temperature;
  nlohmann::basic_json<> CentralState;
  nlohmann::basic_json<> CentralStateEx;
  nlohmann::basic_json<> Capabilities;
};

struct singleLocoInfo : singleBasicResponseState {
  uint16_t LokID;
  bool isUsed;
  uint8_t LevelCount;
  bool isForward;
  uint8_t rawSpeed;
  uint8_t doppelTraktion;
  uint8_t smartSearch;
  uint8_t licht;
  uint8_t f4;
  uint8_t f3;
  uint8_t f2;
  uint8_t f1;
  nlohmann::basic_json<> adf;
};

struct singleTurnoutInfo : singleBasicResponseState {
  uint16_t SwitchID;
  uint8_t SwitchState;
};

struct singleCANDetector : singleBasicResponseState {
  uint16_t NId;
  uint16_t Addr;
  uint8_t Port;
  uint8_t Typ;
  uint16_t v1;
  uint16_t v2;
  bool busyState;
  std::string busyStateName = "NONE";
  nlohmann::basic_json<> railcomAdi;
};

struct singleLocoData : singleBasicResponseState {
  uint8_t Id;
};

struct singleFirmware : singleBasicResponseState {
  uint16_t Version;
};

struct singleCode : singleBasicResponseState {
  uint8_t Code;
};

struct singleHardware : singleBasicResponseState {
  std::string HwType;
  uint32_t HwTypeRaw;
  uint32_t HwVersion;
};

singleLocoData z21_loco_data(std::vector<uint8_t> data);
singleSerialNumberResponse
z21_response_lan_get_serial_number(std::vector<uint8_t> data);
singleVersionResponse z21_response_lan_x_version(std::vector<uint8_t> data);
singleStatusChangedResponse
z21_response_status_changed(std::vector<uint8_t> data);
singleSystemStateDataChangeResponse
z21_response_systemstate_changed(std::vector<uint8_t> data);
singleLocoInfo z21_response_lan_x_loco_info(std::vector<uint8_t> data);
singleTurnoutInfo z21_response_lan_x_turnout_info(std::vector<uint8_t> data);
singleCANDetector z21_response_can_detector(std::vector<uint8_t> data);
singleFirmware z21_response_firmware(std::vector<uint8_t> data);
singleCode z21_response_code(std::vector<uint8_t> data);
singleHardware z21_response_hardware(std::vector<uint8_t> data);

nlohmann::json _jsonconvert(singleSerialNumberResponse s);
nlohmann::json _jsonconvert(singleVersionResponse s);
nlohmann::json _jsonconvert(singleStatusChangedResponse s);
nlohmann::json _jsonconvert(singleSystemStateDataChangeResponse s);
nlohmann::json _jsonconvert(singleLocoInfo s);
nlohmann::json _jsonconvert(singleTurnoutInfo s);
nlohmann::json _jsonconvert(singleCANDetector s);
nlohmann::json _jsonconvert(singleFirmware s);
nlohmann::json _jsonconvert(singleCode s);
nlohmann::json _jsonconvert(singleHardware s);

singleBasicResponseState z21_response(std::vector<uint8_t> data);

ssize_t z21_sendto(int __fd, std::vector<uint8_t> msg, size_t __n,
                   __CONST_SOCKADDR_ARG __addr, socklen_t __addr_len);
