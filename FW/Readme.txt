Ver. 190303
UART communication works

Command interface:
<[COMMAND1]=[Value1];[COMMAND2]=[Value2];...>

Supported commands:
SETMODE=0...255			// Set working mode
COMMAND=0...65535		// Set valve directly (works only in mode=255)

External valve control works
Data output every second works
ADCs reading works, but without analysis

----------------------------------------------------
Ver. 190304

Fixed mode 0 init bug
Fixed valves showing incorrectly after init bug
Mode workflow tested. Delay in millis tested and working

----------------------------------------------------
Ver. 190305

New command STIME=1..600 - Set sample time in seconds. Default 60 seconds. Must be set before sampling is started.
Fixed bug when after system init valves would restore previous state after valve change command arrived

----------------------------------------------------
Ver. 190305

Added reverse working logic for relays. Sarunas needs to know that when receiving serial data.

----------------------------------------------------
Ver. 190326

Added counter code (not working yet because of hardware problems)

----------------------------------------------------
Ver. 190401

Workaround for slow optopulator speed (shitty)
Counter kind of working, but bits reading incorrectly.

----------------------------------------------------
Ver. 190408

Less shitty workaround for slow optopulator
Counter finally works normally (pins are correct now for HW ver 1)

----------------------------------------------------
Ver. 190408
Added CO2 ADC read function
Reworked data aquisition and serial printing functions. Now timer interrupts only set bit and data index, and print happens inside void loop (fixed random crashes when 2 interrupts happen at the same time)
CO2 have 2 buffers x 100 values
CO2 sampling frequency and data polling frequency now set by global constants.
Removed END; after each data packet

----------------------------------------------------
Ver. 190503
Added valves 8 to B, untested yet

----------------------------------------------------
Ver. 190516
All valves tested and working
Rewriten firmware for HW ver 2 (4 counters)
All counters are working normally, and quite fast.

----------------------------------------------------
Ver. 190517
Added sequence V1

!!!!! Need to add C14Time, FlushTime, CoolTime and HeatTime to PC commands
!!!! Need to do ABORT command
!!!! Need to program counter reset sequences

----------------------------------------------------
Ver. 190522
Added big red button for manual start/standby mode change
Some minor bug fixes

!!!!! Need to add C14Time, FlushTime, CoolTime and HeatTime to PC commands
!!!! Need to do ABORT command

----------------------------------------------------
Ver. 190528
Added C14Time, FlushTime, CoolTime and HeatTime to PC commands
BTIME = 1...600s; FTIME = 1..60s; CTIME = 1..600s; HTIME = 500..2000ms; 

----------------------------------------------------
Ver. 190529
Minor code changes

----------------------------------------------------
Ver. 190606
Fixed bug where valves could not be all on at the same time

----------------------------------------------------
Ver. 190704
Added trap heat failsafe (also acts as a trap heat turn off switch when in manual mode)

----------------------------------------------------
Ver. 190709
Fixed errors in trap heat failsafe

----------------------------------------------------
Ver. 190717
Temporary reduced harcoded flust times
Big red button starts sequence

----------------------------------------------------
Ver. 190801
Made vacuum work
Valve 9 is now working

----------------------------------------------------
Ver. 190910
MOdified sequence.
Now we have vacuum in mode 2.

----------------------------------------------------
Ver. 190912
Modified sequence.
Vacuum only on sampling state.
Increased CO2 measurement time to 90 seconds. 

----------------------------------------------------
Ver. 190917
Added CO2 parameter external setting.
Set default CO2 measurement parameter to 5 minutes

----------------------------------------------------
Ver. 191002
Sequence change:
Add CO2 to manual mode
Sample now goes without vacuum

----------------------------------------------------
Ver. 200227
Added new CO2 trap preheat, CO2 release and CO2 flush commands.
PTEMP, RTEMP and FTEMP, values from 1 to 250 C
Removed trap heat failsafe for trap 1 because of using another type pf trap.
Added Zeolite() function which control all the shit of zeolite trap. Need to finish zeolite heat PWM output and temperature readings.
Zeolite heat on pin 6
NEED TO FINISH ZTEMP function

----------------------------------------------------
Ver. 200312
Added temperature sensor support
Added zeolyte heater control on pin 19
Added zeolyte fan control on pin 14 (instead of trap lift)
Fixed ResetC14 function
Button now resets C14 counter

