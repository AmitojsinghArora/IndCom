// This conceptual code demonstrates how to read key press and rotary encoder inputs from an IFM CR1301
// joystick via CAN bus and control its multi-color LEDs.
// It uses specific J1939 PGNs and data structures derived from the provided PDF.
//
// Hardware Setup:
// - ESP32 Development Board
// - MCP2515 CAN Bus Module (connected to ESP32 via SPI)
// - IFM CR1301 Industrial Joystick (connected to MCP2515 CAN bus)
//
// Dependencies:
// - A suitable MCP2515 CAN library (e.g., 'mcp_can.h' by Cory J. Fowler)
//   You might need to install it if not already present.

#include <SPI.h>          // Required for SPI communication with MCP2515
#include <mcp_can.h>      // MCP_CAN library by Cory J. Fowler
#include "SimpleJ1939.hpp"
#include <algorithm>
#include <array>

// --- Configuration for MCP2515 CAN Module ---
// Define the SPI Chip Select (CS) pin for the MCP2515.
// This will vary depending on your ESP32 board and wiring.
// Common ESP32 SPI pins: MOSI (GPIO23), MISO (GPIO19), SCK (GPIO18).
// Ensure these match your wiring.
const int SPI_CS_PIN = 5; // Example CS pin for ESP32

// Create a CAN object instance using the MCP_CAN library
MCP_CAN CAN0(SPI_CS_PIN); 
SimpleJ1939 J1939(&CAN0); //Creating a J1939 protocol object 

uint8_t priority = 0x06;
uint8_t srcAdd = 0x87;
uint8_t destAdd = 0xFF;
uint8_t length = 8;



//Colour defintions:
using RGBColour = std::array<uint8_t, 3>;


// byte black[3] = {0,0,0};
RGBColour  black = {0,0,0};
// byte yellow[3] = {255,255,0};
RGBColour yellow = {255,255,0};
// byte yellow_orange[3] = {249,170,12};
RGBColour yellow_orange = {249,170,0};
// byte orange[3] = {255,128,0};
RGBColour orange = {255,128,0};
// byte red_orange[3] = {249,91,12};
RGBColour red_orange = {249,91,0};
// byte red[3] = {255,0,0};
RGBColour red = {255,0,0};
// byte red_purple[3] = {249,12,162};
RGBColour red_purple = {249,12,162};
// byte purple[3] = {204,0,204};
RGBColour purple = {204,0,204};
// byte blue_purple[3] = {217,12,249};
RGBColour blue_purple = {217,12,249};
// byte blue[3] = {0,0,255};
RGBColour blue = {0,0,255};
// byte blue_green[3] = {12,249,194};
RGBColour blue_green = {10,249,195};
// byte green[3] = {0,255,0};
RGBColour green = {0,255,0};
// byte yellow_green[3] = {139,249,12};
RGBColour yellow_green = {130,249,10};



// Defining colour palette for cleaner function calling
const std::array<RGBColour, 12> colorPalette = {
    yellow_orange,
    orange,
    red_orange,
    red,
    red_purple,
    purple,
    blue_purple,
    blue,
    blue_green,
    green,
    yellow_green,
    yellow
};
uint8_t payload_off[8] = {
    black[0], black[1], black[2],
    black[0], black[1], black[2],
    255, 255
    };

//PGN values for keys (as per operational instructions manual)
int keys[3] = {0xFF75, 0xFF76, 0xFF77};

//PGN values for segments (as per operational instructions manual)
int segs[3] = {0xFF78, 0xFF79};

//CAN declarations

//Counters
uint8_t counter_segSelection = 0;
uint8_t counter_colorSelectionseg1 = 0;
uint8_t counter_colorSelectionseg2 = 0;
uint8_t counter_colorSelectionseg3 = 0;
uint8_t counter_colorSelectionseg4 = 0;
int MillisPassed =0;
unsigned long lastRotationTime = 0;
bool lastDirectionClockwise = true;
unsigned long lastUpdate = 0;

//Enum of all possible states:
enum possibleModes{DEFAULT_POSITION_TRACKING, DETECT_DIRECTION, LIGHT_UP };

//Current state
short state = DEFAULT_POSITION_TRACKING;

//flags for button press
bool rotary_button_pressed = false;
bool reset_button_pressed = false;
bool lightUpbutton = false;

//Defining a union to make individual bit-access easier

union Byte
{
unsigned char byte;
  struct
  {
    bool bit0 : 1;
    bool bit1 : 1;
    bool bit2 : 1;
    bool bit3 : 1;
    bool bit4 : 1;
    bool bit5 : 1;
    bool bit6 : 1;
    bool bit7 : 1;
      
  };
};

