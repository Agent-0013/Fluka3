#include <DueTimer.h>



#define FWver 190801


//************************************ Pin config config ************************************
const byte PWR_OK = 2;
const byte CO2_LED_1 = 6;
const byte CO2_LED_2 = 7;
const byte TRAP_1_LIFT = 14;
const byte TRAP_1_HEAT = 15;
const byte TRAP_2_LIFT = 16;
const byte TRAP_2_HEAT = 17;
const byte C14_skait_0 = 22;
const byte C14_skait_1 = 23;
const byte C14_skait_2 = 24;
const byte C14_skait_3 = 25;
const byte C14_skait_4 = 26;
const byte C14_skait_5 = 27;
const byte C14_skait_6 = 28;
const byte C14_skait_7 = 29;
const byte C14_skait_A = 30;
const byte C14_skait_B = 31;
const byte C14_skait_C = 32;
const byte C14_skait_ADR = 33;
const byte C14_skait_EN = 34;
const byte C14_skait_WR = 35;
const byte Knopke = 20;
//const byte C14_3_skait_EN = 37;
//const byte C14_4_skait_EN = 36;

//const byte C14_skait_Reset = 43;
const byte Relay_0 = 3;
const byte Relay_1 = 36;
const byte Relay_2 = 37;
const byte Relay_3 = 38;
const byte Relay_4 = 39;
const byte Relay_5 = 40;
const byte Relay_6 = 41;
const byte Relay_7 = 42;
const byte Relay_8 = 43;
const byte Relay_9 = 4;
const byte Relay_A = 18;
const byte Relay_B = 5;



const int A_Temp_1 = A0;
const int A_Temp_2 = A1;
const int A_Temp_3 = A2;
const int A_Pres_1 = A3;
const int A_Pres_2 = A4;
const int A_CO2_in_1 = A7;
const int A_CO2_in_2 = A8;
const int A_Flow_1 = A5;
const int A_Flow_2 = A6;
const int A_High_voltage = A9;


//************************************ Mode stuff config ************************************

byte mode_req = 0; // Mode request identifier.
/*  0 - standby
    1 - Sequence 1
    2 - Sequence 2
    3 - ...
    255 - manual control

*/

byte submode = 0; // Used to control function flow in main function.


unsigned int valves_req = 0; // Valve request control register (not only valves, but also trap and heating
/*  bit 0 -
    bit 1 -
    bit 2 -
    bit 3 - ...
    bit 15 - ...

*/

byte mode = 0; // Mode identifier. Working in this one

unsigned int valves = 0; // Valve control register. Working in this one

volatile byte ErrorCode = 0;

byte InfoIndex = 0;         // Info call is [dataFreq] times per second, [dataFreq] time frames, index is time frame number (0 is system state, other CO2 buffer reads)

bool update = false;        // Update valves, mode and stuff

bool timeForData = false;   // This is true [dataFreq] times a second
const byte dataFreq = 4;    // At this frequency data aquisition interrupt is fired.
const byte CO2Freq = 100;     // At this frequency CO2 data is read. Faster is better.

unsigned int C14_1 = 0;     // C14 counters
unsigned int C14_2 = 0;
unsigned int C14_3 = 0;
unsigned int C14_4 = 0;

unsigned int CO2buffer1[100] = {0};       // Buffers and stuff for CO2 measurements
unsigned int CO2buffer2[100] = {0};
byte CO2bufferIndex = 0;
bool CO2buffer2Sel = false;
bool CO2buffer1Ready = false;
bool CO2buffer2Ready = false;




//************************************ Delay stuff config ************************************

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
unsigned long ButtonPushMillis = 0;
unsigned long ButtonNowMillis = 0;
unsigned long HeatStartMillis = 0;
unsigned long HeatNowMillis = 0;
byte HeatNumber = 0;                // 0 - none of the traps heating, 1 - 1 trap heating, 4 - second trap heating, 5 - bad
bool ButtonStatus = false;
unsigned long SampleTime = 60000;  // set to length of first interval.
unsigned long HeatTime = 1500;     // heat for 1500ms
unsigned long FlushTime = 5000;   // Initial trap flush time (5s)
unsigned long CoolTime = 30000;   // Trap cooling delay
unsigned long C14Time = 60000;   // C14 measuring delay



//************************************ Serial parser config************************************

char serialString[64] = {0};         // a String to hold incoming data
char tempChars[64] = {0};        // temporary array for use when parsing
bool stringComplete = false;  // whether the string is complete
bool Receiving = false;       // 1 while receiving data
char stringidx = 0;



