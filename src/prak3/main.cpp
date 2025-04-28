#include <ModbusMaster.h>

// Create ModbusMaster object
ModbusMaster node;

void setup() {
  // Start the Serial Monitor
  Serial.begin(115200);

  // Start Serial2 (RX=GPIO16, TX=GPIO17) with standard Modbus settings
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  // Initialize Modbus communication with slave ID 1
  node.begin(1, Serial2); // Replace 1 with your sensor's Modbus address
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

  delay(2000); // Wait 2 seconds before next read
}
