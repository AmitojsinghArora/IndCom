#include <Arduino.h>
#include "SimpleJ1939.hpp"
#include <mcp_can.h>
#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>


//General CAN Setup:
static unsigned constexpr MCP_CS = 5; //Chip Select Pin
MCP_CAN CAN0(MCP_CS);
SimpleJ1939 J1939(&CAN0);


//Define all the colors:
byte black[3] = {0,0,0};
byte yellow[3] = {255,255,0};
byte yellow_orange[3] = {249,170,12};
byte orange[3] = {255,128,0};
byte red_orange[3] = {249,91,12};
byte red[3] = {255,0,0};
byte red_purple[3] = {249,12,162};
byte purple[3] = {204,0,204};
byte blue_purple[3] = {217,12,249};
byte blue[3] = {0,0,255};
byte blue_green[3] = {12,249,194};
byte green[3] = {0,255,0};
byte yellow_green[3] = {139,249,12};



//PGN values for keys and segements:
long keys[3] = {0xFF75,0xFF76,0xFF77};
long segments[2] = {0xFF78,0xFF79};

//CAN declarations:
byte priority = 0x06;
byte srcAddr = 0x87;
byte destAddr = 0xFF;
byte length = 8;

//Counters:
int counter_segmentSelection = 0;
int counter_colorSelectionseg1 = 0;
int counter_colorSelectionseg2 = 0;
int counter_colorSelectionseg3 = 0;
int counter_colorSelectionseg4 = 0;
int counter_automaticMode = 0;

//Which color is selcted right now for manual mode:
// byte currentlySelectedColor[3]={0,0,0};
byte currentlySelectedColorSeg1[3]={0,0,0};
byte currentlySelectedColorSeg2[3]={0,0,0};
byte currentlySelectedColorSeg3[3]={0,0,0};
byte currentlySelectedColorSeg4[3]={0,0,0};

//Which color is selcted right now for automatic mode:
byte currentlySelectedColorInAutomaticMode[3]={0,0,0};
int indexOfSelectedAutomaticColor = 0;
int howManyMillisHavePassed = 0;


//Enum of all possible states:
enum possibleModes{DEFAULT_MANUAL_SWITCHING_ON_SEGMENTS, CHANGE_COLOR, AUTOMATIC_MODE };

//The current state:
int state = DEFAULT_MANUAL_SWITCHING_ON_SEGMENTS;


//Which buttons are currently pressed down:
bool rotatryButtonCurrentlyPressedDown = false;
bool resetButtonCurrentlyPressedDown = false;
bool automaticButtonCurrentlyPressedDown = false;


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
    // Set the serial interface baud rate
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

    //Set currently selected color to yellow:
    //std::copy(std::begin(yellow), std::end(yellow), std::begin(currentlySelectedColor));
    std::copy(std::begin(yellow), std::end(yellow), std::begin(currentlySelectedColorSeg1));
    std::copy(std::begin(yellow), std::end(yellow), std::begin(currentlySelectedColorSeg2));
    std::copy(std::begin(yellow), std::end(yellow), std::begin(currentlySelectedColorSeg3));
    std::copy(std::begin(yellow), std::end(yellow), std::begin(currentlySelectedColorSeg4));


}