//----------------------------------Void  setup --------------------------------------------
void setup()
{


  Serial.begin(115200);
  Serial.print("Arduino control module, FW version ");
  Serial.print(FWver, DEC);
  Serial.println(" ;");
  //  serialString.reserve(200);    // Reserve 64 bytes for serial message


  //************************************ port setup************************************


  pinMode(PWR_OK, INPUT_PULLUP);   // PWR_OK
  pinMode(CO2_LED_1, OUTPUT);   // CO2_LED_1
  pinMode(CO2_LED_2, OUTPUT);   // CO2_LED_2

  pinMode(TRAP_1_LIFT, OUTPUT);   // TRAP_1_LIFT
  pinMode(TRAP_1_HEAT, OUTPUT);   // TRAP_1_HEAT
  pinMode(TRAP_2_LIFT, OUTPUT);   // TRAP_2_LIFT
  pinMode(TRAP_2_HEAT, OUTPUT);   // TRAP_2_HEAT

  pinMode(C14_skait_0, INPUT_PULLUP);   // C14_skait_0
  pinMode(C14_skait_1, INPUT_PULLUP);   // C14_skait_1
  pinMode(C14_skait_2, INPUT_PULLUP);   // C14_skait_2
  pinMode(C14_skait_3, INPUT_PULLUP);   // C14_skait_3
  pinMode(C14_skait_4, INPUT_PULLUP);   // C14_skait_4
  pinMode(C14_skait_5, INPUT_PULLUP);   // C14_skait_5
  pinMode(C14_skait_6, INPUT_PULLUP);   // C14_skait_6
  pinMode(C14_skait_7, INPUT_PULLUP);   // C14_skait_7


  pinMode(C14_skait_A, OUTPUT);   // C14_skait_A
  pinMode(C14_skait_B, OUTPUT);   // C14_skait_B
  pinMode(C14_skait_C, OUTPUT);   // C14_skait_C
  pinMode(C14_skait_ADR, OUTPUT);   // C14_skait_ADR
  pinMode(C14_skait_EN, OUTPUT);   // C14_skait_EN
  pinMode(C14_skait_WR, OUTPUT);   // C14_skait_WR
  //pinMode(C14_skait_Reset1, OUTPUT);   // C14_skait_Reset
  //pinMode(C14_skait_Reset2, OUTPUT);   // C14_skait_Reset

  pinMode(Relay_0, OUTPUT);   // Relay_0
  pinMode(Relay_1, OUTPUT);   // Relay_1
  pinMode(Relay_2, OUTPUT);   // Relay_2
  pinMode(Relay_3, OUTPUT);   // Relay_3
  pinMode(Relay_4, OUTPUT);   // Relay_4
  pinMode(Relay_5, OUTPUT);   // Relay_5
  pinMode(Relay_6, OUTPUT);   // Relay_6
  pinMode(Relay_7, OUTPUT);   // Relay_7
  pinMode(Relay_8, OUTPUT);   // Relay_8
  pinMode(Relay_9, OUTPUT);   // Relay_9
  pinMode(Relay_A, OUTPUT);   // Relay_A
  pinMode(Relay_B, OUTPUT);   // Relay_B

  pinMode(Knopke, INPUT_PULLUP);

  analogReadResolution(12);   // Set analog read resolution


  //************************************ Configure CO2 stuff ************************************

  Timer3.attachInterrupt(GETinfo).setFrequency(dataFreq).start();
  Timer6.attachInterrupt(getCO2).setFrequency(CO2Freq);
  //Serial.print("Available timer: ");
  //Serial.println(Timer.getAvailable());


  SYSinit();   // set default control and system states (to be used later)




  //************************************ Let the show begin ************************************

}   // End of void setup()



//---------------------------------- void loop () --------------------------------------------

