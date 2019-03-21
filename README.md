# alarmSys
Alarm System Description
This standalone hardware/firmware Alarm system consisting of embedded PIC18F4520 microchip, serial communication device, PIR motion sensor, temperature sensor, LEDs, resistor, capacitor, push button reset and 4x4 keypad connected together with wires in a bread board.
The alarm system can detect motion and trigger a temperature alarm. It incorporates a PIR motion sensor that is running throughout the program with a high priority interrupt running in the background. The Red LED turns on whenever a motion is detected. On the other hand, temperature sensor (TMP36) driven by timer interrupt measures the current temperature and displays it to the user and also triggers the alarm once the current temperature is equal to or greater than the provided threshold temperature. The Yellow LED toggles as it gets a new temperature reading and remains solid whenever the temperature alarm is triggered. It is driven by a low priority interrupt stored at the memory of the microchip.
FT232RL is used to create a bi-directional serial communication between the PIC and the PC. It also provides a common ground and also powers the PIC through the VCC output. A terminal on a PC can be used to interact with the output the message coming from the PIC and to communicate with it. External frequency of 20MHz is provided to the PIC through an external crystal oscillator. The Green LED turns on as system has started successfully and message coming from the PIC is displayed in the terminal of the PIC.
The alarm system also allows user to select a programmed 4x4 matrix keypad as the input method to the PIC. 

The source code for system was written using the MPLAB X IDE in the PC and compiled using XC8 compiler. 

EEPROM was used to store important states of the system.

Detailed Description of PIC Interrupts

PIC interrupts handle the alarm triggering process. The proper bits settings for the interrupts are executed in the program as soon as the user turns on the alarm function. The interrupts are disabled by default and user can also disable the interrupts using the menu function. The detection of motion is handled by using high priority interrupt function of the PIC. Whenever turned on, the high priority interrupt function constantly checks if the INT0 Interrupt Flag is set high due to the interrupt caused by the PIR sensor. If found high, the Red LED is turned ON and user is redirected to a function which prompts user to enter passcode and reset the alarm. The Interrupt Flag resets and the interrupt function checks for interrupts again.
Similarly, the low priority function handles the temperature alarm. The ADC is used for periodic sampling of the temperature reading. Once, the temperature alarm is enabled, the low priority function check for TMR0 flag is set high. If it is high, it stops TMR0 and gets the current temperature reading and starts the timer again. The Yellow LED toggles as it gets new temperature. Meanwhile, the low priority function also checks for ADC completion through ADIF flag bit. While set high, it compares the current temperature reading with the threshold temperature. If the current temperature is greater, it turns ON the Yellow LED and redirects the user to the enter passcode and reset the alarm. The function clears the ADIF flag and repeats the process again until turned OFF.
Detailed Description of Timer Modules
This Alarm system uses an external 20MHz Crystal Oscillator connected with the PIC. The proper configuration bits were selected for the oscillator type in order for the interrupt and serial communication to properly function. In the program, the timer settings are initialized as the program executes. The TMR0 Interrupt Flag is cleared initially and the Enable Bit is set high, the Interrupt Priority bit is set as low priority and the TMRL and TMRH bits are properly assigned with the calculated preload values. As the temperature alarm is enabled, the timer module carries out the ADC and displays the current temperature to the user. This interrupt is used to trigger the alarm.

 
temperature threshold alarm:

The timer settings are stored in the EEPROM of the pic to prevent form erasing after the MCLR Reset button is pressed.

Calculation for ADC and Timer values

The preload values for the timer was calculate as below:

Time period (d)

Oscillator Frequency (Fosc)

We have, X

= 1 sec

= 20 MHz

= d * Fosc

= 1 * (20/4) MHz

= 5 MHz

= 5000000 cycles per oscillator Using the prescalar 1:128 to fit it into 16-bit TMRO register (0 – 65536)

Therefore,
= 65536 – (5000000/128) = 65236-39063 = 26473 = 0x6769
TMR0H = 0x67
TMR0L = 0x69 

The ADC calculation was done as follows:

The result from the ADC is used and the value of the ADRESH register is taken and the address was shifted 8 bits left and the value of ADRESL was added to it and the result was 10 bits and the result was stored in a variable named “tempr’. Following calculation are performed to get the current temperature.
Firstly, it was converted into the range of 0 to 5V, subtract the device offset and convert it to degree centigrade,

Current temperature = (((tempr1023)* 5000)– 500 )/10

Temperature in F = ((temperature in degC) * 1.8) + 32

Description of Keypad’s Reading Method:

The 4x4 keypad is connected to the port D (from port RD0 – RD7). The first four ports are selected as output and the rest four port are selected as input. The first port represents the rows of buttons in the keypad and the remaining bit represents columns of the keypad buttons. A function that continuously checks for the key pressed is created that returns the character pressed using the keypad. The Blue LED is turned on when keypad is used as the input method.
The function works by setting each of the bits of the row’s button high and checking for the port of the column’s button whose bits are also high and then returning the respective character pressed.
For example, RD0, which represent the first rows of buttons (1, 2, 3 and A) is set high and the function checks for high bits in each column. Here, if a user pressed the button in the first column then the function returns ‘1’, for second column bits found high, it returns ‘2’. For the third column, it returns ‘3’ and for the fourth column it returns ‘A’. Similarly, the second, third and fourth rows are now set high one after another and checked for column bits that are high and then return the respective character. While the function checks for column bits high, it also toggles the blue LED to denote that key can be pressed again in the keypad.
The current state of the input used is stored in the EEPROM of the PIC so that the data is not lost after the MCLR Reset.


 