void updateColorForAutomaticMode(){
    indexOfSelectedAutomaticColor += 1;
    if(indexOfSelectedAutomaticColor > 12) indexOfSelectedAutomaticColor -= 12;


    if (indexOfSelectedAutomaticColor == 1){
        std::copy(std::begin(yellow_orange), std::end(yellow_orange), std::begin(currentlySelectedColorInAutomaticMode));
        //Serial.print("Color Yellow Orange\n\r");

    }
    if (indexOfSelectedAutomaticColor == 2){
        std::copy(std::begin(orange), std::end(orange), std::begin(currentlySelectedColorInAutomaticMode));
        //Serial.print("Color Orange\n\r");

    }
    if (indexOfSelectedAutomaticColor == 3){
        std::copy(std::begin(red_orange), std::end(red_orange), std::begin(currentlySelectedColorInAutomaticMode));
        //Serial.print("Color Red Orange\n\r");

    }
    if (indexOfSelectedAutomaticColor == 4){
        std::copy(std::begin(red), std::end(red), std::begin(currentlySelectedColorInAutomaticMode));
        //Serial.print("Color Red\n\r");

    }
    if (indexOfSelectedAutomaticColor == 5){
        std::copy(std::begin(red_purple), std::end(red_purple), std::begin(currentlySelectedColorInAutomaticMode));
        //Serial.print("Color Red Purple\n\r");

    }
    if (indexOfSelectedAutomaticColor == 6){
        std::copy(std::begin(purple), std::end(purple), std::begin(currentlySelectedColorInAutomaticMode));
    }
    if (indexOfSelectedAutomaticColor == 7){
        std::copy(std::begin(blue_purple), std::end(blue_purple), std::begin(currentlySelectedColorInAutomaticMode));
    }
    if (indexOfSelectedAutomaticColor == 8){
        std::copy(std::begin(blue), std::end(blue), std::begin(currentlySelectedColorInAutomaticMode));
    }
    if (indexOfSelectedAutomaticColor == 9){
        std::copy(std::begin(blue_green), std::end(blue_green), std::begin(currentlySelectedColorInAutomaticMode));
    }
    if (indexOfSelectedAutomaticColor == 10){
        std::copy(std::begin(green), std::end(green), std::begin(currentlySelectedColorInAutomaticMode));
    }
    if (indexOfSelectedAutomaticColor == 11){
        std::copy(std::begin(yellow_green), std::end(yellow_green), std::begin(currentlySelectedColorInAutomaticMode));
    }
    if (indexOfSelectedAutomaticColor >= 12 || indexOfSelectedAutomaticColor == 0){
        std::copy(std::begin(yellow), std::end(yellow), std::begin(currentlySelectedColorInAutomaticMode));
    }


}







// void updateCurrentlySelectedColor(int counter){
//   if (counter>1 && counter<=3){
//     std::copy(std::begin(yellow_orange), std::end(yellow_orange), std::begin(currentlySelectedColor));
//     Serial.print("Color Yellow Orange\n\r");
//   }
//   if (counter>3 && counter<=5){
//     std::copy(std::begin(orange), std::end(orange), std::begin(currentlySelectedColor));
//      Serial.print("Color Orange\n\r");
//   }
//   if (counter>5 && counter<=7){
//     std::copy(std::begin(red_orange), std::end(red_orange), std::begin(currentlySelectedColor));
//      Serial.print("Color Red Orange\n\r");
//   }
//   if (counter>7 && counter<=9){
//     std::copy(std::begin(red), std::end(red), std::begin(currentlySelectedColor));
//      Serial.print("Color Red\n\r");
//   }
//   if (counter>9 && counter<=11){
//     std::copy(std::begin(red_purple), std::end(red_purple), std::begin(currentlySelectedColor));
//       Serial.print("Color Red Purple\n\r");
//   }
//   if (counter>11 && counter<=13){
//     std::copy(std::begin(purple), std::end(purple), std::begin(currentlySelectedColor));
//   }
//   if (counter>13 && counter<=15){
//     std::copy(std::begin(blue_purple), std::end(blue_purple), std::begin(currentlySelectedColor));
//   }
//   if (counter>15 && counter<=17){
//     std::copy(std::begin(blue), std::end(blue), std::begin(currentlySelectedColor));
//   }
//   if (counter>17 && counter<=19){
//     std::copy(std::begin(blue_green), std::end(blue_green), std::begin(currentlySelectedColor));
//   }
//   if (counter>19 && counter<=21){
//     std::copy(std::begin(green), std::end(green), std::begin(currentlySelectedColor));
//   }
//   if (counter>21 && counter<=23){
//     std::copy(std::begin(yellow_green), std::end(yellow_green), std::begin(currentlySelectedColor));
//   }
//   if (counter==1 || counter==24 || counter==0){
//     std::copy(std::begin(yellow), std::end(yellow), std::begin(currentlySelectedColor));
//   }
// } 




