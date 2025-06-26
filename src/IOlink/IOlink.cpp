#include <lwIOLink.hpp>

using namespace lwIOLink;

// IO-Link configuration
static constexpr unsigned PDInSize = 2;
static constexpr unsigned PDOutSize = 2;
static constexpr unsigned min_cycle_time = 50000; // 50 ms

uint8_t PDOut[PDOutSize] = {0, 0}; // From Master (not used)
uint8_t PDIn[PDInSize] = {0, 0};   // To Master

constexpr Device::HWConfig HWCfg = {
  .SerialPort = Serial2,
  .Baud = lwIOLink::COM2,
  .WakeupMode = FALLING,
  .Pin = {
    .TxEN = 18,
    .Wakeup = 5,
    .Tx = 17,
    .Rx = 16
  }
};

Device iol_device(PDInSize, PDOutSize, min_cycle_time);

// Joystick analog pins
#define JOYSTICK_X 36
#define JOYSTICK_Y 39

void Device::OnNewCycle() {
  // Read analog joystick values (scale 0–4095 -> 0–255)
  uint8_t joyX = analogRead(JOYSTICK_X) >> 4;
  uint8_t joyY = analogRead(JOYSTICK_Y) >> 4;

  PDIn[0] = joyX;
  PDIn[1] = joyY;

  Serial.print("Joystick X: "); Serial.print(joyX);
  Serial.print(" Y: "); Serial.println(joyY);

  iol_device.SetPDIn(PDIn, sizeof(PDIn));
  iol_device.SetPDInStatus(Valid);
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12); // Ensure full 12-bit ADC resolution
  iol_device.begin(HWCfg);
}

void loop() {
  iol_device.run();  // Handle IO-Link communication
}