void setup()
{
  //Set the serial interface baud rate
  Serial.begin(115200);
    if(CAN0.begin(MCP_ANY, CAN_250KBPS, MCP_8MHZ) == CAN_OK)
    {
        CAN0.setMode(MCP_NORMAL); // Set operation mode to normal so the MCP2515 sends acks to received data.
    }
    else
    {
        Serial.print("CAN Init Failed. Check your SPI connections or CAN.begin intialization .\n\r");
        while(1);
    }


}


void lightUpAllSegmentsAndKeys(const RGBColour& color) {
    uint8_t payload[8] = {
        color[0], color[1], color[2], 
        color[0], color[1], color[2], 
        255, 255
    };

    for (int i = 0; i < 2; i++) {
        J1939.Transmit(segs[i], priority, srcAdd, destAdd, payload, length);
    }

    for (int i = 0; i < 3; i++) {
        J1939.Transmit(keys[i], priority, srcAdd, destAdd, payload, length);
    }
    Serial.println("lightUpAllSegmentsAndKeys");
}

void lightUpAllSegments(const RGBColour& color) {
    uint8_t payload[8] = {
        color[0], color[1], color[2], 
        color[0], color[1], color[2], 
        255, 255
    };
    uint8_t payload_off[8] = {
        black[0], black[1], black[2],
        black[0], black[1], black[2],
        255, 255
    };
    
    for (int i = 0; i < 2; i++) {
        J1939.Transmit(segs[i], priority, srcAdd, destAdd, payload, length);
    }

    for (int i = 0; i < 3; i++) {
        J1939.Transmit(keys[i], priority, srcAdd, destAdd, payload_off, length);
    }
    Serial.println("lightUpAllSegments");
}

void lightOffAllSegmentsAndKeys() {
    uint8_t payload[8] = {
        black[0], black[1], black[2],
        black[0], black[1], black[2],
        255, 255
    };

    for (int i = 0; i < 2; i++) {
        J1939.Transmit(segs[i], priority, srcAdd, destAdd, payload, length);
    }

    for (int i = 0; i < 3; i++) {
        J1939.Transmit(keys[i], priority, srcAdd, destAdd, payload, length);
    }

    //Serial.println("lightOffAllSegmentsAndKeys");
}

//void lightUpNothing()
//void lightUpTill()

void animateAnticlockwiseDirection() {
    const uint8_t delayMs = 100;
    uint8_t payload_off[8] = {
    black[0], black[1], black[2],
    black[0], black[1], black[2],
    255, 255
    };
    uint8_t payload_1[8] = {
    black[0], black[1], black[2],
    green[0], green[1], green[2],
    255, 255
    };
    uint8_t payload_2[8] = {
    green[0], green[1], green[2],
    black[0], black[1], black[2],
    255, 255
    };    

    
    J1939.Transmit(segs[0], priority, srcAdd, destAdd, payload_off, 8);
    J1939.Transmit(segs[1], priority, srcAdd, destAdd, payload_off, 8);
    for (int i = 0; i < 4; ++i) {
    
        // Turn on segment 4 only
        J1939.Transmit(segs[0], priority, srcAdd, destAdd, payload_2, 8);
        J1939.Transmit(segs[1], priority, srcAdd, destAdd, payload_off, 8);
        delay(delayMs);
        // Turn on segment 3 only
        J1939.Transmit(segs[1], priority, srcAdd, destAdd, payload_1, 8);
        J1939.Transmit(segs[0], priority, srcAdd, destAdd, payload_off, 8);
        delay(delayMs);
        // Turn on segment 2 only
        J1939.Transmit(segs[1], priority, srcAdd, destAdd, payload_2, 8);
        J1939.Transmit(segs[0], priority, srcAdd, destAdd, payload_off, 8);
        delay(delayMs);
        // Turn on segment 1 only
        J1939.Transmit(segs[0], priority, srcAdd, destAdd, payload_1, 8);
        J1939.Transmit(segs[1], priority, srcAdd, destAdd, payload_off, 8);
        delay(delayMs);


    }
    J1939.Transmit(segs[0], priority, srcAdd, destAdd, payload_off, 8);
    J1939.Transmit(segs[1], priority, srcAdd, destAdd, payload_off, 8);
    Serial.print("Detected anticlockwise direction rotation");
}

