#include <algorithm>
#include <bits/stdc++.h>
#include <bitset>
#include <boost/asio/buffer.hpp>
#include <boost/exception/exception.hpp>
#include <cmath>
#include <functional>
#include <iostream>
#include <iterator>
#include <queue>
#include <stdio.h>
#include <string.h>
#include <vector>

#include <chrono>
#include <unistd.h>

#include "lib.hpp"

#include <algorithm>
#include <iostream>
#include <iterator>

#include <cstdlib>
#include <iostream>

#include <mutex>

#include <boost/asio.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#define Z21_ADDRESS "192.168.0.111"
#define Z21_PORT 21105

#include "json.hpp"

namespace beast = boost::beast;         // boost/beast.hpp
namespace http = beast::http;           // boost/beast/http.hpp
namespace websocket = beast::websocket; // boost/beast/websocket.hpp
namespace net = boost::asio;            // boost/asio.hpp
using boost::asio::ip::udp;             // boost/asio/ip/udp.hpp

// GLOBAL VARIABLES =>
std::vector<std::function<void(std::vector<uint8_t> data)>> wsListeners;
std::vector<std::function<void(std::vector<uint8_t> data)>> udpListeners;
std::vector<std::function<void(nlohmann::json data)>> broadcastListeners;
// GLOBAL VARIABLES <=

std::mutex WSControlFlowMutex;

void fail(beast::error_code ec, char const *what) {
  std::cerr << what << ": " << ec.message() << "\n";
};

// thread 3..n
class session : public std::enable_shared_from_this<session> {
  websocket::stream<beast::tcp_stream> ws_;
  beast::flat_buffer buffer_;
  std::queue<std::string> q;
  bool qrun = false;
  int t_b = 0;
  int t_u = 0;

public:
  explicit session(boost::asio::ip::tcp::socket &&socket)
      : ws_(std::move(socket)) {}

  ~session() {
    udpListeners[t_u] = [](nlohmann::json data) {};
    broadcastListeners[t_b] = [](nlohmann::json data) {};
  }

  void run() {
    net::dispatch(
        ws_.get_executor(),
        beast::bind_front_handler(&session::on_run, shared_from_this()));
  }

  void on_run() {
    ws_.set_option(
        websocket::stream_base::timeout::suggested(beast::role_type::server));

    ws_.set_option(
        websocket::stream_base::decorator([](websocket::response_type &res) {
          res.set(http::field::server,
                  std::string(BOOST_BEAST_VERSION_STRING) + " z21-websocket");
        }));

    ws_.async_accept(
        beast::bind_front_handler(&session::on_accept, shared_from_this()));
  }

  void on_accept(beast::error_code ec) {
    if (ec)
      return fail(ec, "accept");

    t_u = udpListeners.size();
    auto d_u = [this](std::vector<uint8_t> data) {
      // d = udpData;
      singleBasicResponseState response = z21_response(data);
      if (response.stateName == "LAN_GET_SERIAL_NUMBER") {
        singleSerialNumberResponse s =
            z21_response_lan_get_serial_number(response.raw);
        nlohmann::json j = {{"command", "R_SERIAL_NUMBER"},
                            {"captured", _jsonconvert(s)}};
        write(j.dump());
      }
    };
    udpListeners.push_back(d_u);
    t_b = broadcastListeners.size();
    auto d_b = [this](nlohmann::json data) {
      nlohmann::json j = {{"command", "WS_BROADCAST"}, {"captured", data}};
      write(j.dump());
    };
    broadcastListeners.push_back(d_b);
    do_read();
  }

  void do_read() {
    ws_.async_read(buffer_, beast::bind_front_handler(&session::on_read,
                                                      shared_from_this()));
  }