void updateCurrentlySelectedColorSeg1(int counter){
  if (counter>1 && counter<=3){
    std::copy(std::begin(yellow_orange), std::end(yellow_orange), std::begin(currentlySelectedColorSeg1));
    //Serial.print("Color Yellow Orange\n\r");

  }
  if (counter>3 && counter<=5){
    std::copy(std::begin(orange), std::end(orange), std::begin(currentlySelectedColorSeg1));
     //Serial.print("Color Orange\n\r");

  }
  if (counter>5 && counter<=7){
    std::copy(std::begin(red_orange), std::end(red_orange), std::begin(currentlySelectedColorSeg1));
     //Serial.print("Color Red Orange\n\r");

  }
  if (counter>7 && counter<=9){
    std::copy(std::begin(red), std::end(red), std::begin(currentlySelectedColorSeg1));
     //Serial.print("Color Red\n\r");

  }
  if (counter>9 && counter<=11){
    std::copy(std::begin(red_purple), std::end(red_purple), std::begin(currentlySelectedColorSeg1));
      //Serial.print("Color Red Purple\n\r");

  }
  if (counter>11 && counter<=13){
    std::copy(std::begin(purple), std::end(purple), std::begin(currentlySelectedColorSeg1));
  }
  if (counter>13 && counter<=15){
    std::copy(std::begin(blue_purple), std::end(blue_purple), std::begin(currentlySelectedColorSeg1));
  }
  if (counter>15 && counter<=17){
    std::copy(std::begin(blue), std::end(blue), std::begin(currentlySelectedColorSeg1));
  }
  if (counter>17 && counter<=19){
    std::copy(std::begin(blue_green), std::end(blue_green), std::begin(currentlySelectedColorSeg1));
  }
  if (counter>19 && counter<=21){
    std::copy(std::begin(green), std::end(green), std::begin(currentlySelectedColorSeg1));
  }
  if (counter>21 && counter<=23){
    std::copy(std::begin(yellow_green), std::end(yellow_green), std::begin(currentlySelectedColorSeg1));
  }
  if (counter==1 || counter==24 || counter==0){
    std::copy(std::begin(yellow), std::end(yellow), std::begin(currentlySelectedColorSeg1));
  }
} 

void updateCurrentlySelectedColorSeg2(int counter){
  if (counter>1 && counter<=3){
    std::copy(std::begin(yellow_orange), std::end(yellow_orange), std::begin(currentlySelectedColorSeg2));
    //Serial.print("Color Yellow Orange\n\r");

  }
  if (counter>3 && counter<=5){
    std::copy(std::begin(orange), std::end(orange), std::begin(currentlySelectedColorSeg2));
     //Serial.print("Color Orange\n\r");

  }
  if (counter>5 && counter<=7){
    std::copy(std::begin(red_orange), std::end(red_orange), std::begin(currentlySelectedColorSeg2));
     //Serial.print("Color Red Orange\n\r");

  }
  if (counter>7 && counter<=9){
    std::copy(std::begin(red), std::end(red), std::begin(currentlySelectedColorSeg2));
     //Serial.print("Color Red\n\r");

  }
  if (counter>9 && counter<=11){
    std::copy(std::begin(red_purple), std::end(red_purple), std::begin(currentlySelectedColorSeg2));
     // Serial.print("Color Red Purple\n\r");

  }
  if (counter>11 && counter<=13){
    std::copy(std::begin(purple), std::end(purple), std::begin(currentlySelectedColorSeg2));
  }
  if (counter>13 && counter<=15){
    std::copy(std::begin(blue_purple), std::end(blue_purple), std::begin(currentlySelectedColorSeg2));
  }
  if (counter>15 && counter<=17){
    std::copy(std::begin(blue), std::end(blue), std::begin(currentlySelectedColorSeg2));
  }
  if (counter>17 && counter<=19){
    std::copy(std::begin(blue_green), std::end(blue_green), std::begin(currentlySelectedColorSeg2));
  }
  if (counter>19 && counter<=21){
    std::copy(std::begin(green), std::end(green), std::begin(currentlySelectedColorSeg2));
  }
  if (counter>21 && counter<=23){
    std::copy(std::begin(yellow_green), std::end(yellow_green), std::begin(currentlySelectedColorSeg2));
  }
  if (counter==1 || counter==24 || counter==0){
    std::copy(std::begin(yellow), std::end(yellow), std::begin(currentlySelectedColorSeg2));
  }
} 

