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

#include "lib.hpp"

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

// z21 Datensatz

void calculateChecksum(z21Datensatz &datenSatz) {
  uint8_t p = 0;
  for (uint32_t i = 0; i < ((std::vector<uint8_t>)datenSatz.data).size(); i++) {
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

  z21Datensatz d;
  d.isReq = true;
  d.dataHeader = 0x10;

  s.startDate = castNow();
  s.content = d;

  return s;
};

singleState z21_lan_logoff() {
  singleState s;

  z21Datensatz d;
  d.isReq = true;
  d.dataHeader = 0x30;

  s.startDate = castNow();
  s.content = d;

  return s;
};

singleState z21_lan_x_get_version() {
  singleState s;

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

  z21Datensatz d;
  d.isReq = true;
  d.dataHeader = 0x50;

  uint32_t c = 0;
  for (uint32_t i = 0; i < flags.size(); i++) {
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

  z21Datensatz d;
  d.isReq = true;
  d.dataHeader = 0x51;

  s.startDate = castNow();
  s.content = d;

  return s;
};

singleState z21_lan_systemstate_getdata() {
  singleState s;

  z21Datensatz d;
  d.isReq = true;
  d.dataHeader = 0x85;

  s.startDate = castNow();
  s.content = d;

  return s;
};

singleState z21_lan_get_hwinfo() {
  singleState s;

  z21Datensatz d;
  d.isReq = true;
  d.dataHeader = 0x1A;

  s.startDate = castNow();
  s.content = d;

  return s;
};

singleState z21_lan_get_code() {
  singleState s;

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
  statusResponse.csEmergencyStop = data[2] & 0x01;
  statusResponse.csTrackVoltageOff = data[2] & 0x02;
  statusResponse.csShortCircuit = data[2] & 0x04;
  statusResponse.csProgrammingModeActive = data[2] & 0x20;

  return statusResponse;
};

singleSystemStateDataChangeResponse
z21_response_systemstate_changed(std::vector<uint8_t> data) {
  // TODO: implement prestate here
  singleSystemStateDataChangeResponse stateResponse;
  stateResponse.stateName = "LAN_SYSTEMSTATE_DATACHANGED";
  //...
  stateResponse.MainCurrent = static_cast<int16_t>(bytemerge(data[1], data[0]));
  stateResponse.ProgCurrent = static_cast<int16_t>(bytemerge(data[3], data[2]));
  stateResponse.FilteredMainCurrent =
      static_cast<int16_t>(bytemerge(data[5], data[4]));
  stateResponse.Temperature = static_cast<int16_t>(bytemerge(data[7], data[6]));
  nlohmann::json cs = {};
  uint8_t cs_v = data[12];
  cs["csEmergencyStop"] = cs_v & 0x01;
  cs["csTrackVoltageOff"] = cs_v & 0x02;
  cs["csShortCircuit"] = cs_v & 0x04;
  cs["csProgrammingModeActive"] = cs_v & 0x20;
  stateResponse.CentralState = cs;
  nlohmann::json csex = {};
  uint8_t csex_v = data[13];
  csex["cseHighTemperature"] = csex_v & 0x01;
  csex["csePowerLost"] = csex_v & 0x02;
  csex["cseShortCircuitExternal"] = csex_v & 0x04;
  csex["cseShortCircuitInternal"] = csex_v & 0x08;
  csex["cseRCN213"] = csex_v & 0x20;
  stateResponse.CentralStateEx = csex;
  nlohmann::json cp = {};
  uint8_t cp_v = data[15];
  cp["capDCC"] = cp_v & 0x01;
  cp["capMM"] = cp_v & 0x02;
  cp["capRailCom"] = cp_v & 0x08;
  cp["capLocoCmds"] = cp_v & 0x10;
  cp["capAccessoryCmds"] = cp_v & 0x20;
  cp["capDetectorCmds"] = cp_v & 0x40;
  cp["capNeedsUnlockCode"] = cp_v & 0x80;
  stateResponse.Capabilities = cp;

  return stateResponse;
};

singleLocoInfo z21_response_lan_x_loco_info(std::vector<uint8_t> data) {
  singleLocoInfo infoResponse;
  infoResponse.stateName = "LAN_X_LOCO_INFO";
  infoResponse.LokID = bytemerge(data[2], data[1]);
  infoResponse.isUsed = data[3] >= 8;
  infoResponse.LevelCount = data[4] & 0x07;
  infoResponse.isForward = data[5] >= 128;
  infoResponse.rawSpeed = data[5] & 0x7f;
  // TODO: index überprüfen / oder 5
  std::vector<uint8_t> rf(data.begin() + 6, data.end());
  infoResponse.doppelTraktion = rf[0] & 0x40;
  infoResponse.smartSearch = rf[0] & 0x20;
  infoResponse.licht = rf[0] & 0x10;
  infoResponse.f4 = rf[0] & 8;
  infoResponse.f3 = rf[0] & 4;
  infoResponse.f2 = rf[0] & 2;
  infoResponse.f1 = rf[0] & 1;
  auto a = nlohmann::json::array();

  for (uint8_t t = 0; t < (rf.size() - 1) * 8; t++) {
    a += rf[1 + (int)round(floor((double)t / 8))] &
         (uint8_t)pow(2, 8 - (1 + t % 8));
  }

  infoResponse.adf = a;

  return infoResponse;
};

singleTurnoutInfo z21_response_lan_x_turnout_info(std::vector<uint8_t> data) {
  singleTurnoutInfo turnoutResponse;
  turnoutResponse.stateName = "LAN_X_TURNOUT_INFO";
  turnoutResponse.SwitchID = bytemerge(data[2], data[1]);
  turnoutResponse.SwitchState = data[3] & 0x3; // useless bc first 6 bits = 0

  return turnoutResponse;
};

singleCANDetector z21_response_can_detector(std::vector<uint8_t> data) {
  singleCANDetector stateResponse;
  stateResponse.stateName = "LAN_CAN_DETECTOR";
  stateResponse.NId = bytemerge(data[1], data[0]);
  stateResponse.Addr = bytemerge(data[3], data[2]);
  stateResponse.Port = data[4];
  stateResponse.Typ = data[5];
  stateResponse.v1 = bytemerge(data[7], data[6]);
  stateResponse.v2 = bytemerge(data[9], data[8]);

  if (data[5] == 0x01) {
    switch (data[5]) {
    case 0x0000:
      stateResponse.busyStateName = "FREE_NO_V";
      break;
    case 0x0100:
      stateResponse.busyStateName = "FREE_W_V";
      break;
    case 0x1000:
      stateResponse.busyStateName = "OCCUPIED_NO_V";
      break;
    case 0x1100:
      stateResponse.busyStateName = "OCCUPIED_W_V";
      break;
    case 0x1201:
      stateResponse.busyStateName = "OCCUPIED_U_1";
      break;
    case 0x1202:
      stateResponse.busyStateName = "OCCUPIED_U_2";
      break;
    case 0x1203:
      stateResponse.busyStateName = "OCCUPIED_U_3";
      break;
    default:
      stateResponse.busyState = "UNKNOWN";
    };
  } else if (data[5] >= 0x11 && data[5] <= 0x1F) {
    nlohmann::json a = {{"v1", {{"direction", 0}, {"lok", 0}}},
                        {"v2", {{"direction", 0}, {"lok", 0}}}};
    a["v1"]["direction"] = stateResponse.v1 & 0xC000;
    a["v2"]["direction"] = stateResponse.v2 & 0xC000;
    a["v1"]["lok"] = stateResponse.v1 & 0x3FFF;
    a["v2"]["lok"] = stateResponse.v2 & 0x3FFF;
  }
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

singleFirmware z21_response_firmware(std::vector<uint8_t> data) {
  singleFirmware firmwareData;
  firmwareData.stateName = "LAN_X_GET_FIRMWARE_VERSION";
  firmwareData.Version = bytemerge(data[3], data[2]);

  return firmwareData;
};

singleCode z21_response_code(std::vector<uint8_t> data) {
  singleCode codeData;
  codeData.stateName = "LAN_X_GET_CODE";
  codeData.Code = data[0];

  return codeData;
};

singleHardware z21_response_hardware(std::vector<uint8_t> data) {
  singleHardware hardwareData;
  hardwareData.HwVersion = bytemerge(data[7], data[6], data[5], data[4]);
  hardwareData.HwTypeRaw = bytemerge(data[3], data[2], data[1], data[0]);

  switch (hardwareData.HwTypeRaw) {
  case 0x00000200:
    hardwareData.HwType = "D_HWT_Z21_OLD";
    break;
  case 0x00000201:
    hardwareData.HwType = "D_HWT_Z21_NEW";
    break;
  case 0x00000202:
    hardwareData.HwType = "D_HWT_SMARTRAIL";
    break;
  case 0x00000203:
    hardwareData.HwType = "D_HWT_z21_SMALL";
    break;
  case 0x00000204:
    hardwareData.HwType = "D_HWT_z21_START";
    break;
  case 0x00000205:
    hardwareData.HwType = "D_HWT_SINGLE_BOOSTER";
    break;
  case 0x00000206:
    hardwareData.HwType = "D_HWT_DUAL_BOOSTER";
    break;
  case 0x00000211:
    hardwareData.HwType = "D_HWT_Z21_XL";
    break;
  case 0x00000212:
    hardwareData.HwType = "D_HWT_XL_BOOSTER";
    break;
  case 0x00000301:
    hardwareData.HwType = "D_HWT_Z21_SWITCH_DECODER";
    break;
  case 0x00000302:
    hardwareData.HwType = "D_HWT_Z21_SIGNAL_DECODER";
    break;
  default:
    hardwareData.HwType = "UNKNOWN";
  };

  return hardwareData;
};

singleBasicResponseState z21_response(std::vector<uint8_t> data) {
  std::cout << std::to_string(data.size()) << std::endl;
  singleBasicResponseState s;

  s.dataLen = data[0];
  if (s.dataLen != data.size()) {
    std::cout << "Invalid DATA LENGTH in response packet: Invalid" << std::endl;
    throw std::runtime_error("INVALID_DATA_LENGTH");
  }

  s.header = data[2];

  std::vector<uint8_t> idata(data.begin() + 4, data.begin() + s.dataLen);
  s.raw = idata;

  if (s.header == 0xa2) {
    std::bitset<8> w(idata[5]);
    if (w[0] != 0 && w[1] != 1) {
      std::cout << w << std::endl;
    }
  }
  if (s.header == 0xa1) {
    s.stateName = "LOCO_DATA";
  } else if (s.header == 0x10) {
    s.stateName = "LAN_GET_SERIAL_NUMBER";
    // z21_response_lan_get_serial_number
  } else if (s.header == 0x40 && idata[0] == 0xEF) {
    s.stateName = "LAN_X_LOCO_INFO";
    // z21_response_lan_x_loco_info
  } else if (s.header == 0xC4) {
    std::cout << "CAN" << std::endl;
    s.stateName = "LAN_CAN_DETECTOR";
    // z21_response_can_detector
  } else if (s.header == 0x40 && idata[0] == 0x62 && idata[1] == 0x22) {
    s.stateName = "LAN_X_STATUS_CHANGED";
    // z21_response_status_changed
  } else if (s.header == 0x84) {
    s.stateName = "LAN_SYSTEMSTATE_DATACHANGED";
    // z21_response_systemstate_changed
  } else if (s.header == 0x40 && idata[0] == 0x53) {
    s.stateName = "LAN_X_TURNOUT_INFO";
    // z21_response_lan_x_loco_info
  } else if (s.header == 0x40 && idata[0] == 0x63 && idata[1] == 0x21) {
    s.stateName = "LAN_X_VERSION";
    // z21_response_lan_x_version
  } else if (s.header == 0x40 && idata[0] == 0x61 && idata[1] == 0x00) {
    s.stateName = "LAN_X_BC_TRACK_POWER_OFF";
    // no method required
  } else if (s.header == 0x40 && idata[0] == 0x61 && idata[1] == 0x01) {
    s.stateName = "LAN_X_BC_TRACK_POWER_ON";
    // no method required
  } else if (s.header == 0x40 && idata[0] == 0x61 && idata[1] == 0x08) {
    s.stateName = "LAN_X_BC_TRACK_SHORT_CIRCUIT";
    // no method required
  } else if (s.header == 0x40 && idata[0] == 0x81 && idata[1] == 0x00) {
    s.stateName = "LAN_X_BC_STOPPED";
    // no method required
  } else if (s.header == 0x40 && idata[0] == 0xF3 && idata[1] == 0x0A) {
    s.stateName = "LAN_X_GET_FIRMWARE_VERSION";
    // z21_response_firmware
  } else if (s.header == 0x18) {
    s.stateName = "LAN_X_GET_CODE";
    // z21_response_code
  } else if (s.header == 0x1A) {
    s.stateName = "LAN_GET_HWINFO";
    // z21_response_hardware
  }

  return s;
};

nlohmann::json _jsonconvert(singleSerialNumberResponse s) {
  nlohmann::json j = {};
  j["serialNumber"] = s.serialNumber;
  return j;
}
nlohmann::json _jsonconvert(singleVersionResponse s) {
  nlohmann::json j = {};
  j["xbus_ver"] = s.xbus_ver;
  j["cmdst_id"] = s.cmdst_id;
  return j;
}
nlohmann::json _jsonconvert(singleStatusChangedResponse s) {
  nlohmann::json j = {};
  j["status"] = s.status;
  j["csEmergencyStop"] = s.csEmergencyStop;
  j["csTrackVoltageOff"] = s.csTrackVoltageOff;
  j["csShortCircuit"] = s.csShortCircuit;
  j["csProgrammingModeActive"] = s.csProgrammingModeActive;
  return j;
}
nlohmann::json _jsonconvert(singleSystemStateDataChangeResponse s) {
  nlohmann::json j = {};
  j["MainCurrent"] = s.MainCurrent;
  j["ProgCurrent"] = s.ProgCurrent;
  j["FilteredMainCurrent"] = s.FilteredMainCurrent;
  j["Temperature"] = s.Temperature;
  j["CentralState"] = s.CentralState;
  j["CentralStateEx"] = s.CentralStateEx;
  j["Capabilities"] = s.Capabilities;
  return j;
}
nlohmann::json _jsonconvert(singleLocoInfo s) {
  nlohmann::json j = {};
  j["LokID"] = s.LokID;
  j["isUsed"] = s.isUsed;
  j["LevelCount"] = s.LevelCount;
  j["isForward"] = s.isForward;
  j["rawSpeed"] = s.rawSpeed;
  return j;
}
nlohmann::json _jsonconvert(singleTurnoutInfo s) {
  nlohmann::json j = {};
  j["SwitchID"] = s.SwitchID;
  j["SwitchState"] = s.SwitchState;
  return j;
}
nlohmann::json _jsonconvert(singleCANDetector s) {
  nlohmann::json j = {};
  j["NId"] = s.NId;
  j["Addr"] = s.Addr;
  j["Port"] = s.Port;
  j["Typ"] = s.Typ;
  j["v1"] = s.v1;
  j["v2"] = s.v2;
  return j;
}
nlohmann::json _jsonconvert(singleFirmware s) {
  nlohmann::json j = {};
  j["Version"] = s.Version;
  return j;
}
nlohmann::json _jsonconvert(singleCode s) {
  nlohmann::json j = {};
  j["Code"] = s.Code;
  return j;
}
nlohmann::json _jsonconvert(singleHardware s) {
  nlohmann::json j = {};
  j["HwType"] = s.HwType;
  j["HwVersion"] = s.HwVersion;
  j["HwTypeRaw"] = s.HwTypeRaw;
  return j;
}