void animateClockwiseDirection(uint8_t repeats = 1) {
    const uint8_t delayMs = 100;
    Serial.println("Animate Anticlockwise.");
    uint8_t payload_off[8] = {
    black[0], black[1], black[2],
    black[0], black[1], black[2],
    255, 255
    };
    uint8_t payload_1[8] = {
    black[0], black[1], black[2],
    red[0], red[1], red[2],
    255, 255
    };
    uint8_t payload_2[8] = {
    red[0], red[1], red[2],
    black[0], black[1], black[2],
    255, 255
    };    

    
    J1939.Transmit(segs[0], priority, srcAdd, destAdd, payload_off, 8);
    J1939.Transmit(segs[1], priority, srcAdd, destAdd, payload_off, 8);
    for (int i = 0; i < 4; ++i) {
        // Turn on segment 1 only
        J1939.Transmit(segs[0], priority, srcAdd, destAdd, payload_1, 8);
        J1939.Transmit(segs[1], priority, srcAdd, destAdd, payload_off, 8);
        delay(delayMs);
        // Turn on segment 2 only
        J1939.Transmit(segs[1], priority, srcAdd, destAdd, payload_2, 8);
        J1939.Transmit(segs[0], priority, srcAdd, destAdd, payload_off, 8);
        delay(delayMs);
        // Turn on segment 3 only
        J1939.Transmit(segs[1], priority, srcAdd, destAdd, payload_1, 8);
        J1939.Transmit(segs[0], priority, srcAdd, destAdd, payload_off, 8);
        delay(delayMs);
        // Turn on segment 4 only
        J1939.Transmit(segs[0], priority, srcAdd, destAdd, payload_2, 8);
        J1939.Transmit(segs[1], priority, srcAdd, destAdd, payload_off, 8);
        delay(delayMs);
    }
    J1939.Transmit(segs[0], priority, srcAdd, destAdd, payload_off, 8);
    J1939.Transmit(segs[1], priority, srcAdd, destAdd, payload_off, 8);
    Serial.print("Detected clockwise direction rotation");

}

void reset() {
    //Reset the counters:
    counter_segSelection = 0;
    uint8_t payload_off[8] = {
    black[0], black[1], black[2],
    black[0], black[1], black[2],
    255, 255
    };
    J1939.Transmit(segs[0], priority, srcAdd, destAdd, payload_off, 8);
    J1939.Transmit(segs[1], priority, srcAdd, destAdd, payload_off, 8);
    Serial.print("Reset performed");

}