void loop()
{

  //unsigned long currentMillis = millis();   // For everything to work correctly

  //if (currentMillis - previousMillis >= interval)


  switch (mode)
  {
    //******************************************************************************** Standby MODE ************************************
    case 0:
      {
        switch (submode)
        {
          case 0:   // Initalize the system to default state
            {
              SYSinit();
              submode++;
              break;
            }
          default:
            {
              break;
            }
        }// End of switch (submode)
        break;
      }
    //******************************************************************************** Prepare for sampling MODE  ************************************
    case 1:   // Prepare for sampling
      {
        switch (submode)
        {
          case 0:   // Configure valves for flushing
            {
              setValve(Relay_2, 1);     // 1 is OFF, 0 is ON for relays, normal for TRAPs
              setValve(Relay_3, 1);
              setValve(Relay_4, 1);
              setValve(Relay_6, 1);
              setValve(Relay_7, 1);
              setValve(Relay_8, 1);

              setValve(Relay_1, 0);
              setValve(Relay_5, 0);
              setValve(Relay_9, 0);

              setValve(TRAP_1_LIFT, 0);     // All N2 cylinders down
              setValve(TRAP_2_LIFT, 0);
              previousMillis = millis();
              submode++;
              break;
            }
          case 1:   // Preheat trap 1
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= 1000)  // Delay 1000ms for both N2 cylinders to go down, then start preheating
              {
                setValve(TRAP_1_HEAT, 1);   // Turn ON trap 1 heating
                previousMillis = millis();
                submode++;
              };
              break;
            }
          case 2:   // Wait for trap 1 to preheat, then start preheating trap 2
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= 1000)  // Delay 1s for trap 1 to preheat
              {
                setValve(TRAP_1_HEAT, 0);   // Turn OFF trap 1 heating
                setValve(TRAP_2_HEAT, 1);   // Turn ON trap 2 heating
                previousMillis = millis();
                submode++;
              };
              break;
            }
          case 3: // Wait for trap 2 to preheat, then start flushing
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= 1000)  // Delay 1s for trap 2 to preheat, and start flushing
              {
                setValve(TRAP_2_HEAT, 0);   // Turn OFF trap 1 heating
                previousMillis = millis();
                submode++;
              };
              break;
            }
          case 4:   // Wait for flust to finish, and go to default state
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= FlushTime) //SampleTime)  // Delay "FlushTime" for all shit to go out and return to default state
              {
                setValve(Relay_1, 1);
                setValve(Relay_5, 1);
                setValve(Relay_9, 1);
                mode_req = 2;
                update = true;
              }
              break;
            }
          default:
            {
              mode_req = 0;
              update = true;
              break;
            }
        } // End of switch (submode)

        break;
      }   // End od case mode = 1

    //******************************************************************************** Trap 1 cooling MODE ************************************

    case 2:   // Trap 1 cooling
      {
        switch (submode)
        {
          case 0:   // lift 1 N2 cylinder and start cooling it
            {
              setValve(TRAP_1_LIFT, 1);     // Raise 1 N2 cylinder
              previousMillis = millis();
              submode++;
              break;
            }
          case 1:   // Wait for trap to cool
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= CoolTime)  // When trap is cool
              {
                mode_req = 3;
                update = true;
              };
              break;
            }
          default:
            {
              mode_req = 0;
              update = true;
              break;
            }
        } // End of switch (submode)
        break;

      }     // End of case mode = 2

    //******************************************************************************** Sampling MODE ************************************

    case 3:   // Sample going into trap
      {
        switch (submode)
        {
          case 0:   // Start sample counter
            {
              previousMillis = millis();
              submode++;
              break;
            }
          case 1: // Wait for sample to accumulate in trap 1
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= SampleTime)  // when sampling done
              {
                mode_req = 4;
                update = true;
              };
              break;
            }
          default:
            {
              mode_req = 0;
              update = true;
              break;
            }
        }   // End of switch (submode)
        break;
      }   // End of case mode = 3

    //******************************************************************************** CO2 measuring MODE ************************************

    case 4:   // Measure CO2 (a lot of stuff happens here)
      {
        switch (submode)
        {
          case 0:   // Connect valves for CO2 measurements, lift 2 trap, start cooling trap 2, start CO2 measurements
            {
              setValve(Relay_2, 0);
              setValve(Relay_3, 0);
              setValve(Relay_4, 0);
              setValve(Relay_5, 0);
              setValve(TRAP_2_LIFT, 1);
              StartCO2();
              previousMillis = millis();
              submode++;
              break;
            }
          case 1:   // Wait for trap 2 to cool, release 1 N2 cylinder, 
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= CoolTime)  // When trap 2 is cool
              {
                setValve(TRAP_1_LIFT, 0);
                //Serial.print("StartCO2!!!!!!!!!!!!");
                
                //StartCO2;
                previousMillis = millis();
                submode++;
              };
              break;
            }
          case 2:   // Wait 0.5S until N2 is down and start trap 1 heating
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= 500)  // When N2 is down
              {
                setValve(TRAP_1_HEAT, 1);
                previousMillis = millis();
                submode++;
              }
              break;
            }

          case 3:   // Wait "HeatTime" for trap 1
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= HeatTime)  // When trap 1 is fully heated
              {
                setValve(TRAP_1_HEAT, 0);
                previousMillis = millis();
                submode++;
              }
              break;
            }
          case 4: // Wait for shit to settle down
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= 5000)  // When everything is calm again
              {
                setValve(Relay_9, 0);   // Open carrier gas
                setValve(Relay_1, 0);
                setValve(Relay_2, 1);
                previousMillis = millis();
                submode++;
              }
              break;
            }
          case 5:   // Measure CO2 going throug column
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= 60000)  // When everything is gone (after 60 seconds)
              {
                setValve(Relay_4, 1);   // Lock out CO2 camera
                StopCO2();
                mode_req = 5;
                update = true;
              }
              break;
            }
          default:
            {
              mode_req = 0;
              update = true;
              break;
            }
        }   // End of switch (submode)
        break;
      }   // End of case mode = 4

    //******************************************************************************** Measure beta MODE ************************************

    case 5:   // Measure beta counts
      {
        switch (submode)
        {
          case 0:   // Configure valves
            {
              setValve(Relay_5, 1);
              setValve(Relay_6, 0);
              setValve(Relay_7, 0);
              setValve(Relay_B, 1);     // Relay B is vacuum pump
              setValve(Relay_0, 1);     // Relay 0 is vacuum valve
              submode++;
              break;
            }
          case 1:   // Wait for vacuum
            {
              if (analogRead(A_Pres_2) >= 1992)    // When vacuum is OK  // <<<<<<-------------------------------------    Here will be vacuum waiting variable, but later
              {
                ResetC14();
                submode++;
              }
              break;
            }
          case 2:   // Prepare for trap heating
            {
              setValve(Relay_7, 1);   // Lock C14 camera
              setValve(Relay_0, 0);   // Close vacuum valve
              setValve(Relay_8, 0);   // in case of emergency release all shit to water
              setValve(TRAP_2_LIFT, 0);
              previousMillis = millis();
              submode++;
              break;
            }
          case 3:   // Wait for trap 2 to go down
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= 500)  // When trap 2 is down
              {
                setValve(TRAP_2_HEAT, 1);     // Start heating trap 2
                setValve(Relay_B, 0);   // Turn off vacuum pump
                previousMillis = millis();
                submode++;
              }
              break;
            }
          case 4:   // Wait for trap 2 heat
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= HeatTime)  // When trap 2 is fully heated
              {
                setValve(TRAP_2_HEAT, 0);     // Stop heating trap 2
                previousMillis = millis();
                submode++;
              }
              break;
            }
          case 5:   // Wait for C14 measurements and prepare for flushing C14 camera
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= C14Time)  // When C14 is measured enough
              {
                setValve(Relay_4, 1);       // Start filling C14 camera with carrier gas
                setValve(Relay_5, 0);
                setValve(Relay_9, 0);
                previousMillis = millis();
                submode++;
              }
              break;
            }
          case 6:   // wait for camera to fill with carrier gas
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= 5000)  // When C14 camera is filled enough
              {
                //setValve(Relay_8, 0);
                setValve(Relay_7, 0);         // Start venting to water
                mode_req = 6;
                update = true;
              }
              break;
            }

          default:
            {
              mode_req = 0;
              update = true;
              break;
            }
        }   // End of switch (submode)
        break;
      }     // End of case mode = 5


    //******************************************************************************** Vent MODE ************************************
    case 6:   // Vent everything and prepare system for second measurement
      {
        switch (submode)
        {
          case 0:   // Configure valves for venting
            {
              setValve(Relay_9, 0);
              setValve(Relay_4, 1);
              setValve(Relay_5, 0);
              setValve(Relay_6, 0);
              setValve(Relay_7, 0);
              setValve(Relay_8, 0);

              setValve(Relay_1, 0);
              setValve(Relay_2, 1);
              setValve(Relay_3, 0);
              previousMillis = millis();
              submode++;
              break;
            }
          case 1:   // Vent C14 camera through water
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= 1000) //180000  // When C14 camera is vented enough
              {
                setValve(Relay_6, 1);   //  Change the way of carrier gas to vent CO2 camera and both traps
                setValve(Relay_7, 1);
                setValve(Relay_8, 1);
                setValve(Relay_4, 0);
                previousMillis = millis();
                submode++;
              }
              break;
            }
          case 2:   // vent traps with carrier gas
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= 1000) // 120000 // When traps is vented enough
              {
                mode_req = 0;       // Go to standby
                update = true;
              }
              break;
            }

          default:
            {
              mode_req = 0;
              update = true;
              break;
            }
        }   // End of switch (submode)
        break;
      }   // End of case mode = 6

    //******************************************************************************** Valve sewuencer ************************************
    case 254:   // Vacuum test
      {
        switch (submode)
        {
          case 0:   // Comment
            {
              SYSinit();
              previousMillis = millis();
              submode++;

              break;
            }
          case 1:   // Comment
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= 1000)  // When 1 second passed
              {
                setValve(Relay_3, 0);
                setValve(Relay_4, 0);
                setValve(Relay_5, 0);
                setValve(Relay_6, 0);
                setValve(Relay_7, 0);
                //setValve(Relay_7, 0);
                //previousMillis = millis();
                submode++;
              }
              break;
            }

          case 2:   // Comment
            {
              break;
              currentMillis = millis();
              if (currentMillis - previousMillis >= 60000)  // When 1 second passed
              {
                //setValve(Relay_1, 1);
                setValve(Relay_6, 0);
                previousMillis = millis();
                submode++;
              }
              break;
            }

          case 3:   // Comment
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= 60000)  // When 1 second passed
              {
                setValve(Relay_5, 0);
                setValve(Relay_4, 0);
                previousMillis = millis();
                submode++;
              }
              break;
            }

          case 4:   // Comment
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= 60000)  // When 1 second passed
              {
                setValve(Relay_3, 0);
                //setValve(Relay_4, 0);
                previousMillis = millis();
                submode++;
              }
              break;
            }

          case 5:   // Comment
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= 60000)  // When 1 second passed
              {
                submode = 0;
                //setValve(Relay_4, 1);
                //setValve(Relay_5, 0);
                //previousMillis = millis();
                //submode++;
              }
              break;
            }

          case 6:   // Comment
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= 1000)  // When 1 second passed
              {
                setValve(Relay_5, 1);
                setValve(Relay_6, 0);
                previousMillis = millis();
                submode++;
              }
              break;
            }

          case 7:   // Comment
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= 1000)  // When 1 second passed
              {
                setValve(Relay_6, 1);
                setValve(Relay_7, 0);
                previousMillis = millis();
                submode++;
              }
              break;
            }

          case 8:   // Comment
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= 1000)  // When 1 second passed
              {
                setValve(Relay_7, 1);
                setValve(Relay_8, 0);
                previousMillis = millis();
                submode++;
              }
              break;
            }

          case 9:   // Comment
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= 1000)  // When 1 second passed
              {
                setValve(Relay_8, 1);
                setValve(Relay_9, 0);
                previousMillis = millis();
                submode++;
              }
              break;
            }

          case 10:   // Comment
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= 1000)  // When 1 second passed
              {
                setValve(Relay_9, 1);
                submode = 0;
                //setValve(Relay_8, 0);
                //previousMillis = millis();
                //submode++;
              }
              break;
            }

          default:
            {
              mode_req = 0;
              update = true;
              break;
            }
        }   // End of switch (submode)
        break;
      }   // End of case mode = 254

    /*
                      MODE EXAMPLE

        //******************************************************************************** WTF MODE ************************************
        case X:   // What is going on here
          {
            switch (submode)
            {
              case 0:   // Comment
                {
                  SYSinit();
                  submode++;
                  break;
                }
              default:
                {
                  mode_req = 0;
                  update = true;
                  break;
                }
            }   // End of switch (submode)
            break;
          }   // End of case mode =


    */
    //******************************************************************************** External control MODE ************************************

    case 255:
      {
        // Use currentmilllis and previousmillis here to control trap heat time in manual mode        <<<<<--------------------------------------------------------- !!!!!!!!!!!!!!
        break;
      }

    default:
      {
        Serial.println("Undefined mode!!!");
        break;
      }

  } // End of switch (mode)









  //************************************ Update mode and valves ************************************
  if (update)     // if something changed, update it
  {
    Update();
  }

  //************************************ Trap heat failsafe ************************************

  if ((valves & 0x02) && (valves & 0x08))    // If two traps is heating at the same time
  {
    setValve(TRAP_1_HEAT, 0);
    setValve(TRAP_2_HEAT, 0);
    HeatStartMillis = millis();
    HeatNumber = 0;
    //Serial.println("Shit 1");
  };    // End of if two traps heating


  if ((valves & 0x02) || (valves & 0x08))    // If heating process is going (bit 2 or 4 is set)
  {
    //Serial.println(HeatNumber);
    //Serial.println(HeatStartMillis);
    
    if ((valves & 0x0A) == HeatNumber)    // If nothing in trap heat is changed
    {
      //Serial.println("NOTHING");
      HeatNowMillis = millis();   // Check how long
      if (HeatNowMillis - HeatStartMillis >= HeatTime + 200) // if longer than heattime + 200ms
      {
        switch (HeatNumber)
        {
          case 2:
            {
              setValve(TRAP_1_HEAT, 0);
              // Serial.println("Shit 2");
              break;
            }
          case 8:
            {
              setValve(TRAP_2_HEAT, 0);
              // Serial.println("Shit 3");
              break;
            }
          default:
            {
              setValve(TRAP_1_HEAT, 0);
              setValve(TRAP_2_HEAT, 0);
              // Serial.println("Shit 4");
              break;
            }
        }  // End of switch HeatNumber
        HeatStartMillis = millis();
        HeatNumber = 0;
      };   // End of if longer than heattime + 200ms
    }   // End of If nothing changed
    else
    {
      // Serial.println("STATECHANGE");
      HeatNumber = (valves & 0x0A);   // Set what traps is heating
      HeatStartMillis = millis();      // Reset counter
    };

  } // End of If heating process was started then
  else
  {
    HeatNumber = 0;   // Set what traps is heating
    HeatStartMillis = millis();
  };




  //************************************ Decode Serial Command ************************************
  if (stringComplete) {
    strcpy(tempChars, serialString);  // Copy peceived string to temp buffer
    Serial.print("OK: ");
    Serial.println(tempChars);
    parseData();


    for (byte i = 0; i < 64; i++)
    {
      serialString[i] = 0;
      tempChars[i] = 0;
      //      parsedString[i] = 0;
    }
    // clear the string:
    //serialString = {0};
    stringComplete = false;
  }

  //************************************ Print system status (and CO2 data) to serial ************************************
  if (timeForData)
  {
    switch (InfoIndex)
    {
      case 0:
        {
          GetC14();
          Serial.print("DATA: ");
          Serial.print("T1=");
          Serial.print(analogRead(A_Temp_1), HEX);
          Serial.print(";T2=");
          Serial.print(analogRead(A_Temp_2), HEX);
          Serial.print(";T3=");
          Serial.print(analogRead(A_Temp_3), HEX);
          Serial.print(";P1=");
          Serial.print(analogRead(A_Pres_1), HEX);
          Serial.print(";P2=");
          Serial.print(analogRead(A_Pres_2), HEX);
          Serial.print(";F1=");
          Serial.print(analogRead(A_Flow_1), HEX);
          Serial.print(";F2=");
          Serial.print(analogRead(A_Flow_2), HEX);
          Serial.print(";HV=");
          Serial.print(analogRead(A_High_voltage), HEX);
          Serial.print(";C1=");
          Serial.print(C14_1, HEX);
          Serial.print(";C2=");
          Serial.print(C14_2, HEX);
          Serial.print(";C3=");
          Serial.print(C14_3, HEX);
          Serial.print(";C4=");
          Serial.print(C14_4, HEX);
          Serial.print(";M0=");
          Serial.print(mode);
          Serial.print(";M1=");
          Serial.print(submode);
          Serial.print(";VV=");
          Serial.print(valves, HEX);
          Serial.print(";ER=");
          Serial.print(ErrorCode);
          Serial.println(";");
        }

      default:
        {
          printCO2();
        }

    }     // End of switch (InfoIndex)

    timeForData = false;

  };       // End of if TimeForData

  //************************************ React to big red button ************************************

  if (digitalRead(Knopke) == 0)
  {
    if (ButtonStatus)     // If button was pressed earlier
    {
      ButtonNowMillis = millis();
      if (ButtonNowMillis - ButtonPushMillis >= 3000)  // button is pressed longer than 3 seconds
      {
        if (mode == 0)      // If in standby change to run test
        {
          mode_req = 1;
          update = true;
          ButtonStatus = false;

        } else              // Else goto standby
        {
          mode_req = 0;
          update = true;
          ButtonStatus = false;
        }
      }   // End of if longer than 3 seconds
    } else      // If first time button press
    {
      ButtonStatus = true;      // Change status to pushed
      ButtonPushMillis = millis();    // Start counting
    }

    // ButtonPushMillis = 0;
    // ButtonNowMillis = 0;
    // ButtonStatus = flase;
  }       // End of if Knopke pushed
  else
  {
    ButtonStatus = false;     // Reset button flag
  }   // End of if else knopke pushed



} // End of void loop()