  void on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec == websocket::error::closed)
      return;

    if (ec)
      fail(ec, "read");

    ws_.text(ws_.got_text());

    std::string t = boost::beast::buffers_to_string(buffer_.data());
    nlohmann::json message;
    try {
      message = nlohmann::json::parse(t);
    } catch (nlohmann::json::parse_error e) {
      return;
    }
    if (message["command"] == "COMMON_DISCONNECT") {
      return;
    };
    singleState s;
    if (message["command"] == "DRIVE") {
      s = z21_lan_x_set_loco_drive(message["lok"], message["levelCount"],
                                   message["forward"], message["speedLevel"]);
    } else if (message["command"] == "LOCO_INFO") {
      s = z21_lan_x_get_loco_info(message["lok"]);
    } else if (message["command"] == "HARDWARE") {
      s = z21_lan_get_hwinfo();
    } else if (message["command"] == "LOGOFF") {
      s = z21_lan_logoff();
    } else if (message["command"] == "SERIAL") {
      s = z21_lan_get_serial_number();
    } else if (message["command"] == "SET_BROADCAST") {
      std::vector<uint32_t> flags;
      if (message["AUTOMATIC"] == 1)
        flags.push_back(static_cast<uint32_t>(AUTOMATIC));
      if (message["RBUS"] == 1)
        flags.push_back(static_cast<uint32_t>(RBUS));
      if (message["RAILCOM"] == 1)
        flags.push_back(static_cast<uint32_t>(RAILCOM));
      s = z21_lan_set_broadcastflags(flags);
    } else if (message["command"] == "FUNCTION") {
      // state: 0 = aus, 1 = ein, 2 = umschalten, 3 = nicht erlaubt (siehe 4.3.1
      // Dokumentation)
      s = z21_lan_x_set_loco_function(message["lok"], message["state"],
                                      message["function"]);
    } else if (message["command"] == "GET_BROADCAST") {
      s = z21_get_broadcastflags();
    } else if (message["command"] == "FIRMWARE") {
      s = z21_lan_x_get_firmware_version();
    } else if (message["command"] == "SYSTEMSTATE") {
      s = z21_lan_systemstate_getdata();
    } else if (message["command"] == "TURNOUT") {
      s = z21_lan_x_set_turnout(message["switchId"], message["turnway"],
                                message["queue"]);
    } else if (message["command"] == "STATUS") {
      s = z21_lan_x_get_status();
    } else if (message["command"] == "DC_TRACK_OFF") {
      s = z21_lan_x_set_track_power_off();
    } else if (message["command"] == "DC_TRACK_ON") {
      s = z21_lan_x_set_track_power_on();
    } else if (message["command"] == "EMERGENCY_STOP_ALL") {
      // dc will still be on
      s = z21_lan_x_set_stop();
    } else if (message["command"] == "CODE") {
      s = z21_lan_get_code();
    } else {
      nlohmann::json wse = {{"command", "WS_UNKNOWN"}, {"original", message}};

      write(wse.dump());
      do_read();
      return;
    };
    auto packet = formPacket(s);

    for (u_int32_t i = 0; i < wsListeners.size(); i++) {
      wsListeners[i](packet);
    };

    for (u_int32_t i = 0; i < broadcastListeners.size(); i++) {
      if (i == t_b)
        continue;
      broadcastListeners[i](message);
    };

    nlohmann::json wse = {{"command", "WS_ECHO"}, {"original", message}};

    write(wse.dump());

    do_read();
  }

  void write(std::string m) {
    q.push(m);
    if (qrun == false)
      write_interval();
  }

  void write_interval() {
    if (q.size() == 0)
      return;
    qrun = true;
    std::string msg = q.front();
    ws_.async_write(
        boost::asio::buffer(msg),
        beast::bind_front_handler(&session::on_write, shared_from_this()));
    q.pop();
  }

  void on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec)
      return fail(ec, "write");

    buffer_.consume(buffer_.size());
    qrun = false;

    write_interval();
  }
};

// thread 2
class listener : public std::enable_shared_from_this<listener> {
  net::io_context &ioc_;
  boost::asio::ip::tcp::acceptor acceptor_;
  int test;

public:
  listener(net::io_context &ioc, boost::asio::ip::tcp::endpoint endpoint)
      : ioc_(ioc), acceptor_(ioc) {
    beast::error_code ec;

    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
      fail(ec, "open");
      return;
    }

    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
      fail(ec, "set_option");
      return;
    }

    acceptor_.bind(endpoint, ec);
    if (ec) {
      fail(ec, "bind");
      return;
    }

    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
      fail(ec, "listen");
      return;
    }
  }

  void run() { do_accept(); }

