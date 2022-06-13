#include <DueTimer.h>



#define FWver 190503

//************************************ Pin config config ************************************
const byte PWR_OK = 2;
const byte CO2_LED_1 = 6;
const byte CO2_LED_2 = 7;
const byte TRAP_1_LIFT = 14;
const byte TRAP_1_HEAT = 15;
const byte TRAP_2_LIFT = 16;
const byte TRAP_2_HEAT = 17;
const byte C14_skait_0 = 23;
const byte C14_skait_1 = 22;
const byte C14_skait_2 = 25;
const byte C14_skait_3 = 24;
const byte C14_skait_4 = 27;
const byte C14_skait_5 = 26;
const byte C14_skait_6 = 29;
const byte C14_skait_7 = 28;
const byte C14_skait_WR = 31;
const byte C14_skait_Stop = 30;
const byte C14_skait_Reset1 = 32;
const byte C14_skait_Reset2 = 33;
const byte C14_1_skait_EN = 35;
const byte C14_2_skait_EN = 34;
const byte C14_3_skait_EN = 37;
const byte C14_4_skait_EN = 36;

//const byte C14_skait_Reset = 43;
const byte Relay_0 = 44;
const byte Relay_1 = 45;
const byte Relay_2 = 46;
const byte Relay_3 = 47;
const byte Relay_4 = 48;
const byte Relay_5 = 49;
const byte Relay_6 = 50;
const byte Relay_7 = 51;
const byte Relay_8 = 40;
const byte Relay_9 = 41;
const byte Relay_A = 42;
const byte Relay_B = 43;


const int A_Temp_1 = A0;
const int A_Temp_2 = A1;
const int A_Temp_3 = A2;
const int A_Pres_1 = A3;
const int A_Pres_2 = A4;
const int A_CO2_in_1 = A5;
const int A_CO2_in_2 = A6;
const int A_Flow_1 = A7;
const int A_Flow_2 = A8;
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

unsigned int CO2buffer1[100] = {0};       // Buffers and stuff for CO2 measurements
unsigned int CO2buffer2[100] = {0};
byte CO2bufferIndex = 0;
bool CO2buffer2Sel = false;
bool CO2buffer1Ready = false;
bool CO2buffer2Ready = false;




//************************************ Delay stuff config ************************************

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
unsigned long SampleTime = 5000;  // set to length of first interval.
unsigned long HeatTime = 1500;     // heat for 1500ms


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

  pinMode(C14_1_skait_EN, OUTPUT);   // C14_1_skait_EN
  pinMode(C14_2_skait_EN, OUTPUT);   // C14_2_skait_EN
  pinMode(C14_3_skait_EN, OUTPUT);   // C14_3_skait_EN
  pinMode(C14_4_skait_EN, OUTPUT);   // C14_4_skait_EN
  pinMode(C14_skait_WR, OUTPUT);   // C14_skait_WR
  pinMode(C14_skait_Stop, OUTPUT);   // C14_skait_WR
  pinMode(C14_skait_Reset1, OUTPUT);   // C14_skait_Reset
  pinMode(C14_skait_Reset2, OUTPUT);   // C14_skait_Reset

  pinMode(Relay_0, OUTPUT);   // Relay_0
  pinMode(Relay_1, OUTPUT);   // Relay_1
  pinMode(Relay_2, OUTPUT);   // Relay_2
  pinMode(Relay_3, OUTPUT);   // Relay_3
  pinMode(Relay_4, OUTPUT);   // Relay_4
  pinMode(Relay_5, OUTPUT);   // Relay_5
  pinMode(Relay_6, OUTPUT);   // Relay_6
  pinMode(Relay_7, OUTPUT);   // Relay_7
  pinMode(Relay_8, OUTPUT);   // Relay_4
  pinMode(Relay_9, OUTPUT);   // Relay_5
  pinMode(Relay_A, OUTPUT);   // Relay_6
  pinMode(Relay_B, OUTPUT);   // Relay_7

  analogReadResolution(12);   // Set analog read resolution


  //************************************ Configure CO2 stuff ************************************
  //Timer6.attachInterrupt(getCO2).setFrequency(100);
  //Serial.print("Available timer: ");
  //Serial.println(Timer.getAvailable());


  SYSinit();   // set default control and system states (to be used later)




  //************************************ Let the show begin ************************************

  Timer3.attachInterrupt(GETinfo).setFrequency(dataFreq).start();
  //Timer3.stop();


  StartCO2();               // FOR CO2 DEBUG!!!
}   // End of void setup()



//---------------------------------- void loop () --------------------------------------------