//---------------------------------- SYSinit function --------------------------------------------
void SYSinit (void)
{
  digitalWrite(CO2_LED_1, LOW);
  digitalWrite(CO2_LED_2, LOW);
  digitalWrite(TRAP_1_LIFT, LOW);
  digitalWrite(TRAP_1_HEAT, LOW);
  digitalWrite(TRAP_2_LIFT, LOW);
  digitalWrite(TRAP_2_HEAT, LOW);


  digitalWrite(C14_skait_A, LOW);
  digitalWrite(C14_skait_B, LOW);
  digitalWrite(C14_skait_C, LOW);
  digitalWrite(C14_skait_ADR, LOW);
  digitalWrite(C14_skait_EN, LOW);
  digitalWrite(C14_skait_WR, LOW);
  //digitalWrite(C14_skait_Reset1, LOW);
  //digitalWrite(C14_skait_Reset2, LOW);

  digitalWrite(Relay_0, LOW);
  digitalWrite(Relay_1, HIGH);
  digitalWrite(Relay_2, HIGH);
  digitalWrite(Relay_3, HIGH);
  digitalWrite(Relay_4, HIGH);
  digitalWrite(Relay_5, HIGH);
  digitalWrite(Relay_6, HIGH);
  digitalWrite(Relay_7, HIGH);
  digitalWrite(Relay_8, HIGH);
  digitalWrite(Relay_9, LOW);
  digitalWrite(Relay_A, LOW);
  digitalWrite(Relay_B, LOW);
  getValve();
  valves_req = valves;
  ResetC14();
  StopCO2();

}
//---------------------------------- Getinfo function (now only sets flag) --------------------------------------------
void GETinfo(void)
{
  if (InfoIndex >= (dataFreq - 1))
  {
    InfoIndex = 0;
  } else
  {
    InfoIndex++;
  }
  timeForData = true;
}

