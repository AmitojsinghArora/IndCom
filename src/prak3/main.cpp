#include <ModbusMaster.h>
#include "ModbusServerTCPasync.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

ModbusMaster node;
ModbusServerTCPasync MBserver;

const char *ssid = "Legendwaitforit";
const char *password = "daryLegendary";

WiFiServer server(80);

// Global sensor values
uint16_t temperatureRaw = 0;
uint16_t humidityRaw = 0;

// FC04 handler: Modbus TCP input registers
ModbusMessage FC04(ModbusMessage request) {
  ModbusMessage response;
  uint16_t addr = 0, words = 0;
  request.get(2, addr);
  request.get(4, words);

  if ((addr + words) > 20) {
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
    return response;
  }

  response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
  for (uint16_t i = 0; i < words; i++) {
    switch (addr + i) {
      case 1: response.add(temperatureRaw); break;
      case 2: response.add(humidityRaw); break;
      default: response.add((uint16_t)0); break;
    }
  }

  return response;
}

void handleHTTPClient(WiFiClient client) {
  String request = client.readStringUntil('\r');
  client.read(); // Clear the \n

  float temp = temperatureRaw / 10.0;
  float hum  = humidityRaw / 10.0;

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/plain");
  client.println("Connection: close");
  client.println();
  client.printf("Temperature: %.1f °C\n", temp);
  client.printf("Humidity: %.1f %%\n", hum);
  delay(1);
  client.stop();

  // Let WiFi settle before going back to Modbus
  delay(200);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nConfiguring Access Point...");

  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while (1);
  }

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  server.begin();  // HTTP server

  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // RS485 Serial
  node.begin(1, Serial2);                  // Modbus Slave ID 1

  MBserver.registerWorker(1, READ_INPUT_REGISTER, &FC04);
  MBserver.start(502, 1, 20000);           // Modbus TCP
}

void loop() {
  // Serve HTTP requests
  WiFiClient client = server.available();
  if (client) {
    while (client.connected() && !client.available()) {
      delay(1);
    }
    if (client.available()) {
      handleHTTPClient(client);
    }

    return; // Skip RTU this loop to prevent conflict
  }

  // Poll Modbus RTU sensor
  uint8_t result = node.readInputRegisters(0x0000, 2);
  if (result == node.ku8MBSuccess) {
    temperatureRaw = node.getResponseBuffer(0);
    humidityRaw    = node.getResponseBuffer(1);

    float temperature = temperatureRaw / 10.0;
    float humidity    = humidityRaw / 10.0;

    Serial.printf("Temperature: %.1f °C\n", temperature);
    Serial.printf("Humidity: %.1f %%\n", humidity);
  } else {
    Serial.print("Modbus RTU error: ");
    Serial.println(result);
  }

  delay(1000);  // Poll every second
}
