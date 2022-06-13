#include <DueTimer.h>



#define FWver 190303


const byte PWR_OK = 2;
const byte CO2_LED_1 = 6;
const byte CO2_LED_2 = 7;
const byte TRAP_1_LIFT = 14;
const byte TRAP_1_HEAT = 15;
const byte TRAP_2_LIFT = 16;
const byte TRAP_2_HEAT = 17;
const byte C14_skait_0 = 22;
const byte C14_skait_1 = 24;
const byte C14_skait_2 = 26;
const byte C14_skait_3 = 28;
const byte C14_skait_4 = 30;
const byte C14_skait_5 = 32;
const byte C14_skait_6 = 34;
const byte C14_skait_7 = 36;
const byte C14_1_skait_EN = 38;
const byte C14_2_skait_EN = 39;
const byte C14_3_skait_EN = 40;
const byte C14_4_skait_EN = 41;
const byte C14_skait_WR = 42;
const byte C14_skait_Reset = 43;
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


//************************************ Serial parser config************************************

char serialString[64] = {0};         // a String to hold incoming data
char tempChars[64] = {0};        // temporary array for use when parsing
// char parsedString[64] = {0};        // 2nd temporary array for use when parsing

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


  pinMode(2, INPUT_PULLUP);   // PWR_OK
  pinMode(6, OUTPUT);   // CO2_LED_1
  pinMode(7, OUTPUT);   // CO2_LED_2

  pinMode(14, OUTPUT);   // TRAP_1_LIFT
  pinMode(15, OUTPUT);   // TRAP_1_HEAT
  pinMode(16, OUTPUT);   // TRAP_2_LIFT
  pinMode(17, OUTPUT);   // TRAP_2_HEAT

  pinMode(22, INPUT_PULLUP);   // C14_skait_0
  pinMode(24, INPUT_PULLUP);   // C14_skait_1
  pinMode(26, INPUT_PULLUP);   // C14_skait_2
  pinMode(28, INPUT_PULLUP);   // C14_skait_3
  pinMode(30, INPUT_PULLUP);   // C14_skait_4
  pinMode(32, INPUT_PULLUP);   // C14_skait_5
  pinMode(34, INPUT_PULLUP);   // C14_skait_6
  pinMode(36, INPUT_PULLUP);   // C14_skait_7

  pinMode(38, OUTPUT);   // C14_1_skait_EN
  pinMode(39, OUTPUT);   // C14_2_skait_EN
  pinMode(40, OUTPUT);   // C14_3_skait_EN
  pinMode(41, OUTPUT);   // C14_4_skait_EN
  pinMode(42, OUTPUT);   // C14_skait_WR
  pinMode(43, OUTPUT);   // C14_skait_Reset

  pinMode(44, OUTPUT);   // Relay_0
  pinMode(45, OUTPUT);   // Relay_1
  pinMode(46, OUTPUT);   // Relay_2
  pinMode(47, OUTPUT);   // Relay_3
  pinMode(48, OUTPUT);   // Relay_4
  pinMode(49, OUTPUT);   // Relay_5
  pinMode(50, OUTPUT);   // Relay_6
  pinMode(51, OUTPUT);   // Relay_7



  analogReadResolution(12);   // Set analog read resolution

  SYSinit();   // set default control and system states (to be used later)

  //************************************ Let the show begin ************************************

  Timer3.attachInterrupt(GETinfo).setFrequency(1).start();
  //Timer3.stop();

}   // End of void setup()



//---------------------------------- void loop () --------------------------------------------

void loop()
{
  switch (mode)
  {
    case 0:
      {
        if (submode == 0)
        {
          setValve(TRAP_1_LIFT, 1);
          submode++;
        }
        break;
      }

    case 1:
      {
        if (submode == 0)
        {
          setValve(TRAP_1_LIFT, 0);
          submode++;
        }
        break;
      }

    case 2:
      {
        if (submode == 0)
        {
          setValve(TRAP_2_LIFT, 1);
          submode++;
        }
        break;
      }

    case 3:
      {
        if (submode == 0)
        {
          setValve(TRAP_2_LIFT, 0);
          submode++;
        }
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
  digitalWrite(C14_skait_Reset, LOW);
  digitalWrite(Relay_0, LOW);
  digitalWrite(Relay_1, LOW);
  digitalWrite(Relay_2, LOW);
  digitalWrite(Relay_3, LOW);
  digitalWrite(Relay_4, LOW);
  digitalWrite(Relay_5, LOW);
  digitalWrite(Relay_6, LOW);
  digitalWrite(Relay_7, LOW);

}
//---------------------------------- Getinfo function --------------------------------------------
void GETinfo(void)
{
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
  Serial.print(";VV=");
  Serial.print(valves, BIN);
  Serial.print(";M0=");
  Serial.print(mode);
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
      if ((tmpval < 65535) && (tmpval > 0) && (mode == 255)) // Check if command is OK
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
    digitalWrite(TRAP_1_LIFT, (valves_req&0b0000000000000001));
    digitalWrite(TRAP_1_HEAT, (valves_req&0b0000000000000010)>>1);
    digitalWrite(TRAP_2_LIFT, (valves_req&0b0000000000000100)>>2);
    digitalWrite(TRAP_2_HEAT, (valves_req&0b0000000000001000)>>3);
    digitalWrite(Relay_0, (valves_req&0b0000000000010000)>>4);
    digitalWrite(Relay_1, (valves_req&0b0000000000100000)>>5);
    digitalWrite(Relay_2, (valves_req&0b0000000001000000)>>6);
    digitalWrite(Relay_3, (valves_req&0b0000000010000000)>>7);
    digitalWrite(Relay_4, (valves_req&0b0000000100000000)>>8);
    digitalWrite(Relay_5, (valves_req&0b0000001000000000)>>9);
    digitalWrite(Relay_6, (valves_req&0b0000010000000000)>>10);
    digitalWrite(Relay_7, (valves_req&0b0000100000000000)>>11);
    getValve();
    valves_req = valves;
  };

  update = false;
}