//---------------------------------- Change tracking function --------------------------------------------


//---------------------------------- Serial event function --------------------------------------------
void serialEvent() {
  while (Serial.available())
  {
    // get the new byte:
    char inChar = (char)Serial.read();

    if ((inChar == '>') && (Receiving == true))
    {
      Receiving = false;
      stringComplete = true;
    } else

      if ((Receiving == true) && (inChar != '<'))
      {
        serialString[stringidx] = inChar;
        stringidx++;
      } else

        if (inChar == '<')
        {
          Receiving = true;
          stringidx = 0;
        };

  }
}

//---------------------------------- Parse data function --------------------------------------------

void parseData() {      // split the data into its parts
  Serial.println("Parsing...");

  char * strtokIndx; // this is used by strtok() as an index
  char parsedString[64] = {0};
  bool completed = 0;
  unsigned int tmpval = 0;

  strtokIndx = strtok(tempChars, "=");      // get the command word from string
  strcpy(parsedString, strtokIndx);         // Copy the command word from string for later comparison

  while (completed == 0)
  {

    //  Serial.print("ParsedString value = ");
    //  Serial.println(parsedString);


    if (strcmp(parsedString, "SETMODE") == 0)   // == 0 Because someone thought reverse logic will be fine :D
    {
      strtokIndx = strtok(NULL, ";"); // this continues where the previous call left off, find digit after = until ;
      tmpval = atoi(strtokIndx);      // Convert it to integer
      if (tmpval < 256)               // Check if command is OK
      {
        mode_req = tmpval;
      } else
      {
        mode_req = 0;
      };
      update = true;
      Serial.print("Set mode: ");
      Serial.println(mode_req);
      parsedString[0] = '\0';
    } // End of "SETMODE"

    else if (strcmp(parsedString, "COMMAND") == 0)
    {
      strtokIndx = strtok(NULL, ";"); // this continues where the previous call left off, find digit after = until ;
      tmpval = atoi(strtokIndx);      // Convert it to integer
      if ((tmpval < 65536) && (tmpval >= 0) && (mode == 255)) // Check if command is OK
      {
        valves_req = tmpval;
        update = true;
        Serial.print("Command: ");
        Serial.println(valves_req, BIN);
      } else
      {
        Serial.println("BAD Command or mode =/= 255!");
      };

      parsedString[0] = '\0';
    }   // End of "COMMAND"

    else if (strcmp(parsedString, "STIME") == 0)    // Sample time external setting
    {
      strtokIndx = strtok(NULL, ";"); // this continues where the previous call left off, find digit after = until ;
      tmpval = atoi(strtokIndx);      // Convert it to integer
      if ((tmpval <= 600) && (tmpval > 1)) // Check if command is OK
      {
        SampleTime = tmpval * 1000;
        //update = true;
        Serial.print("Set sample time: ");
        Serial.print(SampleTime / 1000);
        Serial.println(" seconds");
      } else
      {
        Serial.println("BAD sample time parameter < 1s or > 600s");
      };

      parsedString[0] = '\0';
    }   // End of "STIME"

    else if (strcmp(parsedString, "CTIME") == 0)     // Cool time external setting
    {
      strtokIndx = strtok(NULL, ";"); // this continues where the previous call left off, find digit after = until ;
      tmpval = atoi(strtokIndx);      // Convert it to integer
      if ((tmpval <= 600) && (tmpval > 1)) // Check if command is OK
      {
        CoolTime = tmpval * 1000;
        //update = true;
        Serial.print("Set trap cool time: ");
        Serial.print(CoolTime / 1000);
        Serial.println(" seconds");
      } else
      {
        Serial.println("BAD trap cool time parameter < 1s or > 600s");
      };

      parsedString[0] = '\0';
    }   // End of "CTIME"

    else if (strcmp(parsedString, "HTIME") == 0)     // Trap heat time setting
    {
      strtokIndx = strtok(NULL, ";"); // this continues where the previous call left off, find digit after = until ;
      tmpval = atoi(strtokIndx);      // Convert it to integer
      if ((tmpval <= 2000) && (tmpval > 500)) // Check if command is OK
      {
        HeatTime = tmpval;
        //update = true;
        Serial.print("Set trap heat time: ");
        Serial.print(HeatTime);
        Serial.println(" ms");
      } else
      {
        Serial.println("BAD trap heat time parameter < 500 ms or > 2000 ms");
      };

      parsedString[0] = '\0';
    }   // End of "HTIME"

    else if (strcmp(parsedString, "FTIME") == 0)     // Trap flush heat time setting
    {
      strtokIndx = strtok(NULL, ";"); // this continues where the previous call left off, find digit after = until ;
      tmpval = atoi(strtokIndx);      // Convert it to integer
      if ((tmpval <= 60) && (tmpval > 1)) // Check if command is OK
      {
        FlushTime = tmpval * 1000;
        //update = true;
        Serial.print("Set trap flush time: ");
        Serial.print(FlushTime / 1000);
        Serial.println(" seconds");
      } else
      {
        Serial.println("BAD trap flush time parameter < 1 s or > 60 s");
      };

      parsedString[0] = '\0';
    }   // End of "FTIME"

    else if (strcmp(parsedString, "BTIME") == 0)     // Beta counter time setting
    {
      strtokIndx = strtok(NULL, ";"); // this continues where the previous call left off, find digit after = until ;
      tmpval = atoi(strtokIndx);      // Convert it to integer
      if ((tmpval <= 600) && (tmpval > 1)) // Check if command is OK
      {
        C14Time = tmpval * 1000;
        //update = true;
        Serial.print("Set Beta count time: ");
        Serial.print(C14Time / 1000);
        Serial.println(" seconds");
      } else
      {
        Serial.println("BAD Beta count time parameter < 1 s or > 600 s");
      };

      parsedString[0] = '\0';
    }   // End of "BTIME"

    else if (strcmp(parsedString, "ABORT") == 0)     // Beta counter time setting
    {
      strtokIndx = strtok(NULL, ";"); // this continues where the previous call left off, find digit after = until ;
      tmpval = atoi(strtokIndx);      // Convert it to integer
      mode_req = 0;
      update = true;
      

      parsedString[0] = '\0';
    }   // End of "ABORT"


    else
    {
      Serial.println("Unknown command!");
      parsedString[0] = '\0';
    };

    strtokIndx = strtok(NULL, "=;");      // get the command word from string
    strcpy(parsedString, strtokIndx);         // Copy the command word from string for later comparison
    if (strcmp(parsedString, "") == 0)
    {
      //Serial.println("END;");
      completed = 1;
    }


  } // End of while completed

}
//---------------------------------- setValve function --------------------------------------------
void setValve (byte device, bool state)   // Set the requested valve to requested state. Same as digitalWrite, but with status aupdate
{
  digitalWrite(device, state);
  getValve();
  valves_req = valves;
}
//---------------------------------- getValve function --------------------------------------------
void getValve (void)  // Valve status update (needed for every second print function
{

  valves = 0;
  valves += digitalRead(TRAP_1_LIFT) << 0;
  valves += digitalRead(TRAP_1_HEAT) << 1;
  valves += digitalRead(TRAP_2_LIFT) << 2;
  valves += digitalRead(TRAP_2_HEAT) << 3;
  valves += digitalRead(Relay_0) << 4;
  valves += digitalRead(Relay_1) << 5;
  valves += digitalRead(Relay_2) << 6;
  valves += digitalRead(Relay_3) << 7;
  valves += digitalRead(Relay_4) << 8;
  valves += digitalRead(Relay_5) << 9;
  valves += digitalRead(Relay_6) << 10;
  valves += digitalRead(Relay_7) << 11;
  valves += digitalRead(Relay_8) << 12;
  valves += digitalRead(Relay_9) << 13;
  valves += digitalRead(Relay_A) << 14;
  valves += digitalRead(Relay_B) << 15;
}
//---------------------------------- Update mode and valve state --------------------------------------------
void Update(void)
{
  if (mode != mode_req)
  {

    mode = mode_req;
    submode = 0;
  };

  if (valves != valves_req)
  {
    digitalWrite(TRAP_1_LIFT, (valves_req & 0b0000000000000001));
    digitalWrite(TRAP_1_HEAT, (valves_req & 0b0000000000000010) >> 1);
    digitalWrite(TRAP_2_LIFT, (valves_req & 0b0000000000000100) >> 2);
    digitalWrite(TRAP_2_HEAT, (valves_req & 0b0000000000001000) >> 3);
    digitalWrite(Relay_0, (valves_req & 0b0000000000010000) >> 4);
    digitalWrite(Relay_1, (valves_req & 0b0000000000100000) >> 5);
    digitalWrite(Relay_2, (valves_req & 0b0000000001000000) >> 6);
    digitalWrite(Relay_3, (valves_req & 0b0000000010000000) >> 7);
    digitalWrite(Relay_4, (valves_req & 0b0000000100000000) >> 8);
    digitalWrite(Relay_5, (valves_req & 0b0000001000000000) >> 9);
    digitalWrite(Relay_6, (valves_req & 0b0000010000000000) >> 10);
    digitalWrite(Relay_7, (valves_req & 0b0000100000000000) >> 11);
    digitalWrite(Relay_8, (valves_req & 0b0001000000000000) >> 12);
    digitalWrite(Relay_9, (valves_req & 0b0010000000000000) >> 13);
    digitalWrite(Relay_A, (valves_req & 0b0100000000000000) >> 14);
    digitalWrite(Relay_B, (valves_req & 0b1000000000000000) >> 15);

    getValve();
    valves_req = valves;
  };

  update = false;
}