void loop()
{

  //unsigned long currentMillis = millis();   // For everything to work correctly

  //if (currentMillis - previousMillis >= interval)


  switch (mode)
  {
    //************************************ Standby MODE ************************************
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
    //************************************ Sampling MODE  ************************************
    case 1:   // Sample aquire started
      {
        switch (submode)
        {
          case 0:   // Start sample pump
            {
              previousMillis = millis();
              setValve(Relay_5, 0);       // Start sample pump relay_5
              setValve(TRAP_1_HEAT, 1);   // Start trap 1 preheat
              submode++;
              break;
            }
          case 1:   // Preheat and lift 1 N2 cylinder
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= 1000)  // Delay 1000ms for trap to preheat, and burn everything out
              {
                setValve(TRAP_1_HEAT, 0);   // Turn OFF trap 1 heating
                setValve(TRAP_2_HEAT, 1);   // Turn ON trap 2 heating
                previousMillis = millis();  // !!!!!!!!!CRYTICAL!!!!!!!!!!! DO NOT RESET PREVIOUS MILLIS AFTER THAT, AS IT WILL AFFECT SAMPLING TIME!!!
                setValve(TRAP_1_LIFT, 1);   // Lift lift 1 N2 cylinder to start sampling
                submode++;
              };
              break;
            }

          case 2:   // Wait for cylinder to get cold and turn on sample gas
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= 1000)  // Delay 1s for trap 2 to preheat, and lift it up
              {
                setValve(TRAP_2_HEAT, 0);   // Turn OFF trap 2 heating
                setValve(TRAP_2_LIFT, 1);   // Lift lift 2 N2 cylinder to start sampling
                submode++;
              };
              break;
            }
          case 3: // Wait for sample time
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= SampleTime)  // Delay SampleTime seconds for sample to aquire
              {
                submode++;
                setValve(Relay_0, 0);   // Realy_0 = valve A ON (Open carrier gas)
                setValve(Relay_1, 0);   // Realy_1 = valve B ON (Open loop to CO2 measurement and 2 TRAP)
                setValve(Relay_5, 1);   // Realy_5 = Stop sampling pump
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

    //************************************ Heat and measure CO2 mode ************************************

    case 2:   // Release and Measure CO2
      {
        switch (submode)
        {
          case 0:   // Configure valves for heating
            {
              setValve(Relay_0, 0);   // Realy_0 = valve A ON (Open carrier gas)      // Should be open already, just in case started from this mode
              setValve(Relay_1, 0);   // Realy_1 = valve B ON (Open loop to CO2 measurement and 2 TRAP)
              previousMillis = millis();
              setValve(TRAP_1_LIFT, 0);
              submode++;
              break;
            }
          case 1:   // Wait for cylinder to get down
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= 500)  // Delay 500ms for trap to get down
              {
                previousMillis = millis();
                setValve(TRAP_1_HEAT, 1); // Turn ON trap 1 heating
                StartCO2();               // Start CO2 counter
                submode++;
              };
              break;
            }

          case 2:
            {
              currentMillis = millis();
              if (currentMillis - previousMillis >= HeatTime)  // Delay HeatTime seconts for trap 1 to heat
              {
                setValve(TRAP_1_HEAT, 0);       // Turn OFF trap 1 heating
                submode++;
              }
              break;
            }
          case 3:
            {
              mode_req = 3;
              update = true;
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

      }     // End od case mode = 1

    case 3:
      {
        switch (submode)
        {
          case 0:   // Initalize the system to default state
            {
              SYSinit();
              ResetC14();
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

    case 255:
      {
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

  }       // End of if TimeForData

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
  digitalWrite(C14_1_skait_EN, LOW);
  digitalWrite(C14_2_skait_EN, LOW);
  digitalWrite(C14_3_skait_EN, LOW);
  digitalWrite(C14_4_skait_EN, LOW);
  digitalWrite(C14_skait_WR, LOW);
  digitalWrite(C14_skait_Stop, LOW);
  digitalWrite(C14_skait_Reset1, LOW);
  digitalWrite(C14_skait_Reset2, LOW);
  digitalWrite(Relay_0, HIGH);
  digitalWrite(Relay_1, HIGH);
  digitalWrite(Relay_2, HIGH);
  digitalWrite(Relay_3, HIGH);
  digitalWrite(Relay_4, HIGH);
  digitalWrite(Relay_5, HIGH);
  digitalWrite(Relay_6, HIGH);
  digitalWrite(Relay_7, HIGH);
  digitalWrite(Relay_8, HIGH);
  digitalWrite(Relay_9, HIGH);
  digitalWrite(Relay_A, HIGH);
  digitalWrite(Relay_B, HIGH);
  getValve();
  valves_req = valves;
  ResetC14();

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
      if ((tmpval < 65536) && (tmpval > 0) && (mode == 255)) // Check if command is OK
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
    else if (strcmp(parsedString, "STIME") == 0)
    {
      strtokIndx = strtok(NULL, ";"); // this continues where the previous call left off, find digit after = until ;
      tmpval = atoi(strtokIndx);      // Convert it to integer
      if ((tmpval < 600) && (tmpval > 1)) // Check if command is OK
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
    digitalWrite(Relay_8, (valves_req & 0b0000100000000000) >> 12);
    digitalWrite(Relay_9, (valves_req & 0b0000100000000000) >> 13);
    digitalWrite(Relay_A, (valves_req & 0b0000100000000000) >> 14);
    digitalWrite(Relay_B, (valves_req & 0b0000100000000000) >> 15);
    
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
  Timer6.attachInterrupt(getCO2).setFrequency(CO2Freq).start();
  //Timer6.start();
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
    Serial.print("CO20: ");
    for (j = 0; j < 100; j++)
    {
      if ((j == 20) || (j == 40) || (j == 60) || (j == 80))
      {
        Serial.println("");
        Serial.print("CO2");
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
    Serial.print("CO20: ");
    for (j = 0; j < 100; j++)
    {
      if ((j == 20) || (j == 40) || (j == 60) || (j == 80))
      {
        Serial.println("");
        Serial.print("CO2");
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

//---------------------------------- Reset C14 counters --------------------------------------------

void ResetC14 (void)
{
  digitalWrite(C14_skait_Stop, HIGH);
  shittyDelay(100);
  digitalWrite(C14_skait_Reset1, HIGH);
  shittyDelay(100);
  digitalWrite(C14_skait_Reset2, HIGH);
  shittyDelay(100);
  digitalWrite(C14_skait_Reset1, LOW);
  shittyDelay(100);
  digitalWrite(C14_skait_Reset2, LOW);
  shittyDelay(100);
  digitalWrite(C14_skait_Stop, LOW);
  shittyDelay(100);
  ;
}

//---------------------------------- Get C14 counter values to C14_1 and C14_2 variables --------------------------------------------

void GetC14 (void)
{

  C14_1 = 0;
  C14_2 = 0;

  digitalWrite(C14_skait_WR, HIGH); // Copy counter data to buffers
  shittyDelay(100);                 // Shitty delay made of digitalRead (0) to dev/null
  digitalWrite(C14_skait_WR, LOW);
  shittyDelay(100);
  digitalWrite(C14_1_skait_EN, HIGH); // Enable 1 counter
  shittyDelay(100);

  C14_1 += digitalRead(C14_skait_0) << 0;
  C14_1 += digitalRead(C14_skait_1) << 1;
  C14_1 += digitalRead(C14_skait_2) << 2;
  C14_1 += digitalRead(C14_skait_3) << 3;
  C14_1 += digitalRead(C14_skait_4) << 4;
  C14_1 += digitalRead(C14_skait_5) << 5;
  C14_1 += digitalRead(C14_skait_6) << 6;
  C14_1 += digitalRead(C14_skait_7) << 7;



  digitalWrite(C14_1_skait_EN, LOW); // Disable 1 counter
  shittyDelay(100);
  digitalWrite(C14_2_skait_EN, HIGH); // Enable 2 counter
  shittyDelay(100);

  C14_1 += digitalRead(C14_skait_0) << 8;
  C14_1 += digitalRead(C14_skait_1) << 9;
  C14_1 += digitalRead(C14_skait_2) << 10;
  C14_1 += digitalRead(C14_skait_3) << 11;
  C14_1 += digitalRead(C14_skait_4) << 12;
  C14_1 += digitalRead(C14_skait_5) << 13;
  C14_1 += digitalRead(C14_skait_6) << 14;
  C14_1 += digitalRead(C14_skait_7) << 15;


  digitalWrite(C14_2_skait_EN, LOW); // Disable 2 counter
  shittyDelay(100);
  digitalWrite(C14_3_skait_EN, HIGH); // Enable 3 counter
  shittyDelay(100);

  C14_2 += digitalRead(C14_skait_0) << 0;
  C14_2 += digitalRead(C14_skait_1) << 1;
  C14_2 += digitalRead(C14_skait_2) << 2;
  C14_2 += digitalRead(C14_skait_3) << 3;
  C14_2 += digitalRead(C14_skait_4) << 4;
  C14_2 += digitalRead(C14_skait_5) << 5;
  C14_2 += digitalRead(C14_skait_6) << 6;
  C14_2 += digitalRead(C14_skait_7) << 7;

  digitalWrite(C14_3_skait_EN, LOW); // Disable 3 counter
  shittyDelay(100);
  digitalWrite(C14_4_skait_EN, HIGH); // Enable 4 counter
  shittyDelay(100);

  C14_2 += digitalRead(C14_skait_0) << 8;
  C14_2 += digitalRead(C14_skait_1) << 9;
  C14_2 += digitalRead(C14_skait_2) << 10;
  C14_2 += digitalRead(C14_skait_3) << 11;
  C14_2 += digitalRead(C14_skait_4) << 12;
  C14_2 += digitalRead(C14_skait_5) << 13;
  C14_2 += digitalRead(C14_skait_6) << 14;
  C14_2 += digitalRead(C14_skait_7) << 15;

  digitalWrite(C14_4_skait_EN, LOW); // Disable 4 counter

}

//---------------------------------- Shitty delay --------------------------------------------

void shittyDelay (unsigned int us)
{
  for (us; us > 0; us--)

  {
    digitalRead(C14_skait_0);
  }

}
