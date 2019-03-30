#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Arduino.h>
#include "sensirion_uart.h"
#include "sps30.h"

const char* ssid = "<your wifi ssid>";
const char* password = "<your wifi password>";

ESP8266WebServer server(80);

void setup() {
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  bool mdns_inited = false;
  if(MDNS.begin("esp8266")) {
    mdns_inited = true;
  }

	sensirion_uart_open();
  int probe_result = sps30_probe();
  int start_measurement_result = sps30_start_measurement();

  server.onNotFound([](){
    server.send(404, "text/plain", "404 -- Not Found");
  });

  server.on("/", [](){
    server.send(200, "text/plain", "hello world!");
  });

  server.on("/debug_status", [&](){
    char buf[50];
    sprintf(buf, "probe:%d, start_measurement:%d", probe_result, start_measurement_result);
    server.send(200, "text/plain", buf);
  });

  server.on("/read_measurement", [](){
    sps30_measurement measurement;
    
    s16 result = sps30_read_measurement(&measurement);
    while(result != 0){
      delay(50);
      result = sps30_read_measurement(&measurement);
    }
    
    
    bool err_state = SPS_IS_ERR_STATE(result);
    String mc_1p0 = String(measurement.mc_1p0);
    String mc_2p5 = String(measurement.mc_2p5);
    String mc_4p0 = String(measurement.mc_4p0);
    String mc_10p0 = String(measurement.mc_10p0);
    String nc_0p5 = String(measurement.nc_0p5);
    String nc_1p0 = String(measurement.nc_1p0);
    String nc_2p5 = String(measurement.nc_2p5);
    String nc_4p0 = String(measurement.nc_4p0);
    String nc_10p0 = String(measurement.nc_10p0);
    String typical_particle_size = String(measurement.typical_particle_size);
    
    
    char buffer[500];
    sprintf(buffer, 
    "{\"err_state\": %d, \"typical_particle_size\": %s, \"mc_1p0\": %s, \"mc_2p5\": %s, \"mc_4p0\": %s, \"mc_10p0\": %s, \"nc_0p5\": %s, \"nc_1p0\": %s, \"nc_2p5\": %s, \"nc_4p0\": %s, \"nc_10p0\": %s}", 
    err_state, typical_particle_size.c_str(),
    mc_1p0.c_str(), mc_2p5.c_str(), mc_4p0.c_str(), mc_10p0.c_str(),
    nc_0p5.c_str(), nc_1p0.c_str(), nc_2p5.c_str(), nc_4p0.c_str(), nc_10p0.c_str());
    server.send(200, "text/plain", buffer);

  });

  server.begin();
}

void loop() {
  server.handleClient();
}