//---------------------------------- Start CO2 counter --------------------------------------------
void StartCO2 (void)
{
  CO2buffer2Sel = false;
  CO2buffer1Ready = false;
  CO2buffer2Ready = false;
  CO2bufferIndex = 0;
  //Timer6.attachInterrupt(getCO2).setFrequency(CO2Freq).start();
  Timer6.start();
  ;
}

//---------------------------------- get CO2 data (timer interrupt) --------------------------------------------
void getCO2 (void)
{

  if (CO2buffer2Sel)
  {
    CO2buffer2[CO2bufferIndex] = analogRead(A_CO2_in_1);
  } else
  {
    CO2buffer1[CO2bufferIndex] = analogRead(A_CO2_in_1);
  }

  CO2bufferIndex++;

  if (CO2bufferIndex > 99)
  {
    if (CO2buffer2Sel)
    {
      CO2buffer2Ready = true;
      CO2buffer2Sel = false;
      CO2bufferIndex = 0;
    } else
    {
      CO2buffer1Ready = true;
      CO2buffer2Sel = true;
      CO2bufferIndex = 0;
    };

    if ((CO2buffer2Ready) && (CO2buffer1Ready))
    {
      //Serial.println("CO2 buffers full, data will be lost");
    };
  }  // end of  if (CO2bufferIndex > 99)
}  // End of getCO2

