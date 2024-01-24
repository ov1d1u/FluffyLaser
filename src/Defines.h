#ifndef ARDUINO_DEFINES_H
#define ARDUINO_DEFINES_H

#define MAX_MOVES 50

#define INIT_X 100
#define INIT_Y 70

struct Configuration {
  String ssid;
  String password;
  String mqtt_server;
  String mqtt_user;
  String mqtt_password;
  int mqtt_port;
};

#endif