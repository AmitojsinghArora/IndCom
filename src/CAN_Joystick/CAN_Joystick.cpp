// CR1301 joystick - multi-mode control with encoder tracking, direction animation, and position mode
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

using Color = std::array<byte, 3>;
const Color BLACK = {0, 0, 0};
const Color YELLOW = {255, 255, 0};
const std::array<Color, 12> COLOR_CYCLE = {
  Color{249, 170, 12}, Color{255, 128, 0}, Color{249, 91, 12}, Color{255, 0, 0},
  Color{249, 12, 162}, Color{204, 0, 204}, Color{217, 12, 249}, Color{0, 0, 255},
  Color{12, 249, 194}, Color{0, 255, 0}, Color{139, 249, 12}, Color{255, 255, 0}
};

enum Mode { DEFAULT_MODE, COLOR_CHANGE, AUTOMATIC, ROTATION_DIRECTION, POSITION_MODE };
Mode currentMode = DEFAULT_MODE;

int encoderPosition = 0;
bool rotatingClockwise = true;
bool lastDirectionClockwise = true;
int colorIndices[4] = {0, 0, 0, 0};
Color currentColor = COLOR_CYCLE[0];
unsigned long lastUpdate = 0;
bool encoderMoved = false;
unsigned long lastRotationTime = 0;
int autoSpeedCounter = 0;
unsigned long autoLastColorChange = 0;
int positionModeStartVal = 0;

void transmitSegments(const Color& c, int highlightSegment = -1) {
  Color segColors[4] = {BLACK, BLACK, BLACK, BLACK};
  if (highlightSegment >= 0 && highlightSegment < 4) {
    segColors[highlightSegment] = c;
  }
  byte p1[8] = { segColors[0][0], segColors[0][1], segColors[0][2],
                 segColors[1][0], segColors[1][1], segColors[1][2], 255, 255 };
  byte p2[8] = { segColors[2][0], segColors[2][1], segColors[2][2],
                 segColors[3][0], segColors[3][1], segColors[3][2], 255, 255 };
  J1939.Transmit(SEGMENT_PGNS[0], PRIORITY, SRC_ADDR, DEST_ADDR, p1, PAYLOAD_LEN);
  J1939.Transmit(SEGMENT_PGNS[1], PRIORITY, SRC_ADDR, DEST_ADDR, p2, PAYLOAD_LEN);
}

void transmitPositionMode(byte value) {
  int segment = value / 6;
  if (segment > 3) segment = 3;
  transmitSegments(YELLOW, segment);
}

void resetSystem() {
  currentMode = DEFAULT_MODE;
  encoderPosition = 0;
  autoSpeedCounter = 0;
  lastRotationTime = millis();
  transmitSegments(BLACK);
}

void updateAutomaticMode() {
  if (millis() - autoLastColorChange > (100 * (autoSpeedCounter + 1))) {
    autoLastColorChange = millis();
    currentColor = COLOR_CYCLE[(encoderPosition++ % COLOR_CYCLE.size())];
    transmitSegments(currentColor);
  }
}

void setup() {
  Serial.begin(115200);
  if (CAN0.begin(MCP_ANY, CAN_250KBPS, MCP_8MHZ) == CAN_OK)
    CAN0.setMode(MCP_NORMAL);
  else {
    Serial.println("CAN Init Failed");
    while (true);
  }
  transmitSegments(BLACK); // Start with lights off
}

void loop() {
  long pgn;
  byte prio, src, dst, data[8];
  int len;

  if (J1939.Receive(&pgn, &prio, &src, &dst, data, &len) == 0 && pgn == 0xFF64 && len >= 4) {
    byte encVal = data[3] & 0x1F;
    bool directionBit = data[3] & 0x20;
    rotatingClockwise = !directionBit;
    encoderMoved = encVal > 0;

    if (encoderMoved) {
      lastDirectionClockwise = rotatingClockwise;
      encoderPosition += rotatingClockwise ? encVal : -encVal;
      lastRotationTime = millis();
    }

    bool resetPressed = data[1] & 0x40;
    bool statusKey3Pressed = data[1] & 0x10;
    bool statusKey5Pressed = data[2] & 0x01;
    bool rotaryPressed = data[3] & 0x40;

    if (resetPressed) resetSystem();
    else if (rotaryPressed && currentMode == DEFAULT_MODE) currentMode = COLOR_CHANGE;
    else if (rotaryPressed && currentMode == COLOR_CHANGE) currentMode = DEFAULT_MODE;
    else if (statusKey5Pressed) currentMode = ROTATION_DIRECTION;
    else if (statusKey3Pressed) {
      currentMode = POSITION_MODE;
      positionModeStartVal = encoderPosition;
    } else if (data[1] & 0x1) currentMode = AUTOMATIC;

    if (millis() - lastUpdate > 100) {
      lastUpdate = millis();

      switch (currentMode) {
        case COLOR_CHANGE: {
          int segment = encoderPosition / 6;
          if (segment > 3) segment = 3;
          int& idx = colorIndices[segment];
          idx = (idx + (rotatingClockwise ? 1 : -1) + 12) % 12;
          currentColor = COLOR_CYCLE[idx];
          transmitSegments(currentColor, segment);
          break;
        }
        case AUTOMATIC:
          updateAutomaticMode();
          break;
        case ROTATION_DIRECTION:
          if (millis() - lastRotationTime <= 2000) {
            int highlight = lastDirectionClockwise ? (millis() / 150) % 4 : (3 - ((millis() / 150) % 4));
            transmitSegments(YELLOW, highlight);
          } else transmitSegments(BLACK);
          break;
        case POSITION_MODE:
          transmitPositionMode(abs(encoderPosition - positionModeStartVal) % 25);
          break;
        default :
          transmitSegments(BLACK);
          break;
      }

      encoderMoved = false;
    }
  }
}