void updateCurrentlySelectedColorSeg3(int counter){
  if (counter>1 && counter<=3){
    std::copy(std::begin(yellow_orange), std::end(yellow_orange), std::begin(currentlySelectedColorSeg3));
    //Serial.print("Color Yellow Orange\n\r");

  }
  if (counter>3 && counter<=5){
    std::copy(std::begin(orange), std::end(orange), std::begin(currentlySelectedColorSeg3));
     //Serial.print("Color Orange\n\r");

  }
  if (counter>5 && counter<=7){
    std::copy(std::begin(red_orange), std::end(red_orange), std::begin(currentlySelectedColorSeg3));
     //Serial.print("Color Red Orange\n\r");

  }
  if (counter>7 && counter<=9){
    std::copy(std::begin(red), std::end(red), std::begin(currentlySelectedColorSeg3));
     //Serial.print("Color Red\n\r");

  }
  if (counter>9 && counter<=11){
    std::copy(std::begin(red_purple), std::end(red_purple), std::begin(currentlySelectedColorSeg3));
      //Serial.print("Color Red Purple\n\r");

  }
  if (counter>11 && counter<=13){
    std::copy(std::begin(purple), std::end(purple), std::begin(currentlySelectedColorSeg3));
  }
  if (counter>13 && counter<=15){
    std::copy(std::begin(blue_purple), std::end(blue_purple), std::begin(currentlySelectedColorSeg3));
  }
  if (counter>15 && counter<=17){
    std::copy(std::begin(blue), std::end(blue), std::begin(currentlySelectedColorSeg3));
  }
  if (counter>17 && counter<=19){
    std::copy(std::begin(blue_green), std::end(blue_green), std::begin(currentlySelectedColorSeg3));
  }
  if (counter>19 && counter<=21){
    std::copy(std::begin(green), std::end(green), std::begin(currentlySelectedColorSeg3));
  }
  if (counter>21 && counter<=23){
    std::copy(std::begin(yellow_green), std::end(yellow_green), std::begin(currentlySelectedColorSeg3));
  }
  if (counter==1 || counter==24 || counter==0){
    std::copy(std::begin(yellow), std::end(yellow), std::begin(currentlySelectedColorSeg3));
  }
} 

void updateCurrentlySelectedColorSeg4(int counter){
  if (counter>1 && counter<=3){


    std::copy(std::begin(yellow_orange), std::end(yellow_orange), std::begin(currentlySelectedColorSeg4));
    //Serial.print("Color Yellow Orange\n\r");

  }
  if (counter>3 && counter<=5){
    std::copy(std::begin(orange), std::end(orange), std::begin(currentlySelectedColorSeg4));
     //Serial.print("Color Orange\n\r");

  }
  if (counter>5 && counter<=7){
    std::copy(std::begin(red_orange), std::end(red_orange), std::begin(currentlySelectedColorSeg4));
     //Serial.print("Color Red Orange\n\r");

  }
  if (counter>7 && counter<=9){
    std::copy(std::begin(red), std::end(red), std::begin(currentlySelectedColorSeg4));
     //Serial.print("Color Red\n\r");

  }
  if (counter>9 && counter<=11){
    std::copy(std::begin(red_purple), std::end(red_purple), std::begin(currentlySelectedColorSeg4));
      //Serial.print("Color Red Purple\n\r");

  }
  if (counter>11 && counter<=13){
    std::copy(std::begin(purple), std::end(purple), std::begin(currentlySelectedColorSeg4));
  }
  if (counter>13 && counter<=15){
    std::copy(std::begin(blue_purple), std::end(blue_purple), std::begin(currentlySelectedColorSeg4));
  }
  if (counter>15 && counter<=17){
    std::copy(std::begin(blue), std::end(blue), std::begin(currentlySelectedColorSeg4));
  }
  if (counter>17 && counter<=19){
    std::copy(std::begin(blue_green), std::end(blue_green), std::begin(currentlySelectedColorSeg4));
  }
  if (counter>19 && counter<=21){
    std::copy(std::begin(green), std::end(green), std::begin(currentlySelectedColorSeg4));
  }
  if (counter>21 && counter<=23){
    std::copy(std::begin(yellow_green), std::end(yellow_green), std::begin(currentlySelectedColorSeg4));
  }
  if (counter==1 || counter==24 || counter==0){
    std::copy(std::begin(yellow), std::end(yellow), std::begin(currentlySelectedColorSeg4));
  }
} 



