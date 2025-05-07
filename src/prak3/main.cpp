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

// Global variables to hold the latest sensor values
uint16_t temperature = 0;
uint16_t humidity = 0;
float temp = temperature / 10.0;
float hum = humidity / 10.0;

ModbusMessage FC04(ModbusMessage request) {
  ModbusMessage response;      // The Modbus message we are going to give back
  uint16_t addr = 0;           // Start address
  uint16_t words = 0;          // # of words requested
  request.get(2, addr);        // read address from request
  request.get(4, words);       // read # of words from request

  // Address overflow?
  if ((addr + words) > 20) {
    // Yes - send respective error response
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  // Set up response
  response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
  
  for (uint16_t i = 0; i < words; i++) {
    switch (addr + i) {
      case 1:
        response.add(temp);  // Register 0x0001
        break;
      case 2:
        response.add(hum);     // Register 0x0002
        break;
      default:
        response.add((uint16_t)0);     // Return 0 for undefined addresses
        break;
    }
  }
  // Send response back
  return response;
}

// Handle simple HTTP requests
void handleHTTPClient(WiFiClient client) {
  String request = client.readStringUntil('\r');
  client.read();  // Consume the '\n'

  // Send basic plain text response with sensor values
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/plain");
  client.println("Connection: close");
  client.println();

  client.print("Temperature: ");
  client.print(temp, 1);
  client.println(" °C");

  client.print("Humidity: ");
  client.print(hum, 1);
  client.println(" %");

  delay(1);
  client.stop();
}

void setup() {
  // Start the Serial Monitor
  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");

  // You can remove the password parameter if you want the AP to be open.
  // a valid password must have more than 7 characters
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while (1);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Serial.println("Server started");

  // Start Serial2 (RX=GPIO16, TX=GPIO17) with standard Modbus settings
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  // Initialize Modbus communication with slave ID 1
  node.begin(1, Serial2); 

  server.begin(); // Start the server on port 502
  Serial.println("Modbus server started on port 502");

  MBserver.registerWorker(1, READ_INPUT_REGISTER, &FC04);     // FC=04 for serverID=1
  MBserver.start(502, 1, 20000);
}


void loop() {
  uint8_t result;

  // Read 2 input registers starting from address 0x0000
  result = node.readInputRegisters(0x0000, 2);

  if (result == node.ku8MBSuccess) {
    // Print temperature and humidity assuming 0.1 scaling
    float temperature = node.getResponseBuffer(0) / 10.0;
    float humidity = node.getResponseBuffer(1) / 10.0;

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  } else {
    // Print error code if Modbus request fails
    Serial.print("Modbus error: ");
    Serial.println(result);
  }

  // Handle HTTP requests
  WiFiClient client = server.available();
  if (client) {
    while (client.connected() && !client.available()) {
      delay(1);
    }
    if (client.available()) {
      handleHTTPClient(client);
    }
  }

  delay(1000); // Wait 1 second before next read
}