void loop()
{
  //Declarations
  uint8_t nPriority;
  uint8_t nSrcAdd;
  uint8_t nDestAdd;
  uint8_t nData[8];
  int nDataLen;
  long lPGN;


  char sString[80];
  if (J1939.Receive(&lPGN, &nPriority, &nSrcAdd, &nDestAdd, nData, &nDataLen) == 0)
    { 
       
        sprintf(sString, "PGN: 0x%X Src: 0x%X Dest: 0x%X ", (int)lPGN, nSrcAdd, nDestAdd);
        //Serial.print(sString);

        if (int(lPGN) == 0xFF64){   //J1939 PGN: 0xFF64 is meant for the keys on the CR1301
            if (nDataLen == 0 )
            {
                Serial.print("No Data.\n\r");
            }
            else
            {

                Byte a;
                Byte b;
                Byte c;
                Byte d;
                a.byte = nData[1];
                b.byte = nData[2];
                c.byte = nData[3];
                d.byte = nData[4];
                //Prints all the received CAN data:
                // Serial.print("Data: ");
                // for (int nIndex = 0; nIndex < nDataLen; nIndex++)
                // {
                //   sprintf(sString, "0x%X ", nData[nIndex]);
                //   Serial.print(sString);
                // }
                // Serial.print("\n\r");
                //Checking the bit changes for the rotary knob being rotated and pressed.
                // for (int nIndex = 7; nIndex >= 0; nIndex--) 
                // {
                //   Serial.print((c.byte >> nIndex) & 0x01);
                // }

                // Serial.print("\n\r");
  
                //If reset Button is pressed down:                  
                bool resetButton_HasRisingEdge = false;
                if(a.bit0 == 1)
                {
                    //Button is pressed down:
                    if(reset_button_pressed == false){
                        resetButton_HasRisingEdge = true;
                        reset_button_pressed = true;
                    }
                }else{
                    //Button is not pressed down:
                    reset_button_pressed = false;
                }
                
                //If Light Up Button is pressed down:                  
                bool lightUpbutton_HasRisingEdge = false;
                if(a.bit4 == 1)
                {
                    //Button is pressed down:
                    if(lightUpbutton == false){
                        lightUpbutton_HasRisingEdge = true;
                        lightUpbutton = true;
                    }
                }else{
                    //Button is not pressed down:
                    lightUpbutton = false;
                }



                //Calculate Rotary Encoder Delta (Direction + Steps)
                int delta_rotation = c.bit0 * 1 + c.bit1 * 2 + c.bit2 * 4 + c.bit3 * 8 + c.bit4 * 16;
                if(c.bit5 == 1)//Detect rotation direction of encoder
                {
                    delta_rotation = -delta_rotation;
                }
                
                bool rotaryButton_HasRisingEdge = false;

                //If rotary button is pressed
                if(c.bit6 == 1)//Rotary button pressed COLOR Change mode
                {
                    //Button is pressed down:
                    if(rotary_button_pressed== false){
                        rotaryButton_HasRisingEdge = true;
                        rotary_button_pressed = true;
                    }
                }else{
                    //Button is not pressed down:
                    rotary_button_pressed = false;
                }

                switch(state)
                {
                  case DEFAULT_POSITION_TRACKING:
                  {
                  counter_segSelection += delta_rotation;

                    //Limit the encode to values between 0 and 24:
                        if(counter_segSelection<0) {counter_segSelection = 24;
                        }
                        if(counter_segSelection>24) counter_segSelection = 0;
                        while(counter_segSelection>24) counter_segSelection -= 24;
                        while(counter_segSelection<0) counter_segSelection += 24;
                        Serial.println("Counter value");
                        Serial.println(counter_segSelection);
                        //Lighting Up now:
                        if(counter_segSelection == 0)
                        {
                            lightOffAllSegmentsAndKeys();
                        }
                        // else    lightUpAllSegmentsAndKeys(red);
                        if (counter_segSelection>0 && counter_segSelection<=2){    
                            lightUpAllSegments(colorPalette[0]);
                        }
                        if (counter_segSelection>2 && counter_segSelection<=4){
                            lightUpAllSegments(colorPalette[1]);
                        } 
                        if (counter_segSelection>4 && counter_segSelection<=6){
                            lightUpAllSegments(colorPalette[2]);
                        }
                        if (counter_segSelection>6 && counter_segSelection<=8){
                            lightUpAllSegments(colorPalette[3]);
                        } 
                        if (counter_segSelection>8 && counter_segSelection<=10){
                            lightUpAllSegments(colorPalette[4]);
                        }
                        if (counter_segSelection>10 && counter_segSelection<=12){
                            lightUpAllSegments(colorPalette[5]);
                        }
                        if (counter_segSelection>12 && counter_segSelection<=14){
                            lightUpAllSegments(colorPalette[6]);
                        }
                        if (counter_segSelection>14 && counter_segSelection<=16){
                            lightUpAllSegments(colorPalette[7]);
                        }
                        if (counter_segSelection>16 && counter_segSelection<=18){
                            lightUpAllSegments(colorPalette[8]);
                        }
                        if (counter_segSelection>18 && counter_segSelection<=20){
                            lightUpAllSegments(colorPalette[9]);
                        }
                        if (counter_segSelection>20 && counter_segSelection<=22){
                            lightUpAllSegments(colorPalette[10]);
                        }
                        if (counter_segSelection>22 && counter_segSelection<=24){
                            lightUpAllSegments(colorPalette[11]);
                        }

                        

                        if(rotaryButton_HasRisingEdge == true){         //Trigger for rotary button press
                            
                            //if(counter_segSelection != 0) {
                              state = DETECT_DIRECTION; 
                              rotaryButton_HasRisingEdge = false;  
                        //}
                        }
                      
                        if(resetButton_HasRisingEdge == true) reset();  //Trigger for resetbutton press
                        
                        
                    //     if(lightUpbutton_HasRisingEdge == true) {       //Trigger for lightup button press
                    //       state = LIGHT_UP;
                    //       MillisPassed = 0;
                    //     }    
                      }break;


                  case DETECT_DIRECTION: {
                    J1939.Transmit(segs[0], priority, srcAdd, destAdd, payload_off, 8);
                    J1939.Transmit(segs[1], priority, srcAdd, destAdd, payload_off, 8);
                    Serial.println("Detecting direction!");
                    Serial.print("\n \r");
                    if (c.bit0 ==1 && c.bit5 == 1) animateAnticlockwiseDirection();
                    if (c.bit0 == 1 && c.bit5 == 0) animateClockwiseDirection();  
                    
                    
                    if(rotaryButton_HasRisingEdge == true){         //Trigger for rotary button press
                            
                      //if(counter_segSelection != 0) {
                          state = DEFAULT_POSITION_TRACKING; 
                          rotaryButton_HasRisingEdge = false;  
                        //}
                    }
                    if(resetButton_HasRisingEdge == true) reset();
                    // if(lightUpbutton_HasRisingEdge == true) {       //Trigger for lightup button press
                    //       state = LIGHT_UP;
                    //       MillisPassed = 0;
                    //     }    
                    }break;              

                

                }

              }
            }
          }
        }
    
   