void lightUpAllSegmentsAndKeys(byte color[3]){
    byte payload[8] = {0,0,0,0,0,0,255,255};
    payload[0] = color[0];
    payload[1] = color[1];
    payload[2] = color[2];
    payload[3] = color[0];
    payload[4] = color[1];
    payload[5] = color[2];
    J1939.Transmit(segments[0], priority, srcAddr, destAddr, payload, length);
    payload[0] = color[0];
    payload[1] = color[1];
    payload[2] = color[2];
    payload[3] = color[0];
    payload[4] = color[1];
    payload[5] = color[2];
    J1939.Transmit(segments[1], priority, srcAddr, destAddr, payload, length);
    J1939.Transmit(keys[0], priority, srcAddr, destAddr, payload, length);
    J1939.Transmit(keys[1], priority, srcAddr, destAddr, payload, length);
    J1939.Transmit(keys[2], priority, srcAddr, destAddr, payload, length);
    //Serial.print("lightUpAllSegmentsAndKeys\n\r");
}

void lightOffAllSegmentsAndKeys(){
    byte payload[8] = {0,0,0,0,0,0,255,255};
    payload[0] = black[0];
    payload[1] = black[1];
    payload[2] = black[2];
    payload[3] = black[0];
    payload[4] = black[1];
    payload[5] = black[2];
    J1939.Transmit(segments[0], priority, srcAddr, destAddr, payload, length);
    payload[0] = black[0];
    payload[1] = black[1];
    payload[2] = black[2];
    payload[3] = black[0];
    payload[4] = black[1];
    payload[5] = black[2];
    J1939.Transmit(segments[1], priority, srcAddr, destAddr, payload, length);
    J1939.Transmit(keys[0], priority, srcAddr, destAddr, payload, length);
    J1939.Transmit(keys[1], priority, srcAddr, destAddr, payload, length);
    J1939.Transmit(keys[2], priority, srcAddr, destAddr, payload, length);
    Serial.print("lightOffAllSegmentsAndKeys\n\r");

}


void lightUpNothing(){
  byte payload[8] = {0,0,0,0,0,0,255,255};
  payload[0] = black[0];
  payload[1] = black[1];
  payload[2] = black[2];
  payload[3] = black[0];
  payload[4] = black[1];
  payload[5] = black[2];
  J1939.Transmit(segments[0], priority, srcAddr, destAddr, payload, length);
  payload[0] = black[0];
  payload[1] = black[1];
  payload[2] = black[2];
  payload[3] = black[0];
  payload[4] = black[1];
  payload[5] = black[2];
  J1939.Transmit(segments[1], priority, srcAddr, destAddr, payload, length);
  Serial.print("lightUpNothing\n\r");

}

//void lightUpTillSeg1(byte color[3]){
void lightUpTillSeg1(){
  byte payload[8] = {0,0,0,0,0,0,255,255};
  payload[0] = currentlySelectedColorSeg1[0];
  payload[1] = currentlySelectedColorSeg1[1];
  payload[2] = currentlySelectedColorSeg1[2];
  payload[3] = currentlySelectedColorSeg2[0];
  payload[4] = currentlySelectedColorSeg2[1];
  payload[5] = currentlySelectedColorSeg2[2];
  J1939.Transmit(segments[0], priority, srcAddr, destAddr, payload, length);
  payload[0] = currentlySelectedColorSeg3[0];
  payload[1] = currentlySelectedColorSeg3[1];
  payload[2] = currentlySelectedColorSeg3[2];
  payload[3] = currentlySelectedColorSeg4[0];
  payload[4] = currentlySelectedColorSeg4[1];
  payload[5] = currentlySelectedColorSeg4[2];
  J1939.Transmit(segments[1], priority, srcAddr, destAddr, payload, length);
  //Serial.print("lightUpTillSeg1\n\r");

}