private:
  void do_accept() {
    acceptor_.async_accept(
        net::make_strand(ioc_),
        beast::bind_front_handler(&listener::on_accept, shared_from_this()));
  }

  void on_accept(beast::error_code ec, boost::asio::ip::tcp::socket socket) {
    if (ec) {
      fail(ec, "accept");
    } else {
      std::make_shared<session>(std::move(socket))->run();
    }

    do_accept();
  }
};

// thread 1
class server {
public:
  boost::asio::ip::udp::endpoint a_endpoint = boost::asio::ip::udp::endpoint(
      boost::asio::ip::address::from_string(Z21_ADDRESS), Z21_PORT);
  std::queue<std::vector<uint8_t>> q;
  bool qrun = false;

  server(boost::asio::io_context &io_context)
      : socket_(io_context, udp::endpoint(udp::v4(), 21105)) {
    auto l = [this](std::vector<uint8_t> data) { ws_message(data); };
    wsListeners.push_back(l);

    initial_broadcast();
  }

  void initial_broadcast() {
    std::vector<uint32_t> flags;
    flags.push_back(static_cast<uint32_t>(AUTOMATIC));
    flags.push_back(static_cast<uint32_t>(RBUS));
    flags.push_back(static_cast<uint32_t>(RAILCOM));
    singleState s1 = z21_lan_set_broadcastflags(flags);

    auto buf = formPacket(s1);

    char *charPnt = reinterpret_cast<char *>(&buf[0]);

    std::cout << "here" << std::endl;

    socket_.async_send_to(
        boost::asio::buffer(charPnt, buf.size()), a_endpoint,
        [this](boost::system::error_code, std::size_t) { do_receive(); });
  }

  void ws_message(std::vector<uint8_t> data) {
    q.push(data);
    if (qrun == false)
      send_interval();
  }

  void send_interval() {
    if (q.size() == 0)
      return;
    qrun = true;
    auto data = q.front();
    std::fill_n(data_, 1024, 0);
    std::copy(data.begin(), data.end(), data_);
    do_send(data.size());
    q.pop();
  }

  void do_receive() {
    socket_.async_receive_from(
        boost::asio::buffer(data_, max_length), sender_endpoint_,
        [this](boost::system::error_code ec, std::size_t bytes_recvd) {
          if (!ec && bytes_recvd > 0) {
            std::vector<u_int8_t> v = {};
            for (u_int32_t j = 0; j < bytes_recvd; j++) {
              v.push_back(data_[j]);
            }
            for (u_int32_t i = 0; i < udpListeners.size(); i++) {
              udpListeners[i](v);
            };
            do_receive();
          } else {
            do_receive();
          }
        });
  }

  void do_send(std::size_t length) {
    socket_.async_send_to(
        boost::asio::buffer(data_, length), a_endpoint,
        [this](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/) {
          qrun = false;
          send_interval();
        });
  }

private:
  udp::socket socket_;
  udp::endpoint sender_endpoint_;
  enum { max_length = 1024 };
  char data_[max_length];
};

int main(int argc, char *argv[]) {
  try {
    if (argc != 1) {
      return 1;
    }

    std::thread udpThread([] {
      boost::asio::io_context io_context;

      server s(io_context);
      io_context.run();
    });

    std::thread wsMainThread([] {
      std::shared_ptr<listener> listener_;
      std::vector<std::thread> v;
      const boost::asio::ip::address address = net::ip::make_address("0.0.0.0");
      const unsigned short port = static_cast<unsigned short>(8080);
      const int threads = 4;

      net::io_context ioc{threads};

      auto d = std::make_shared<listener>(
          ioc, boost::asio::ip::tcp::endpoint{address, port});
      d->run();

      v.reserve(threads - 1);
      for (auto i = threads - 1; i > 0; --i)
        v.emplace_back([&ioc] { ioc.run(); });
      ioc.run();
    });

    udpThread.detach();
    wsMainThread.join();

    // control mechanism thread / dead thread

  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
};