//---------------------------------- print CO2 data when buffer is full --------------------------------------------

void printCO2 (void)
{
  byte j = 0;

  if (CO2buffer1Ready)
  {
    Serial.print("CO2_0: ");
    for (j = 0; j < 100; j++)
    {
      if ((j == 20) || (j == 40) || (j == 60) || (j == 80))
      {
        Serial.println("");
        Serial.print("CO2_");
        Serial.print(j / 20);
        Serial.print(": ");
        // Serial.print((String) "CO2" + (j/20) + ": ");
      };
      Serial.print(CO2buffer1[j], HEX);
      Serial.print(";");
    }
    Serial.println("");
    CO2buffer1Ready = false;
  };

  if (CO2buffer2Ready)
  {
    Serial.print("CO2_0: ");
    for (j = 0; j < 100; j++)
    {
      if ((j == 20) || (j == 40) || (j == 60) || (j == 80))
      {
        Serial.println("");
        Serial.print("CO2_");
        Serial.print(j / 20);
        Serial.print(": ");
        // Serial.print((String) "CO2" + (j/20) + ": ");
      };
      Serial.print(CO2buffer2[j], HEX);
      Serial.print(";");
    }
    Serial.println("");
    CO2buffer2Ready = false;
  };


}

void StopCO2 (void)
{
  Timer6.stop();
}

//---------------------------------- Reset C14 counters --------------------------------------------

void ResetC14 (void)
{
  //  digitalWrite(C14_skait_Stop, HIGH);
  shittyDelay(100);
  //  digitalWrite(C14_skait_Reset1, HIGH);
  shittyDelay(100);
  //  digitalWrite(C14_skait_Reset2, HIGH);
  shittyDelay(100);
  //  digitalWrite(C14_skait_Reset1, LOW);
  shittyDelay(100);
  //  digitalWrite(C14_skait_Reset2, LOW);
  shittyDelay(100);
  //  digitalWrite(C14_skait_Stop, LOW);
  shittyDelay(100);
  ;
}

//---------------------------------- Get C14 counter values to C14_1 and C14_2 variables --------------------------------------------

