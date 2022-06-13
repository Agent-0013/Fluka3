#include <DueTimer.h>



#define FWver 190326

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
const byte C14_skait_WR = 31;
const byte C14_skait_Stop = 30;
const byte C14_skait_Reset1 = 32;
const byte C14_skait_Reset2 = 33;
const byte C14_1_skait_EN = 34;
const byte C14_2_skait_EN = 35;
const byte C14_3_skait_EN = 36;
const byte C14_4_skait_EN = 37;

//const byte C14_skait_Reset = 43;
const byte Relay_0 = 44;
const byte Relay_1 = 45;
const byte Relay_2 = 46;
const byte Relay_3 = 47;
const byte Relay_4 = 48;
const byte Relay_5 = 49;
const byte Relay_6 = 50;
const byte Relay_7 = 51;

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

bool update = false;

unsigned int C14_1 = 0;
unsigned int C14_2 = 0;

//************************************ Delay stuff config ************************************
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
unsigned long SampleTime = 60000;  // set to length of first interval.
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
  Serial.println(" ;END");
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

  analogReadResolution(12);   // Set analog read resolution

  SYSinit();   // set default control and system states (to be used later)




  //************************************ Let the show begin ************************************

  Timer3.attachInterrupt(GETinfo).setFrequency(1).start();
  //Timer3.stop();

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
  getValve();
  valves_req = valves;

}
//---------------------------------- Getinfo function --------------------------------------------
void GETinfo(void)
{
  GetC14();
  Serial.print("DATA: ");
  Serial.print("T1=");
  Serial.print(analogRead(A_Temp_1));
  Serial.print(";T2=");
  Serial.print(analogRead(A_Temp_2));
  Serial.print(";T3=");
  Serial.print(analogRead(A_Temp_3));
  Serial.print(";P1=");
  Serial.print(analogRead(A_Pres_1));
  Serial.print(";P2=");
  Serial.print(analogRead(A_Pres_2));
  Serial.print(";F1=");
  Serial.print(analogRead(A_Flow_1));
  Serial.print(";F2=");
  Serial.print(analogRead(A_Flow_2));
  Serial.print(";HV=");
  Serial.print(analogRead(A_High_voltage));
  Serial.print(";C1=");
  Serial.print(C14_1);
  Serial.print(";C2=");
  Serial.print(C14_2);
  Serial.print(";M0=");
  Serial.print(mode);
  Serial.print(";M1=");
  Serial.print(submode);
  Serial.print(";VV=");
  Serial.print(valves, BIN);
  Serial.print(";ER=");
  Serial.print(ErrorCode);
  Serial.println(";END");
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
    getValve();
    valves_req = valves;
  };

  update = false;
}

//---------------------------------- Start CO2 counter --------------------------------------------
void StartCO2 (void)
{
  ;
}

void ResetC14 (void)
{
  digitalWrite(C14_skait_Stop, HIGH);
  digitalWrite(C14_skait_Reset1, HIGH);
  digitalWrite(C14_skait_Reset2, HIGH);
  digitalWrite(C14_skait_Reset1, LOW);
  digitalWrite(C14_skait_Reset2, LOW);
  digitalWrite(C14_skait_Stop, LOW);
  ;
}

void GetC14 (void)
{
  digitalWrite(C14_skait_WR, HIGH); // Copy counter data to buffers
  unsigned long j = millis();
  unsigned long k = j;
  //while ((k - j) < 2) {k = millis();};  // Fancy 2ms delay for counters to copy data to internal registers
  digitalWrite(C14_skait_WR, LOW);

  digitalWrite(C14_1_skait_EN, HIGH); // Enable 1 counter
  j = millis();
  k = j;
  //while ((k - j) < 2) {k = millis();};  // another Fancy 1ms delay
  C14_1 = digitalRead(C14_skait_0) * 128 + digitalRead(C14_skait_1) * 64 + digitalRead(C14_skait_2) * 32 + digitalRead(C14_skait_3) * 16 + digitalRead(C14_skait_4) * 8 + digitalRead(C14_skait_5) * 4 + digitalRead(C14_skait_6) * 2 + digitalRead(C14_skait_7) * 1;
  digitalWrite(C14_1_skait_EN, LOW); // Disable 1 counter
  
  digitalWrite(C14_2_skait_EN, HIGH); // Enable 2 counter
  j = millis();
  k = j;
  //while ((k - j) < 2) {k = millis();};  // another Fancy 1ms delay
  C14_1 = C14_1 + digitalRead(C14_skait_0) * 32768 + digitalRead(C14_skait_1) * 16384 + digitalRead(C14_skait_2) * 8192 + digitalRead(C14_skait_3) * 4096 + digitalRead(C14_skait_4) * 2048 + digitalRead(C14_skait_5) * 1024 + digitalRead(C14_skait_6) * 512 + digitalRead(C14_skait_7) * 256;
  digitalWrite(C14_2_skait_EN, LOW); // Disable 2 counter
  
  digitalWrite(C14_3_skait_EN, HIGH); // Enable 3 counter
  j = millis();
  k = j;
  //while ((k - j) < 2) {k = millis();};  // another Fancy 1ms delay
  C14_2 = digitalRead(C14_skait_0) * 128 + digitalRead(C14_skait_1) * 64 + digitalRead(C14_skait_2) * 32 + digitalRead(C14_skait_3) * 16 + digitalRead(C14_skait_4) * 8 + digitalRead(C14_skait_5) * 4 + digitalRead(C14_skait_6) * 2 + digitalRead(C14_skait_7) * 1;
  digitalWrite(C14_3_skait_EN, LOW); // Disable 3 counter
  
  digitalWrite(C14_4_skait_EN, HIGH); // Enable 4 counter
  j = millis();
  k = j;
  //while ((k - j) < 2) {k = millis();};  // another Fancy 1ms delay
  C14_2 = C14_1 + digitalRead(C14_skait_0) * 32768 + digitalRead(C14_skait_1) * 16384 + digitalRead(C14_skait_2) * 8192 + digitalRead(C14_skait_3) * 4096 + digitalRead(C14_skait_4) * 2048 + digitalRead(C14_skait_5) * 1024 + digitalRead(C14_skait_6) * 512 + digitalRead(C14_skait_7) * 256;
  digitalWrite(C14_4_skait_EN, LOW); // Disable 4 counter

}