void lightUpTillSeg2(){
  byte payload[8] = {0,0,0,0,0,0,255,255};
  payload[0] = black[0];
  payload[1] = black[1];
  payload[2] = black[2];
  payload[3] = currentlySelectedColorSeg2[0];
  payload[4] = currentlySelectedColorSeg2[1];
  payload[5] = currentlySelectedColorSeg2[2];
  J1939.Transmit(segments[0], priority, srcAddr, destAddr, payload, length);
  payload[0] = black[0];
  payload[1] = black[1];
  payload[2] = black[2];
  payload[3] = black[0];
  payload[4] = black[1];
  payload[5] = black[2];
  J1939.Transmit(segments[1], priority, srcAddr, destAddr, payload, length);
  //Serial.print("lightUpTillSeg2\n\r");

}

void lightUpTillSeg3(){
  byte payload[8] = {0,0,0,0,0,0,255,255};
  payload[0] = black[0];
  payload[1] = black[1];
  payload[2] = black[2];
  payload[3] = currentlySelectedColorSeg2[0];
  payload[4] = currentlySelectedColorSeg2[1];
  payload[5] = currentlySelectedColorSeg2[2];
  J1939.Transmit(segments[0], priority, srcAddr, destAddr, payload, length);
  payload[0] = currentlySelectedColorSeg3[0];
  payload[1] = currentlySelectedColorSeg3[1];
  payload[2] = currentlySelectedColorSeg3[2];
  payload[3] = black[0];
  payload[4] = black[1];
  payload[5] = black[2];
  J1939.Transmit(segments[1], priority, srcAddr, destAddr, payload, length);
 // Serial.print("lightUpTillSeg3\n\r");

}

void lightUpTillSeg4(){
  byte payload[8] = {0,0,0,0,0,0,255,255};
  payload[0] = black[0];
  payload[1] = black[1];
  payload[2] = black[2];
  payload[3] = currentlySelectedColorSeg2[0];
  payload[4] = currentlySelectedColorSeg2[1];
  payload[5] = currentlySelectedColorSeg2[2];
  J1939.Transmit(segments[0], priority, srcAddr, destAddr, payload, length);
  payload[0] = currentlySelectedColorSeg3[0];
  payload[1] = currentlySelectedColorSeg3[1];
  payload[2] = currentlySelectedColorSeg3[2];
  payload[3] = currentlySelectedColorSeg4[0];
  payload[4] = currentlySelectedColorSeg4[1];
  payload[5] = currentlySelectedColorSeg4[2];
  J1939.Transmit(segments[1], priority, srcAddr, destAddr, payload, length);
  //Serial.print("lightUpTillSeg4\n\r");

}


void reset(){
        
    printf("Start reset\n\r");

    //Set currently selected color to yellow:
    std::copy(std::begin(yellow), std::end(yellow), std::begin(currentlySelectedColorSeg1));
    std::copy(std::begin(yellow), std::end(yellow), std::begin(currentlySelectedColorSeg2));
    std::copy(std::begin(yellow), std::end(yellow), std::begin(currentlySelectedColorSeg3));
    std::copy(std::begin(yellow), std::end(yellow), std::begin(currentlySelectedColorSeg4));

    //Reset the counters:
    counter_segmentSelection = 0;
    counter_colorSelectionseg1 = 0;
    counter_colorSelectionseg2 = 0;
    counter_colorSelectionseg3 = 0;
    counter_colorSelectionseg4 = 0;
    howManyMillisHavePassed = 0;


    //printf("Finished reset\n\r");

}