void GetC14 (void)
{

  C14_1 = 0;
  C14_2 = 0;
  C14_3 = 0;
  C14_4 = 0;

  // Copy counter data to buffers
  digitalWrite(C14_skait_WR, HIGH); // Select 2 counter
  shittyDelay(1);
  digitalWrite(C14_skait_WR, LOW);



  digitalWrite(C14_skait_A, LOW); // Select 1 counter
  digitalWrite(C14_skait_B, LOW);
  digitalWrite(C14_skait_C, LOW);
  shittyDelay(1);                 // Shitty delay made of digitalRead (0) to dev/null
  digitalWrite(C14_skait_ADR, HIGH);    // Enable 74LS138P output (counter select)
  shittyDelay(1);


  C14_1 += digitalRead(C14_skait_0) << 0;
  C14_1 += digitalRead(C14_skait_1) << 1;
  C14_1 += digitalRead(C14_skait_2) << 2;
  C14_1 += digitalRead(C14_skait_3) << 3;
  C14_1 += digitalRead(C14_skait_4) << 4;
  C14_1 += digitalRead(C14_skait_5) << 5;
  C14_1 += digitalRead(C14_skait_6) << 6;
  C14_1 += digitalRead(C14_skait_7) << 7;

  digitalWrite(C14_skait_ADR, LOW);     // Disable 74LS138P output (counter select)

  digitalWrite(C14_skait_A, HIGH); // Select 2 counter
  digitalWrite(C14_skait_B, LOW);
  digitalWrite(C14_skait_C, LOW);
  shittyDelay(1);                 // Shitty delay made of digitalRead (0) to dev/null
  digitalWrite(C14_skait_ADR, HIGH);    // Enable 74LS138P output (counter select)
  shittyDelay(1);



  C14_1 += digitalRead(C14_skait_0) << 8;
  C14_1 += digitalRead(C14_skait_1) << 9;
  C14_1 += digitalRead(C14_skait_2) << 10;
  C14_1 += digitalRead(C14_skait_3) << 11;
  C14_1 += digitalRead(C14_skait_4) << 12;
  C14_1 += digitalRead(C14_skait_5) << 13;
  C14_1 += digitalRead(C14_skait_6) << 14;
  C14_1 += digitalRead(C14_skait_7) << 15;

  digitalWrite(C14_skait_ADR, LOW);     // Disable 74LS138P output (counter select)

  digitalWrite(C14_skait_A, LOW); // Select 3 counter
  digitalWrite(C14_skait_B, HIGH);
  digitalWrite(C14_skait_C, LOW);
  shittyDelay(1);                 // Shitty delay made of digitalRead (0) to dev/null
  digitalWrite(C14_skait_ADR, HIGH);    // Enable 74LS138P output (counter select)
  shittyDelay(1);

  C14_2 += digitalRead(C14_skait_0) << 0;
  C14_2 += digitalRead(C14_skait_1) << 1;
  C14_2 += digitalRead(C14_skait_2) << 2;
  C14_2 += digitalRead(C14_skait_3) << 3;
  C14_2 += digitalRead(C14_skait_4) << 4;
  C14_2 += digitalRead(C14_skait_5) << 5;
  C14_2 += digitalRead(C14_skait_6) << 6;
  C14_2 += digitalRead(C14_skait_7) << 7;

  digitalWrite(C14_skait_ADR, LOW);     // Disable 74LS138P output (counter select)

  digitalWrite(C14_skait_A, HIGH); // Select 4 counter
  digitalWrite(C14_skait_B, HIGH);
  digitalWrite(C14_skait_C, LOW);
  shittyDelay(1);                 // Shitty delay made of digitalRead (0) to dev/null
  digitalWrite(C14_skait_ADR, HIGH);    // Enable 74LS138P output (counter select)
  shittyDelay(1);

  C14_2 += digitalRead(C14_skait_0) << 8;
  C14_2 += digitalRead(C14_skait_1) << 9;
  C14_2 += digitalRead(C14_skait_2) << 10;
  C14_2 += digitalRead(C14_skait_3) << 11;
  C14_2 += digitalRead(C14_skait_4) << 12;
  C14_2 += digitalRead(C14_skait_5) << 13;
  C14_2 += digitalRead(C14_skait_6) << 14;
  C14_2 += digitalRead(C14_skait_7) << 15;

  digitalWrite(C14_skait_ADR, LOW);     // Disable 74LS138P output (counter select)

  digitalWrite(C14_skait_A, LOW); // Select 5 counter
  digitalWrite(C14_skait_B, LOW);
  digitalWrite(C14_skait_C, HIGH);
  shittyDelay(1);                 // Shitty delay made of digitalRead (0) to dev/null
  digitalWrite(C14_skait_ADR, HIGH);    // Enable 74LS138P output (counter select)
  shittyDelay(1);

  C14_3 += digitalRead(C14_skait_0) << 0;
  C14_3 += digitalRead(C14_skait_1) << 1;
  C14_3 += digitalRead(C14_skait_2) << 2;
  C14_3 += digitalRead(C14_skait_3) << 3;
  C14_3 += digitalRead(C14_skait_4) << 4;
  C14_3 += digitalRead(C14_skait_5) << 5;
  C14_3 += digitalRead(C14_skait_6) << 6;
  C14_3 += digitalRead(C14_skait_7) << 7;


  digitalWrite(C14_skait_ADR, LOW);     // Disable 74LS138P output (counter select)

  digitalWrite(C14_skait_A, HIGH); // Select 6 counter
  digitalWrite(C14_skait_B, LOW);
  digitalWrite(C14_skait_C, HIGH);
  shittyDelay(1);                 // Shitty delay made of digitalRead (0) to dev/null
  digitalWrite(C14_skait_ADR, HIGH);    // Enable 74LS138P output (counter select)
  shittyDelay(1);

  C14_4 += digitalRead(C14_skait_0) << 0;
  C14_4 += digitalRead(C14_skait_1) << 1;
  C14_4 += digitalRead(C14_skait_2) << 2;
  C14_4 += digitalRead(C14_skait_3) << 3;
  C14_4 += digitalRead(C14_skait_4) << 4;
  C14_4 += digitalRead(C14_skait_5) << 5;
  C14_4 += digitalRead(C14_skait_6) << 6;
  C14_4 += digitalRead(C14_skait_7) << 7;

  digitalWrite(C14_skait_ADR, LOW);     // Disable 74LS138P output (counter select)



}

//---------------------------------- Shitty delay --------------------------------------------

void shittyDelay (unsigned int us)
{
  for (us; us > 0; us--)

  {
    digitalRead(C14_skait_0);
  }

}
