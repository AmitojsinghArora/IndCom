// Cleaned-up CR1301 joystick code with animation mode
#include <Arduino.h>
#include "SimpleJ1939.hpp"
#include <mcp_can.h>
#include <array>

static constexpr uint8_t MCP_CS = 5;
MCP_CAN CAN0(MCP_CS);
SimpleJ1939 J1939(&CAN0);

constexpr long SEGMENT_PGNS[] = { 0xFF78, 0xFF79 };
constexpr long KEY_PGNS[] = { 0xFF75, 0xFF76, 0xFF77 };
constexpr uint8_t PRIORITY = 0x06, SRC_ADDR = 0x87, DEST_ADDR = 0xFF, PAYLOAD_LEN = 8;

// Colors
using Color = std::array<byte, 3>;
const Color BLACK = {0, 0, 0};
const Color YELLOW = {255, 255, 0};
const std::array<Color, 12> COLOR_CYCLE = {
  Color{249, 170, 12}, Color{255, 128, 0}, Color{249, 91, 12}, Color{255, 0, 0},
  Color{249, 12, 162}, Color{204, 0, 204}, Color{217, 12, 249}, Color{0, 0, 255},
  Color{12, 249, 194}, Color{0, 255, 0}, Color{139, 249, 12}, Color{255, 255, 0}
};

// State
enum Mode { DEFAULT, COLOR_CHANGE, AUTOMATIC, ROTATION_ANIMATION };
Mode currentMode = DEFAULT;

Color segColors[4] = { YELLOW, YELLOW, YELLOW, YELLOW };
int segmentIndex = 0;
int colorIndices[4] = {0, 0, 0, 0};
int autoColorIndex = 0;
int autoSpeedCounter = 0;
int rotationSegmentIndex = 0;
unsigned long lastTime = 0;
bool rotatingClockwise = true;
bool buttonPressed[3] = {false, false, false}; // [rotary, reset, automatic]
bool statusButton3Pressed = false;

void transmitSegmentColors() {
  byte p1[8] = { segColors[0][0], segColors[0][1], segColors[0][2],
                 segColors[1][0], segColors[1][1], segColors[1][2], 255, 255 };
  byte p2[8] = { segColors[2][0], segColors[2][1], segColors[2][2],
                 segColors[3][0], segColors[3][1], segColors[3][2], 255, 255 };
  J1939.Transmit(SEGMENT_PGNS[0], PRIORITY, SRC_ADDR, DEST_ADDR, p1, PAYLOAD_LEN);
  J1939.Transmit(SEGMENT_PGNS[1], PRIORITY, SRC_ADDR, DEST_ADDR, p2, PAYLOAD_LEN);
}

void setAllSegmentColor(const Color& c) {
  for (auto& col : segColors) col = c;
  transmitSegmentColors();
  byte payload[8] = {c[0], c[1], c[2], c[0], c[1], c[2], 255, 255};
  for (auto pgn : KEY_PGNS)
    J1939.Transmit(pgn, PRIORITY, SRC_ADDR, DEST_ADDR, payload, PAYLOAD_LEN);
}

void resetSystem() {
  for (int i = 0; i < 4; ++i) segColors[i] = YELLOW, colorIndices[i] = 0;
  segmentIndex = autoColorIndex = autoSpeedCounter = rotationSegmentIndex = 0;
  lastTime = millis();
  setAllSegmentColor(YELLOW);
}

void setup() {
  Serial.begin(115200);
  if (CAN0.begin(MCP_ANY, CAN_250KBPS, MCP_8MHZ) == CAN_OK)
    CAN0.setMode(MCP_NORMAL);
  else {
    Serial.println("CAN Init Failed.");
    while (true);
  }
  resetSystem();
}

void updateColorCycle(Color& c, int& index, int delta) {
  index = (index + delta + 12) % 12;
  c = COLOR_CYCLE[index];
}

void animateRotation() {
  Color off = BLACK;
  Color lit = YELLOW;
  Color frame[4] = {off, off, off, off};
  frame[rotationSegmentIndex] = lit;
  segColors[0] = frame[0];
  segColors[1] = frame[1];
  segColors[2] = frame[2];
  segColors[3] = frame[3];
  transmitSegmentColors();
}

void loop() {
  long pgn;
  byte prio, src, dst, data[8];
  int len;

  if (J1939.Receive(&pgn, &prio, &src, &dst, data, &len) == 0 && pgn == 0xFF64 && len > 1) {
    bool btn3 = data[1] & 0x08;
    bool btn3Rising = btn3 && !statusButton3Pressed;
    statusButton3Pressed = btn3;

    if (btn3Rising) {
      currentMode = (currentMode == ROTATION_ANIMATION) ? DEFAULT : ROTATION_ANIMATION;
      rotationSegmentIndex = 0;
      lastTime = millis();
      if (currentMode == DEFAULT) setAllSegmentColor(BLACK);
      return;
    }

    // Handle other button states
    bool btnReset = data[1] & 0x40;
    bool btnAuto = data[1] & 0x10;
    bool btnRot = data[3] & 0x40;
    bool btns[] = {btnRot, btnReset, btnAuto};

    int delta = (data[3] & 0x1F) * ((data[3] & 0x20) ? -1 : 1);

    for (int i = 0; i < 3; ++i) {
      bool rising = btns[i] && !buttonPressed[i];
      buttonPressed[i] = btns[i];
      if (rising) {
        if (i == 1) { // reset
          resetSystem();
          currentMode = DEFAULT;
          return;
        } else if (i == 2) { // auto mode
          currentMode = (currentMode == AUTOMATIC) ? DEFAULT : AUTOMATIC;
          return;
        } else if (i == 0 && currentMode == DEFAULT && segmentIndex > 0) {
          currentMode = COLOR_CHANGE;
          return;
        } else if (i == 0 && currentMode == COLOR_CHANGE) {
          currentMode = DEFAULT;
          return;
        }
      }
    }

    // Mode-specific logic
    switch (currentMode) {
      case DEFAULT:
        segmentIndex = std::clamp(segmentIndex + delta, 0, 24);
        for (int i = 0; i < 4; ++i) segColors[i] = (segmentIndex > 6 * i) ? segColors[i] : BLACK;
        transmitSegmentColors();
        break;

      case COLOR_CHANGE:
        if (segmentIndex > 0) {
          int idx = segmentIndex / 6;
          updateColorCycle(segColors[idx], colorIndices[idx], delta);
          transmitSegmentColors();
        }
        break;

      case AUTOMATIC:
        autoSpeedCounter = std::clamp(autoSpeedCounter + delta, 0, 24);
        if (millis() - lastTime > (100 * (autoSpeedCounter + 1))) {
          lastTime = millis();
          updateColorCycle(segColors[0], autoColorIndex, 1);
          for (int i = 1; i < 4; ++i) segColors[i] = segColors[0];
          transmitSegmentColors();
        }
        break;

      case ROTATION_ANIMATION:
        rotatingClockwise = (data[3] & 0x20) == 0;
        if (millis() - lastTime >= 150) {
          lastTime = millis();
          rotationSegmentIndex = (rotationSegmentIndex + (rotatingClockwise ? 1 : 3)) % 4;
          animateRotation();
        }
        break;
    }
  }
}