void loop()
{
    // Declarations
    byte nPriority;
    byte nSrcAddr;
    byte nDestAddr;
    byte nData[8];
    int nDataLen;
    long lPGN;

    char sString[80];

    // Check for received J1939 messages
    if (J1939.Receive(&lPGN, &nPriority, &nSrcAddr, &nDestAddr, nData, &nDataLen) == 0)
    { 
       


        // auto millisec_since_epoch = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();    
        // printf("howManyMillisHavePassed %d\n", howManyMillisHavePassed);

        sprintf(sString, "PGN: 0x%X Src: 0x%X Dest: 0x%X ", (int)lPGN, nSrcAddr, nDestAddr);
        //Serial.print(sString);

        if (int(lPGN) == 0xFF64){
            if (nDataLen == 0 )
            {
                Serial.print("No Data.\n\r");
            }
            else
            {
                //Prints all the received CAN data:
                Serial.print("Data: ");
                for (int nIndex = 0; nIndex < nDataLen; nIndex++)
                {
                  sprintf(sString, "0x%X ", nData[nIndex]);
                  Serial.print(sString);
                }
                Serial.print("\n\r");
                
                //If reset Button is pressed down:                  
                bool resetButton_HasRisingEdge = false;
                if(nData[1] == 0x40)
                {
                    //Button is pressed down:
                    if(resetButtonCurrentlyPressedDown == false){
                        resetButton_HasRisingEdge = true;
                        resetButtonCurrentlyPressedDown = true;
                    }
                }else{
                    //Button is not pressed down:
                    resetButtonCurrentlyPressedDown = false;
                }
          
          
                   
                //If Automatic Mode Button is pressed down:                  
                bool automaticButton_HasRisingEdge = false;
                if(nData[1] == 0x10)
                {
                    //Button is pressed down:
                    if(automaticButtonCurrentlyPressedDown == false){
                        automaticButton_HasRisingEdge = true;
                        automaticButtonCurrentlyPressedDown = true;
                    }
                }else{
                    //Button is not pressed down:
                    automaticButtonCurrentlyPressedDown = false;
                }
          




                Byte b;
                b.byte = nData[3];

               
                int delta_rotation = b.bit0 * 1 + b.bit1 * 2 + b.bit2 * 4 + b.bit3 * 8 + b.bit4 * 16;
                if(b.bit5 == 1)//Rotary button pressed COLOR Change mode
                {
                    delta_rotation = -delta_rotation;
                }
                
                bool rotatryButton_HasRisingEdge = false;
                if(b.bit6 == 1)//Rotary button pressed COLOR Change mode
                {
                    //Button is pressed down:
                    if(rotatryButtonCurrentlyPressedDown == false){
                        rotatryButton_HasRisingEdge = true;
                        rotatryButtonCurrentlyPressedDown = true;
                    }
                }else{
                    //Button is not pressed down:
                    rotatryButtonCurrentlyPressedDown = false;
                }
          

                



                //printf("Counter Segement is at %d\n", counter_segmentSelection);
                //printf("Counter Color is at %d\n", counter_colorSelection);


                switch (state)
                {

                    case DEFAULT_MANUAL_SWITCHING_ON_SEGMENTS:
                    {
                        

                        counter_segmentSelection += delta_rotation; 

                        //Limit the encode to values between 0 and 24:
                        if(counter_segmentSelection<0) 
                        {
                        counter_segmentSelection = 0;}
                        if(counter_segmentSelection>24) counter_segmentSelection = 24;

                        
                        //printf("From currently selected color G value is:%d\n\r", currentlySelectedColor[1]);

                       
                        ////Now we just light up stuff:
                        
                        if(counter_segmentSelection == 0)
                        {
                            lightUpNothing();
                        }

                        if (counter_segmentSelection>0 && counter_segmentSelection<=6){    
                            lightUpTillSeg2();
                        }
                        if (counter_segmentSelection>6 && counter_segmentSelection<=12){
                            lightUpTillSeg3();
                        } 
                        if (counter_segmentSelection>12 && counter_segmentSelection<=18){
                            lightUpTillSeg4();
                        } 
                        if (counter_segmentSelection>18 && counter_segmentSelection<=24){
                            lightUpTillSeg1();
                        } 



                        if(rotatryButton_HasRisingEdge == true){
                            
                            if(counter_segmentSelection != 0){
                              state = CHANGE_COLOR; 
                            }

                            //Change state
                            //counter_colorSelectionseg1 = 0;
                            //counter_colorSelectionseg2 = 0;
                            //counter_colorSelectionseg3 = 0;
                            //counter_colorSelectionseg4 = 0;
                        }
                        if(resetButton_HasRisingEdge == true){
                            reset();
                        }
                        if(automaticButton_HasRisingEdge == true){
                            state = AUTOMATIC_MODE;
                            howManyMillisHavePassed = 0;
                        }

                        //Serial.print("Hey we are in default mode now\n\r");



                    } break;
                        
                    case CHANGE_COLOR:{

                        if (counter_segmentSelection>0 && counter_segmentSelection<=6){
                            counter_colorSelectionseg2 += delta_rotation; 

                            while(counter_colorSelectionseg2>24) counter_colorSelectionseg2 -= 24;
                            while(counter_colorSelectionseg2<0) counter_colorSelectionseg2 += 24;

                            updateCurrentlySelectedColorSeg2(counter_colorSelectionseg2);
                        }
                        if (counter_segmentSelection>6 && counter_segmentSelection<=12){
                            counter_colorSelectionseg3 += delta_rotation; 
                            while(counter_colorSelectionseg3>24) counter_colorSelectionseg3 -= 24;
                            while(counter_colorSelectionseg3<0) counter_colorSelectionseg3 += 24;
                            updateCurrentlySelectedColorSeg3(counter_colorSelectionseg3);
                        } 
                        if (counter_segmentSelection>12 && counter_segmentSelection<=18){
                            counter_colorSelectionseg4 += delta_rotation; 
                            while(counter_colorSelectionseg4>24) counter_colorSelectionseg4 -= 24;
                            while(counter_colorSelectionseg4<0) counter_colorSelectionseg4 += 24;
                            updateCurrentlySelectedColorSeg4(counter_colorSelectionseg4);
                        } 
                        if (counter_segmentSelection>18 && counter_segmentSelection<=24){
                            counter_colorSelectionseg1 += delta_rotation; 
                            while(counter_colorSelectionseg1>24) counter_colorSelectionseg1 -= 24;
                            while(counter_colorSelectionseg1<0) counter_colorSelectionseg1 += 24;
                            updateCurrentlySelectedColorSeg1(counter_colorSelectionseg1);
                        } 
                        
                        
                    
                        //Light up the particular the LEDs, but just during the color selection mode:
                        if (counter_segmentSelection>0 && counter_segmentSelection<=6){    
                            lightUpTillSeg2();
                        }
                        if (counter_segmentSelection>6 && counter_segmentSelection<=12){
                            lightUpTillSeg3();
                        } 
                        if (counter_segmentSelection>12 && counter_segmentSelection<=18){
                            lightUpTillSeg4();
                        } 
                        if (counter_segmentSelection>18 && counter_segmentSelection<=24){
                            lightUpTillSeg1();
                        } 


                        //Serial.print("Hey we are in color cahnge mode now\n\r");



                        if(rotatryButton_HasRisingEdge == true){
                            state = DEFAULT_MANUAL_SWITCHING_ON_SEGMENTS; //Change state
                        }
                        if(resetButton_HasRisingEdge == true){
                            state = DEFAULT_MANUAL_SWITCHING_ON_SEGMENTS;
                            reset();
                        }
                        if(automaticButton_HasRisingEdge == true){
                            state = AUTOMATIC_MODE;
                        }

                    } break;

                    case AUTOMATIC_MODE:
                    {
                        //Serial.print("Hey we are in automatic mode now\n\r");

                        if(counter_automaticMode<0) counter_automaticMode = 0;
                        if(counter_automaticMode>24) counter_automaticMode = 24;

                        counter_automaticMode += delta_rotation; 

                        howManyMillisHavePassed += 30;
                        int duration = 100 * (counter_automaticMode+1);
                        if(howManyMillisHavePassed > duration){
                            howManyMillisHavePassed -= duration;
                            
                            //now figure out the color in which we should light up all the LEDS 
                            updateColorForAutomaticMode();
                            
                        }

                        //Light up all segments in the color that we figured out: 
                        lightUpAllSegmentsAndKeys(currentlySelectedColorInAutomaticMode);

                        if(resetButton_HasRisingEdge == true){
                            state = DEFAULT_MANUAL_SWITCHING_ON_SEGMENTS;
                            lightOffAllSegmentsAndKeys();
                            reset();
                        }
                        if(automaticButton_HasRisingEdge == true){
                            state = DEFAULT_MANUAL_SWITCHING_ON_SEGMENTS;
                            lightOffAllSegmentsAndKeys();
                        }
                    }break;
                
                
                }
            }    
        }
    }
}