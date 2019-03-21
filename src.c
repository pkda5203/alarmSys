 /*******************************
 * Author: Prabesh Khadka
 * Building a PIC18F4520 Standalone Alarm System with EUSART Communication
 ********************************/




#include <p18f4520.h>
#include <xc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <eeprom_routines.h>


// Defining _XTAL_FREQ and selecting the XC8 compiler allows us to use the delay functions __delay_ms(x) and __delay_us(x)
#define _XTAL_FREQ 20000000     //For using the External Crystal of 20 MHz


//Configuration bits settings code goes here
// CONFIG1H
#pragma config OSC = HS         // Oscillator Selection bits (HS oscillator)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 3         // Brown Out Reset Voltage bits (Minimum setting)

// CONFIG2H
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config CCP2MX = PORTC   // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = ON      // PORTB A/D Enable bit (PORTB<4:0> pins are configured as analog input channels on Reset)
#pragma config LPT1OSC = OFF    // Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
#pragma config MCLRE = ON      // MCLR Pin Enable bit (RE3 input pin enabled; MCLR disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000800-001FFFh) not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (002000-003FFFh) not code-protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (004000-005FFFh) not code-protected)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (006000-007FFFh) not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000800-001FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (002000-003FFFh) not write-protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (004000-005FFFh) not write-protected)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (006000-007FFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000800-001FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (002000-003FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (004000-005FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (006000-007FFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot block (000000-0007FFh) not protected from table reads executed in other blocks)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.


//variable defination 
char *passcode;                 //variable pointer to store the initial passcode
char *password;                 //variable to store each user entered passcode 
int counter;                    //counter to check if all passcode is correct (correct = 4)
unsigned int address;           //variable used to store value for eeprom address
int i;                          //variable to store the index
char input;                     //variable to store menu choices made by user
char user_input[4];             //array of char to store the passcode
double curr_temp;               //stores the current temperature reading
double prev_temp;
char tempr[4];                  //stores the current temperature
char thres[4];                  //stores the current temperature
int keypad_used=0;              //store int to determine input method 1=keypad, 0=keyboard
double threshld_temp = 75.00;   //set initial threshold tempreture
unsigned int temp_reading;      //stores temperature reading
char key_pressed;               //stores the character pressed using the keypad


//initialize variable to store different state of the alarm system
unsigned char *pir_state = "DISABLED";
unsigned char *tempr_state = "DISABLED";
unsigned char *input_method = "KEYBOARD";

//function Prototype

void startup();
char input_key_pressed(); 
char *get_userpasscode();
char *get_userinput();
void passcode_check();
void pass_check();
void login_page();
char read_EEPROM(int add);
void write_EEPROM(int add, char c);
void update_pwd();
void resetScreen();
void delay();
int verify_pass(); 
void get_state();
void componentStatus();
void motion_sensor_menu();
void motion_sensor_settings();
void motion_triggered();
void settingsTimer();
void interrupt My_ISR_High(void);
void interrupt low_priority My_ISR_Low(void);
void display_input();
void tempr_reading();
void tempr_sensor_menu();
void set_threshold_temp();
void tempr_sensor_setting(); 
void keypad();
void change_passcode();
void initial_login();
void temp_triggered();
void get_input();
void print_msg(char *msg);



void putch(unsigned char chr)
/*
 *  void putch()
 *
 *  This function helps to print the characters to the EUSART communication terminal
 */
{
    while (TXIF == 0)       // wait until TXREG is not busy
        ;
    TXREG = chr;            // send character
}




void print_msg(char *data) 
/*
 *  void print_msg()
 *
 *  This is an independent function that takes char pointer as input and prints the characters to the screen 
 *  instead of calling printf function directly
 */
{
    while (*data)                       // iterates the loop while there is a chracter remaining in the array
    {
        while (TXSTAbits.TRMT == 0);    // wait until possible previous transmission data is done
        TXREG = *data;                  // send 8-bit byte
        while (TXSTAbits.TRMT == 1);    // wait until transmit shift register status bit is empty
        data++;                         // increments the message pointer to next address
    }
}

int main()
/*
 *  int main()
 *
 *  This is the main function that initializes port settings, communication settings and 
 *  keypad settings. It directs to initial create passcode page if the pic is powered on for first time
 *  or else redirects to the login page
 */
{ 
    ADCON1 = 0b10001110;    // PORTA & PORTE digital I/O pins
    TRISA = 0b11100001;     // I/0 settings for PORTA
    TRISB = 0b11000000;     // I/0 settings PORTB
    TRISC = 0b11010000;     // I/0 settings PORTC
    TRISD = 0b11111111;     // I/0 settings PORTD
    TRISE = 0b00000100;     // I/0 settings PORTE
    PORTBbits.RB5 = 0;      // turn on the green LED
    TRISB=0b00000000;       // set port B as output
    
    __delay_ms(50);         //delay
    
    //settings for serial communication
    TRISCbits.RC7 = 1;      //set TX as output  
    TRISCbits.RC6 = 0;      //set RX as input
    SPBRG = 32;             //setup baud rate 
    TXSTAbits.SYNC = 0;     //asyncronous transmittion
    TXSTAbits.TXEN = 1;     //transmit enabled
    RCSTAbits.SPEN = 1;     //RCSTA Register Settings enable serialport
    RCSTAbits.CREN = 1;     //enable continous reciever

    //settings for keypad
    TRISDbits.RD7 = 1;      //set RD7 as the input
    TRISDbits.RD6 = 1;      //set RD6 as the input
    TRISDbits.RD5 = 1;      //set RD5 as the input
    TRISDbits.RD4 = 1;      //set RD4 as the input
    TRISDbits.RD3 = 0;      //set RD3 as the output
    TRISDbits.RD2 = 0;      //set RD3 as the output 
    TRISDbits.RD1 = 0;      //set RD3 as the output
    TRISDbits.RD0 = 0;      //set RD0 as the output

    PORTDbits.RD0 = 1;      //set RD0 high
    PORTDbits.RD1 = 1;      //set RD1 high
    PORTDbits.RD2 = 1;      //set RD2 high
    PORTDbits.RD3 = 1;      //set RD3 high
    
    
    startup();                        //load initail settings
    motion_sensor_settings();         //load the settings for the motion sensor
    settingsTimer();                  //load the setting for the timer
    tempr_sensor_setting();           //load temperature settings
    tempr_reading();                  //get the current temperature reading

    while (1) 
    {         // maintains the loop until the program is halted
   
        PORTBbits.RB3 = 1;      //set the green LED on: System up and running

        if (read_EEPROM(0x00) == (char) 255)    //reprogramming the pic or first time login
        {  
            initial_login();                    //goes to create pascode page
        }       
        else                          
            {
            print_msg("\n\rEnter your Passcode to continue: ");
            pass_check();                       //login normally 
        }
    
    }
}



char *get_userinput()
/*
 *  char *get_userinput()
 *
 *  This function gets the passcode entered by the user using keypad or keyboard
 *  and save the data into the EEPROM of the PIC
 */
{
    int i;
    address = 0x00;                                     //set address to main address of EEPROM
    if (keypad_used == 1 && PORTBbits.RB4 == 1)
    {
     for (i = 0; i < 4; i++)                            // maintains loop 4 times to get the 4-digit passcode
        {
            keypad();                                   // calls the keypad() function to get character entered
            PORTBbits.RB4 = 0;                          //turn of LED
            passcode[i] = key_pressed;                  // sets the char variable option with the key pressed
            _EEREG_EEPROM_WRITE(address, passcode[i]);  //write to the EEPROM
            delay();                                    // calls a delay function for 0.5 sec delay          
            print_msg("*");                             // prints * inplace of characters
            PORTBbits.RB4 = 1;                          
            address++;                                  //increment the pointer
        }
        while (key_pressed!= 'D') {                     // checks if the 'D' is pressed
            keypad();                                   // call keypad()function
        }
    }
    else
    {
        for (i = 0; i < 4; i++)                         //to set 4 digit for the passcode
        {           
            passcode[i] = input_key_pressed();          //call function to get the input from user
            _EEREG_EEPROM_WRITE(address, passcode[i]);  // writes the individual character to the address specified in EEPROM
            print_msg("*");                             //print character '*' for passcode entered
            address++;                                  // increment pointer to the next address
        }
        while (RCREG != 13);                            // checks for Enter key pressed
    }
    return passcode;                                    //returns 4 digit passcode
}




char *get_userpasscode()
/*
 *  char *get_userpasscode()
 *
 *  This function returns the passcode entered by the user using keypad or keyboard
 *  and save the data into the variable password used for verification
 */
{
    int i;                          //intialize index
    if (keypad_used == 1 && PORTBbits.RB4 == 1)
    {
     for (i = 0; i < 4; i++) 
        {
            keypad();                    // calls the keypad function
            PORTBbits.RB4 = 0;
            user_input[i] = key_pressed; // sets the char variable option with the key pressed
            delay();             
            print_msg("*");             // prints * for the passcode
            PORTBbits.RB4 = 1;
        }
        while (key_pressed!= 'D') 
        {      // checks for D key pressed
            keypad();                   // calls the keypad function to get the input
        }
    }
    else
    {
        for (i = 0; i < 4; i++)       
        {
            user_input[i] = input_key_pressed();    //get the user pressed key
            print_msg("*");               // prints * for passcode entered
        }
        while (RCREG != 13);                //wait for enter key to be pressed
    }
        return user_input;                  // returns the result array
       
}
    



char input_key_pressed()
/*
 *  char input_key_pressed()
 *
 *  This function returns the single input entered by the user to choose between the
 *  menu options
 */
{
    while (PIR1bits.RC1IF == 0);    // while the key is pressed
    char input = RCREG;             //copy the RC data into input
    return input;                   // returns input
  
}




void settingsTimer() 
/*
 * void settingsTimer()
 *
 *  This function initializes necessary timer modes and settings. It loads the high and low
 *  bits with the preload value 
 */
{
    INTCONbits.TMR0IF = 0;      // clears the Timer0 interrupt flag
    INTCONbits.TMR0IE = 1;      // sets the Timer0 interrupt enable bit
    INTCON2bits.TMR0IP = 0;     // sets Timer0 interrupt as low priority
    T0CON = 0b00000110;         // controll register settings
    TMR0H = 0x67;               // load TMR0H with preload value
    TMR0L = 0x69;               // load TMR0L with preload value

}




void login_page()
/*
 * void login_page()
 *
 *  This function prints the main menu UI and handles the input given by the user.
 *  it calls another function to display the status of the components
 */
{
    
    int i;          //index
  
    while(1)        //start the loop
    {
        print_msg("\033[2J");  
        print_msg("\033[14;1H");
        componentStatus(); 
        print_msg("\r|------------------------------------------------------------------------------|\n\r");
        print_msg("\r|                             ---MAIN MENU---                                  |\n\r");
        print_msg("\r|------------------------------------------------------------------------------|\n\r");
        print_msg("\r|     1. Motion Sensor Menu                                                    |\n\r");
        print_msg("\r|     2. Tempreture Sensor Menu                                                |\n\r");
        print_msg("\r|     3. Change Input Method to KEYPAD                                         |\n\r");
        print_msg("\r|     4. Change Input Method to KEYBOARD                                       |\n\r");
        print_msg("\r|     5. Change Current Passcode                                               |\n\r");
        print_msg("\r|                                                  0. Return to Main Menu      |\n\r");
        print_msg("\r--------------------------------------------------------------------------------\n\r");
        __delay_ms(50);
       
   
        print_msg("\n\rEnter your choice: ");
        if (keypad_used == 1 && PORTBbits.RB4 == 1)
        {
         
            keypad();                       // get input key from the keypad
            PORTBbits.RB4 = 0;              //turn off LED
            __delay_ms(300);
            while (TXSTAbits.TRMT == 0);    // wait until previous transmission data is done
            TXREG = key_pressed;            // copy the data
            input = key_pressed;            // sets the char variable option with the key pressed
            delay();                        // calls a delay function for 1 sec delay
            PORTBbits.RB4 = 1;
            
            while (key_pressed != 'D') 
            {                               // checks if the 'D' is pressed in keypad, here pressing 'D' functions as enter
                keypad();                   //calls keypad function
            }
        }
        else
        {
            input = input_key_pressed();        // get the input from the user
            while (TXSTAbits.TRMT == 0);        // wait for transmission
            TXREG = input;                      // transfer data
            while (RCREG != 13);                // wait for enter key to be pressed
        }
        switch (input) 
        {
            case '1':
                delay();
                motion_sensor_menu();           //go to motion sensor menu  
                break;
                
            case '2':
                delay();
                tempr_sensor_menu();            //go to temperature alarm menu
                break;
                 
            case '3':
              
                input_method = "Keypad  ";      // sets Keypad as input method
                write_EEPROM(0x033, '1');    // stores the input method to EEPROM
                PORTBbits.RB4 = 1;              // Turn on the blue led
                keypad_used = 1;                // keyboard status on
                print_msg("\033[2J");     
                print_msg("\n\rInput method is now changed to KEYPAD---");
                delay();
                delay();
                break;
                
            case '4':   
               
                input_method = "Keyboard";      // input method is keyboard
                write_EEPROM(0x033, '0');    // stores the input method to EEPROM
                PORTBbits.RB4 = 0;              // blue LED turned off
                keypad_used = 0;                // keypad status off
                print_msg("\033[2J"); 
                print_msg("\n\rInput method is now changed to KEYBOARD---");
                delay();
                delay();
                break;
                           
            case '5': 
                    
                change_passcode();              //change passcode using another function
                break;

            case '0':
                print_msg("\033[2J");       
                delay();
                print_msg("\n\rreturning to main menu ------   ");
                __delay_ms(1000);
                exit(0);                        //exits to login page

            default:                            //if invalid option is selected
                print_msg("\n\r--Invalid Option--\n\r---select again--");
                __delay_ms(1000);
                print_msg("\033[2J");
                break;
        }
    }
        
}


void startup() 
/*
 * void startup()
 *
 *  This function update the passcode by reading the EEPROM and displays the welcome
 *  screen
 */
{
    int i;
    
    
   
    for (i = 0, address = 0x00; i < 4; address++, i++)
    {
        password[i] = read_EEPROM(address);
    }
    print_msg("\033[2J"); 
    print_msg("\r--------------------------------------------------------------------------------\n\r");
    print_msg("\r|                       CSE 3442/5442: Embedded Systems 1                      |\n\r");
    print_msg("\r|Lab 7: Building a PIC18F4520 Standalone Alarm System with EUSART Communication|\n\r");
    print_msg("\r|------------------------------------------------------------------------------|\n\r");
    print_msg("\r|                                                                              |\n\r");
    print_msg("\r|   NAME:   Prabesh Khadka                                                     |\n\r");
    print_msg("\r|   UTA ID: 100120007                                                          |\n\r");
    print_msg("\r|------------------------------------------------------------------------------|\n\r");
    print_msg("\r|                    !!!WELCOME TO THE ALARM SYSTEM!!!                         |\n\r");
    print_msg("\r--------------------------------------------------------------------------------\n\r");
    print_msg("\r\n");
    print_msg("\r\n");
    print_msg("\r\n");
    __delay_ms(50);

}

void update_pwd()
/*
 * void update_pwd()
 *
 *  This function updates the current passcode stored in the variable by reading the PIC's 
 *  EEPROM.
 */
{
    int i=0;
    for (i = 0, address=0x00; i < 4; address++, i++)    //read EEPROM
    {
        passcode[i] = read_EEPROM(address);
    }
}

void write_EEPROM(int add, char c) 
/*
 * void write_EEPROM(int add, char c)
 *
 *  This function takes in the address and the character to be stored. The function stores character c 
 *  into the given EEPROM memmory addresss
 */
{
    EEADR = add;                //EEPROM location destination
    EEDATA = c;                 //EEDATA to write user input
    
                                //EECON2 configurations for writing
    EEPGD = 0;
    CFGS = 0;
    WREN = 1;
    EECON2 = 0x55; 
    EECON2 = 0xAA; 
    EECON1bits.WR = 1;          // set WR bit
    while (EECON1bits.WR == 1); // wait to clear WR
}


char read_EEPROM(int add) 
/*
 * void read_EEPROM(int add)
 *
 *  This function takes in the address and reads the character stored at that particular
 *  memory location
 */
{
    EEADR = add;        //EEPROM source
    
    EEPGD = 0;
    CFGS = 0;
    RD = 1;
    return EEDATA;      //fetch and return EEDATA
}



void componentStatus() 
/*
 * void componentStatus()
 *
 *  This function displays the current states of the system component in the terminal.
 */
{
    print_msg("\r--------------------------------------------------------------------------------\n\r");
    print_msg("\r|                       CSE 3442/5442: Embedded Systems 1                      |\n\r");
    print_msg("\r|Lab 7: Building a PIC18F4520 Standalone Alarm System with EUSART Communication|\n\r");
    print_msg("\r|------------------------------------------------------------------------------|\n\r");
    print_msg("\r|                              ---Alarm Status---                              |\n\r");
    print_msg("\r|------------------------------------------------------------------------------|\n\r");
    printf("\r|     Motion Sensor:         %s                                          |\n\r", pir_state);
    printf("\r|     Temperature Sensor:    %s                                          |\n\r",tempr_state);
    printf("\r|     Input method selected: %s                                          |\n\r",input_method);
    print_msg("\r|                                                                              |\n\r");
    printf("\r|     Current Temperature:   %.2fF                                            |\n\r",curr_temp);
    printf("\r|     Threshold Temperature: %.2fF                                            |\n\r",threshld_temp);
       
}




void get_state() 
/*
 * void get_state()
 *
 *  This function gets the status of the components and helps to update the current 
 *  status
 */
{
//
//    //Checking the values in EEPROM
    if (read_EEPROM(0x0017) != (char) 255) 
                //check status in EEPROM for motion sensor's state
    {

       if (read_EEPROM(0x0017) == '1') 
       {
            pir_state = "ENABLED ";
            motion_sensor_settings();
            INTCONbits.INT0IE = 1;          //Enable high priority interrupt
       } 
       else if (read_EEPROM(0x0017) == '0')
       {
            pir_state = "DISABLED";
            INTCONbits.INT0IE =0;           //Disable the INT0 interrupt
       }
    }

    if (read_EEPROM(0x011) != (char) 255)    //temperature settings location
    {
        int i;
        unsigned int temp_address = 0x011;                     //location where temperature settings were saved
        for (i = 0; i < 5; i++)             // i<5 because we are only two digit after decimal value
        {
            thres[i] = read_EEPROM(temp_address);    //copy the contents into an array
            temp_address++;                          //increment the address
        }
        threshld_temp = atof(thres);                //convert the values into double and update threshold temperature
    }
    if (read_EEPROM(0x05) != (char) 255)                
    {
        if (read_EEPROM(0x05) == '1') {
            tempr_state = "ENABLED ";
            tempr_sensor_setting();
            T0CONbits.TMR0ON = 1;                   //enable timer
        } 
        else if (read_EEPROM(0x05) == '2') 
        {
            tempr_state = "DISABLED";
            T0CONbits.TMR0ON = 0;                   //disable timer
            PIE1bits.ADIE = 0;                      //Set ADIE enable bit
            PORTBbits.RB5 = 0;
        }
    }                                                  //check stored method of input
    if (read_EEPROM(0x033) != (char) 255) 
    {
        if (read_EEPROM(0x033) == '1')               //for keypad
        {

            PORTBbits.RB4 = 1;                      //turn on the LED
            keypad_used = 1;
            input_method = "Keypad  ";              //update the state
        }   
        else if (read_EEPROM(0x033) == '0')          //for keyboard
        {
            PORTBbits.RB4 = 0;
            input_method = "Keyboard";              //update the state
        }
    }

   
}



void motion_sensor_settings()
/*
 * void motion_sensor_settings()
 *
 *  This function initializes necessary settings for high priority interrupts. It enables the interrupts
 */
{
    RCONbits.IPEN = 1; //Enabling both high and low priority Interrupts

    INTCONbits.GIE = 1;             // enables the global interrupt 
    INTCONbits.INT0IE = 0;          //disable the interrupt
    INTCON2bits.INTEDG0 = 0;         //interrupt on falling edge
    INTCONbits.INT0IF = 0;          // clears the INT0IF flag bit
    TRISBbits.RB2 = 0;              //PORTB pin 2 as an output to light up Red led
    TRISBbits.RB0 = 1;              //B0 as input
    PORTBbits.RB2 = 0;              //RB2 set low

    TRISBbits.RB3 = 0;              //set up as output

}


void motion_sensor_menu()
/*
 * void motion_sensor_menu()
 *
 *  This function provides UI to the motion sensor menu and handles the input response by
 *  either turning ON or OFF the motion alarm
 */
{       
    int i=1;
    
    print_msg("\033[2J");
    componentStatus(); 
    print_msg("\r|------------------------------------------------------------------------------|\n\r");
    print_msg("\r|                         ---Motion Sensor MENU---                             |\n\r");
    print_msg("\r|------------------------------------------------------------------------------|\n\r");
    print_msg("\r|     1. Enable Motion Sensor                                                  |\n\r");
    print_msg("\r|     2. Disable Motion Sensor                                                 |\n\r");
    print_msg("\r|                                                                              |\n\r");
    print_msg("\r|                                                     0. Return to main menu   |\n\r");
    print_msg("\r--------------------------------------------------------------------------------\n\r");
    __delay_ms(50);
    print_msg("\n\rEnter your choice: ");
    
    if (keypad_used == 1 && PORTBbits.RB4 == 1)         //if key pad is used as input method
    {
         
        keypad();                                // calls the keypadOnly() function to get the key pressed
        PORTBbits.RB4 = 0;                      
        __delay_ms(300);
        while (TXSTAbits.TRMT == 0);            // wait until transmission is completed
        TXREG = key_pressed;            // send one 8-byte
        input = key_pressed;            // sets the char variable option with the key pressed
        delay();                        // calls a delay function for 1 sec delay
        PORTBbits.RB4 = 1;
        while (key_pressed != 'D')
        { // checks if the 'D' is pressed 
            keypad();
        }
    }
    else
    {
        input = input_key_pressed();        // get the input from the user
        while (TXSTAbits.TRMT == 0);        // wait for transmission
        TXREG = input;                      // transfer data
        while (RCREG != 13);
    }
    switch (input) 
    {
        case '1':                  
            write_EEPROM(0x0017, '1');    // update status of PIR sensor in the EEPROM
            pir_state = "ENABLED ";     // motion sensor state is ON
            motion_sensor_settings();    // calls settingsPIR function to load the PIR settings
            INTCONbits.INT0IE = 1;      // enables the high priority interrupt
            print_msg("\033[2J");
            print_msg("\n\rMotion Sensor has been enabled!!");
            delay();
            delay();
            break;
            
        case '2': 
            write_EEPROM(0x0017, '0');    // update status of PIR sensor in the EEPROM
            pir_state = "DISABLED";     // motion sensor state is disabled
            INTCONbits.INT0IE = 0;      // disables the high priority interrupt
            PORTBbits.RB3 = 0;          // turn off led
            print_msg("\033[2J");
            print_msg("\n\rMotion Sensor has been disabled!!");
            delay();
            delay();
            break;
            
        case '0':
            print_msg("\033[2J");      //new page
            delay();
            print_msg("\n\rreturning to main menu ------   ");
            __delay_ms(1000);
            exit(0); 
            
        default:                   
            //if invalid option is selected
            print_msg("\n\r--Invalid Option--\n\r");
            print_msg("---select again--\n\r");
            __delay_ms(1000);
            //print_msg("\033[2J");
            break;
       }
       
}

void pass_check()
/*
 * void pass_check()
 *
 *  This function verifies the passcode entered with the system's passcode by returning the 
 *  total number of matched digit. It does this by calling another counter function
 */
{
    update_pwd();                       //update passcode
    get_state();                        //get the curren component state
    tempr_reading();                    
    while(1)        //create a loop
    {
      
        password = get_userpasscode();    // gets the passcode input from the user
        counter = verify_pass();          // verifies the passcode and returns counter
        
        if (counter==4)                  // if the passcode is valid
        {
            print_msg("\033[2J");
            print_msg("\n\r---Passcode Accepted---");
            delay();
            
            
            print_msg("\n\r!!Wait a moment!!");
            delay();
            delay();
            
            login_page();               //go the main menu
        } 
    
        else 
            // if the passcode is invalid ask user to try again
        {
            print_msg("\n\r---Incorrect Password Entered!!---");
            print_msg("\n\r!!Try Again!!");
            delay();
            print_msg("\n\rEnter your Passcode again: ");
          
        }
    }
        
}


int verify_pass()
/*
 * int verify_pass()
 *
 *  This function returns the total number of matched digit entered in the passcode
 *  versus the passcode stored in the system's EEPROM. If the passcode matches then the \
 *  function returns 4  
 */
{
    update_pwd();
    int i, tick = 0;            //intialize variable to track count
    address = 0x00;             //set EEPROM address
    
    for (i = 0;i < 4; i++) 
    {
        if((password[i])==(read_EEPROM(address)))
        {
            tick++;             //increment count for each correct passcode       
        }    
           address++;           //increment EEPROM address       
    }
    return tick;    //return result as integer
 }

  
    
    
void delay()
/*
 * void delay()
 *
 *  This function creates a delay of 0.5 seconds 
 */
{
    int i;
    for (i = 0; i < 25; i++)
    {
        __delay_ms(20);         // calls delay function in XC.h to creates 20 milliseconds delay
    }
}

void interrupt My_ISR_High(void) 
/*
 * void interrupt My_ISR_High(void)
 *
 *  This function is a high priority interrupt function that checks for interrupt int the INT0
 *  by constanstly checking for interrupt flag caused by motion sensor
 */
{

    if (INTCONbits.INT0IF == 1 && INTCONbits.INT0IE == 1)
    {                                       // checks if the INT0IF and INT0IE is set high
        PORTBbits.RB2 = 1;                  //Turns on the red LED to indicate motion
        INTCONbits.INT0IF = 0;              //Reset the interrupt flag 
        motion_triggered();
    }
}


void interrupt low_priority My_ISR_Low(void) 
/*
 * void interrupt low_priority My_ISR_Low(void)
 *
 *  This function is a low priority interrupt function that checks for interrupt int the TMR0
 *  by constanstly checking for interrupt flag and also by checking the ADC interrupt flag
 *  it compares the current temperature with threshold temperature and triggers the temp alarm
 */
{

      if (INTCONbits.TMR0IF == 1)       // checks if the TMR0IF is set high
    {
        T0CONbits.TMR0ON = 0;           // stops the Timer0
        prev_temp = curr_temp;          // store previous temperature
        tempr_reading();                // gets current temperature reading
       INTCONbits.TMR0IF = 0;           // clears the TMR0IF flag bit
       
        TMR0H = 0x67;
        TMR0L = 0x69;
        T0CONbits.TMR0ON = 1;           // Timer0 ON
    }
      
    if (PIR1bits.ADIF == 1)             // checks if the ADC done
    {
        temp_triggered();               //call tempreture alarm triggered function
    }
    PIR1bits.ADIF = 0;              //clear flag
}


void motion_triggered() 
/*
 * void motion_triggered()
 *
 *  This function is prints the UI for the motion dectect and promts the user to enter the
 *  passcode to reset the alarm. The system also prompts the user if the want to re-enable 
 *  the alarm
 */
{
    int i;    

    print_msg("\033[2J"); 
    print_msg("\r--------------------------------------------------------------------------------\n\r");
    print_msg("\r|                      !!!  Motion Has Been Detected  !!!                      |\n\r");
    print_msg("\r|------------------------------------------------------------------------------|\n\r");
    print_msg("\n\r");
    
    while(1) 
    {
        print_msg("\n\n\rEnter the passcode to reset the alarm: ");
        delay();
        password = get_userpasscode();    // gets the passcode input from the user
        counter = verify_pass();          // verifies the passcode and returns interger        

        if (counter == 4) 
        {
            PORTBbits.RB2 = 0;
            print_msg("\033[2J");
            while(1)
            {
                print_msg("\r\n Do you want to re-enable motion sensor?(Yes=1/No=2)\n\r");
                print_msg("\n\rEnter your choice: ");
                delay();
                    // checks if the input from the keypad only mode is on
                if (keypad_used == 1 && PORTBbits.RB4 == 1)
                {

                 keypad();                  // keypad function 
                PORTBbits.RB4 = 0;          //turn off LED
                __delay_ms(300);
                while (TXSTAbits.TRMT == 0); // wait until possible previous transmission data is done
                TXREG = key_pressed;         // send one 8-byte
                input = key_pressed;        // store key pressed
                delay();                     // calls a delay function for 1 sec delay
                PORTBbits.RB4 = 1;
                while (key_pressed != 'D') 
                {                               // checks if the 'D' is pressed in keypad, here pressing 'D' functions as enter
                    keypad();
                }
          
            }
            else
            {
                input = input_key_pressed();        // get the input from the user
                while (TXSTAbits.TRMT == 0);        // wait for transmission
                TXREG = input;                      // transfer data
                while (RCREG != 13);
            }
            switch (input) 
            {
                case '1':                           // iPIR sensor alarm enabled
                    motion_sensor_settings();       // load motoin sensor settings
                     write_EEPROM(0x0017, '1');
                    pir_state = "ENABLED ";         // sets status enabled
                    INTCONbits.INT0IE = 1;          //Enable the INT0 interrupt
                    print_msg("\033[2J");
                    print_msg("\n\rMotion Sensor alarm has been re-enabled!");
                    delay();
                    delay();
                    write_EEPROM(0x0017, '1');        // saves the status of the PIR to EEPROM
                    exit(0);

                case '2':
                    pir_state = "DISABLED";         // sets the status of the PIR to variable of char *
                    write_EEPROM(0x0017, '0');        // writes the status of the PIR to EEPROM
                    INTCONbits.INT0IE = 0;          //Disable the INT0 interrupt
                    print_msg("\033[2J");
                    print_msg("\n\rMotion sensor alarm has been disabled!");
                    delay();
                    delay();
                    exit(0);

                default:            //if invalid option is selected
                    print_msg("\n\r--Invalid Option--\n\r");
                    print_msg("---select again--\n\n\r");
                    __delay_ms(1000);
                     break;
            }
        }
        }else {
            
            print_msg("\n\r---Incorrect Password Entered!!---");
            print_msg("\n\r!!Try Again!!");
            delay();
            delay();

        }
        
      }
       
}

void set_threshold_temp()
/*
 * void set_threshold_temp()
 *
 *  This function allows the user to enter the thershold temperatur in double value and 
 *  store it in the pic EEPROM
 */
{
    int i=0;
    unsigned int addrr= 0x011;
    print_msg("\n\rEnter new threshold temperature: ");
    delay(); // calls delay function for 0.5 sec delay
    if (keypad_used == 1 && PORTBbits.RB4 == 1)  
    {
        for (i = 0; i < 2; i++) 
        {
            keypad();                       // calls the keypad() function
            
            tempr[i] = key_pressed;         // inputs the key pressed
            
            write_EEPROM(addrr, tempr[i]);  // write to EEPROM
            
            while (TXSTAbits.TRMT == 0);    // wait until transmission data is done 
            TXREG = tempr[i];               // send data
            delay();
            addrr++;                         // moves the pointer to the next address
            delay();
            PORTBbits.RB4 = 0;
            delay();                         // calls a delay function for 1 sec delay
            PORTBbits.RB4 = 1;
        }
        while (key_pressed != 'D') 
        {                                       // waits until D is pressed
            keypad();
        }
     }
     else
     {  
        do
        {
            tempr[i] = input_key_pressed();     // gets the input from the user
            write_EEPROM(addrr, tempr[i]);
            while (TXSTAbits.TRMT == 0);        // wait until transmission data is done 
            TXREG = tempr[i];                   // send data
            addrr++;                            // moves the pointer to the next address
            i++;                                   //increment index
        }while (RCREG != 13); // checks if the user hit 'Enter'
    }   
    threshld_temp = atof(tempr);                // converts the temperature to double
    print_msg("\033[2J");
    print_msg("\n\rNew Temperature Sensor Alarm Threshold has been set..");
    delay(); 
    delay();                                    // calls delay function for 0.5 sec delay
}


void tempr_reading()
/*
 *  tempr_reading()
 *
 *  This function conducts the ADC and displays the current temperature in Farhenheit Scale
 */
{
    int i;
    ADCON0 = 0b00000001;        // sets the ADCON0 register 
                                // input from port A0 for FOSC/4
    
    ADCON1 = 0b00001110;        // setting the A/D port
    ADCON2 = 0b10101100;        // sets the configuration of ADCON2 register to select
                                // the result format as right justified
                                // and A/D conversion clock select bits
    TRISAbits.TRISA0 = 1;       // sets the PORT A0 as input
    ADCON0bits.GO = 1;          // A/D conversion in progress
    while (ADCON0bits.DONE == 1);       // wait while A/D conversion is done
    temp_reading = ADRESH;              // right justified
    temp_reading = temp_reading << 8;   // left shifts the value by 8
    temp_reading += ADRESL;             // adds the result
    curr_temp = temp_reading;
    curr_temp = (double) ((curr_temp / 1023)*5000);  //convert result to a range of 0 to 5V  
    curr_temp = curr_temp - 500;                     // subtracts the offset of 500 mV 
    curr_temp = curr_temp / 10;                      // converts to degree celsius [10 mV = 1 degree celsius]
    curr_temp = (curr_temp)*1.8 + 32;                // converts degree to F [F = C*1.8 + 32]
     
}


void tempr_sensor_setting() 
/*
 *  tempr_sensor_setting() 
 *
 *  This function intialize settings for the motion sensor
 */
{
    RCONbits.IPEN = 1;          // enable high and low priority interrupt
    PIR1bits.ADIF = 0;          //Clear ADIF flag bit
    IPR1bits.ADIP = 0;          //ADC is low Priority
    PIE1bits.ADIE = 0;          //ADIE disabled
    INTCONbits.PEIE = 1;        //PEIE enabled
    INTCONbits.GIE = 1;         //GIE enabled
    PORTBbits.RB5 = 0;          //turn off the led
}


void tempr_sensor_menu()  
/*
 *  tempr_sensor_menu() 
 *
 *  This function display the menu for temoerature sensor alarm option. The user can
 *  either enable, disable the alarm or change the thershold temperature
 */
{   
    print_msg("\033[2J");
    componentStatus(); 
    print_msg("\r|------------------------------------------------------------------------------|\n\r");
    print_msg("\r|                       ---Temperature Sensor MENU---                          |\n\r");
    print_msg("\r|------------------------------------------------------------------------------|\n\r");
    print_msg("\r|      1. Enable Temperature Sensor                                            |\n\r");
    print_msg("\r|      2. Disable Temperature Sensor                                           |\n\r");
    print_msg("\r|      3. Change Threshold Temperature                                         |\n\r");
    print_msg("\r|                                                                              |\n\r");
    print_msg("\r|                                                  0. Return to main menu      |\n\r");
    print_msg("\r|                                                                              |\n\r");
    print_msg("\r--------------------------------------------------------------------------------\n\r");
    __delay_ms(50);
       
    print_msg("\n\rEnter your choice: ");
    if (keypad_used == 1 && PORTBbits.RB4 == 1)
    {
        keypad();               // calls the keypadOnly() function to get the key pressed
        PORTBbits.RB4 = 0;
        __delay_ms(300);
        while (TXSTAbits.TRMT == 0);// wait until transmission data is done
        TXREG = key_pressed;        // send one 8-byte
        input = key_pressed;        // copies the character pressed
        delay();                    // delay
        PORTBbits.RB4 = 1;
        while (key_pressed != 'D')
        {                           // checks if the D key is pressed
            keypad();
        }
    }
    else
    {
        input = input_key_pressed();        // get the input from the user
        while (TXSTAbits.TRMT == 0);        // wait for transmission
        TXREG = input;                      // transfer data
        while (RCREG != 13);
    }
       
    switch (input) 
    {
        case '1': 
            write_EEPROM(0x05, '1');    //write to EEPROM
            tempr_state = "ENABLED ";   //save the state to EEPROM
            T0CONbits.TMR0ON = 1;       //Timer0 ON
            PIE1bits.ADIE=1;            // enables the AD conversion
            PORTBbits.RB5 = 0;          //turn off the LED
            delay(); 
            delay();
            break;

        case '2':
            write_EEPROM(0x05, '0');    //Write to EEPROM
            tempr_state = "DISABLED";   //temperature alarm state disabled
            T0CONbits.TMR0ON = 0;       // stops the Timer0
            PIE1bits.ADIE = 0;          //ADIE diabled
            PORTBbits.RB5 = 0;          //turn off led
            print_msg("\n\rTemperature Sensor has been disabled.");
            delay();
            break;
            
        case '3':
            set_threshold_temp();       //change threshold temperature
            break;
            
        case '0':
                print_msg("\033[2J");      //new page
                delay();
                print_msg("\n\rreturning to main menu ------   ");
                __delay_ms(1000);
                exit(0); 
            
        default:                   //if invalid option is selected
            print_msg("\n\r--Invalid Option--\n\r");
            print_msg("---select again--\n\r");
            __delay_ms(1000);
            break;
       }
       
}

void keypad()
/*
 *  void keypad() 
 *
 *  This function is used to get the key pressed in the 4x4 keypad using the alorithm to 
 *  check for high bits in the row and column and identifying the char key pressed
 */
{
    while(1)
    {
        
        PORTDbits.RD0 = 1;  //set RD0 high
        PORTDbits.RD1 = 0;  //set RD1 low
        PORTDbits.RD2 = 0;  //set RD2 bit low
        PORTDbits.RD3 = 0;  //set R3 bit low
    
        if(PORTDbits.RD4 == 1)  //for key 1
        {
            PORTDbits.RD0 = 0;  //set key low
            PORTBbits.RB4 = 0;  //blue light off
            PORTBbits.RB4 = 1;  //toggle lights
            key_pressed = '1'; 
            break;              //return 
        }
        
        if(PORTDbits.RD5 == 1)  //for key 2
        {
            PORTDbits.RD0 = 0;  //set key low    
            PORTBbits.RB4 = 0;  //blue LED
            PORTBbits.RB4 = 1;  //toggle LED
            key_pressed ='2';   //2 key pressed
            break;              //return 
        }
        if(PORTDbits.RD6 == 1)  //for key 3
        {
            PORTDbits.RD0 = 0;  //set key low
            PORTBbits.RB4 = 0;  //blue led off
            //delay();
            PORTBbits.RB4 = 1;  //toggle LED
            key_pressed ='3';   //3 key pressed
            break;              //return
        }
        
        if(PORTDbits.RD7 == 1)  //for key A
        {
            PORTDbits.RD0 = 0;  //set key low
            PORTBbits.RB4 = 0;  //blue led off
            PORTBbits.RB4 = 1;  //toggle LED
            key_pressed ='A';   //3 key pressed
            break;              //return 
        }
        
        PORTDbits.RD0 = 0;      //for next row
        PORTDbits.RD1 = 1;
        PORTDbits.RD2 = 0;
        PORTDbits.RD3 = 0;
    
        if(PORTDbits.RD4 == 1) //forkey4
        {
            PORTDbits.RD1 = 0;  //set key low
            PORTBbits.RB4 = 0;  //blue led off
            //delay();
            PORTBbits.RB4 = 1;  //toggle LED
            key_pressed = '4';  //4 key pressed
            break;              //return 
        }
    
        if(PORTDbits.RD5 == 1)  //for key5
        {
            PORTDbits.RD1 = 0;  //set key low
            PORTBbits.RB4 = 0;  //blue led off
            //delay();
            PORTBbits.RB4 = 1;  //toggle LED
            key_pressed ='5';   //5 key pressed
            break;              //return 
        
        }
    
        if(PORTDbits.RD6 == 1)  //for key 6
        {
            PORTDbits.RD1 = 0;  //set key low
            PORTBbits.RB4 = 0;  //blue led off
            //delay();
            PORTBbits.RB4 = 1;  //toggle LED
            key_pressed ='6';   //6 key pressed
            break;              //return 
        }
        
        if(PORTDbits.RD7 == 1)  //for key B
        {
            PORTDbits.RD1 = 0;  //set key low
            PORTBbits.RB4 = 0;  //blue led off
            // delay();
            PORTBbits.RB4 = 1;  //toggle LED
            key_pressed ='B';   //B key pressed
            break;              //return 
        }
	
        //Sets PORT RD2 to high
        PORTDbits.RD0 = 0;
        PORTDbits.RD1 = 0;
        PORTDbits.RD2 = 1;
        PORTDbits.RD3 = 0;

        if(PORTDbits.RD4 == 1)  //for key 7
        {
            PORTDbits.RD2 = 0;  //set key low
            PORTBbits.RB4 = 0;  //blue led off
            //delay();
            PORTBbits.RB4 = 1;  //toggle LED
            key_pressed ='7';   //7 key pressed
            break;              //return
        }

        if(PORTDbits.RD5 == 1)  //for key 8
        {
            PORTDbits.RD2 = 0;  //set key low
            PORTBbits.RB4 = 0;  //blue led off
            //delay();
            PORTBbits.RB4 = 1;  //toggle LED
            key_pressed = '8';  //8 key pressed
            break;              //return 
        }

        if(PORTDbits.RD6 == 1)  //key9
        {
            PORTDbits.RD2 = 0;  //set key low
            PORTBbits.RB4 = 0;  //blue led off
            // delay();
            PORTBbits.RB4 = 1;  //toggle LED
            key_pressed ='9';   //9 key pressed
            break;              //return 
        }
        
        if(PORTDbits.RD7 == 1)  //key C
        {
            PORTDbits.RD2 = 0;  //set key low
            PORTBbits.RB4 = 0;  //blue led off
            // delay();
            PORTBbits.RB4 = 1;  //toggle LED
            key_pressed ='C';   //C key pressed
            break;              //return 
        }
        //Sets PORT RD3 to high
        PORTDbits.RD0 = 0;
        PORTDbits.RD1 = 0;
        PORTDbits.RD2 = 0;
        PORTDbits.RD3 = 1;

        if(PORTDbits.RD4 == 1)  //'*' key pressed
        {
            PORTDbits.RD3 = 0;  //set key low
            PORTBbits.RB4 = 0;  //blue led off
            // delay();
            PORTBbits.RB4 = 1;  //toggle LED
            key_pressed = '*';  //* key pressed
            break;              //return 
        }
        
        if(PORTDbits.RD5 == 1)  //'0' key pressed
        {
            PORTDbits.RD3 = 0;  //set key low
            PORTBbits.RB4 = 0;  //blue led off
            //delay();
            PORTBbits.RB4 = 1;  //toggle LED
            key_pressed = '0';  //0 key pressed
            break;              //return 
        }
    
        if(PORTDbits.RD6 == 1)  //'#' key pressed
        {
            PORTDbits.RD3 = 0;  //set key low
            PORTBbits.RB4 = 0;  //blue led off
            // delay();
            PORTBbits.RB4 = 1;  //toggle LED
            key_pressed ='#';   //# key pressed
            break;              //return
        }
    
        if(PORTDbits.RD7 == 1)  //for key D
        {
            PORTDbits.RD4 = 0;  //set key low
            PORTBbits.RB4 = 0;  //blue led off
            // delay();
            PORTBbits.RB4 = 1;  //toggle LED
            key_pressed ='D';   //D key pressed
            break;              //return
        }
    }
}

void change_passcode()
/*
 *  void change_passcode() 
 *
 *  This function to change the current passcode. It prompts the user for current passcode 
 *  and if the passcode matches, it gets the new passcode entered and stores it in the PICs
 *  EEPROM and updates the system passcode
 */
{
    print_msg("\033[2J");  //new page 
    print_msg("\r--------------------------------------------------------------------------------\n\r");
    print_msg("\r|                       CSE 3442/5442: Embedded Systems 1                      |\n\r");
    print_msg("\r|Lab 7: Building a PIC18F4520 Standalone Alarm System with EUSART Communication|\n\r");
    print_msg("\r|------------------------------------------------------------------------------|\n\r");
    print_msg("\r|                                                                              |\n\r");
    print_msg("\r|                                                                              |\n\r");
    print_msg("\r|                                                                              |\n\r");
    print_msg("\r|                                    CHANGE PASSCODE                           |\n\r");
    print_msg("\r|                                                                              |\n\r");
    print_msg("\r|                                                                              |\n\r");
    print_msg("\r|------------------------------------------------------------------------------|\n\r");
    print_msg("\r|                                                                              |\n\r");
    print_msg("\r--------------------------------------------------------------------------------\n\r");
    print_msg("\n\r");
    print_msg("\n\r");
    print_msg("\n\r");
    print_msg("\n\r");
    print_msg("\r\nEnter current password to verify: ");
    
    password = get_userpasscode();  // gets the passcode input from the user
    counter = verify_pass();        //verify user entered passcode and get int value in return
                    
    if (counter==4)                 //if the passcode is correct
    {
        print_msg("\r\nEnter the new passcode: "); 
        delay();
        passcode = get_userinput();

        while (RCREG != 13);        
        print_msg("\n\r");
        print_msg("\n\r--Password changed successfullly, use it for next login!!--");
        delay();
        delay();
        exit(0);
    }
    else            //if the passcode is entered incorrectly
    {
        print_msg("\n\r--Incorrect passcode entered--");
        delay();
        delay();
    }
               
}

void initial_login()
/*
 *  void initial_login() 
 *
 *  This function to displays the inital login page for the first time login right after the 
 *  PIC has been reprogrammed
 */
{
    print_msg("\033[2J"); 
    print_msg("\r--------------------------------------------------------------------------------\n\r");
    print_msg("\r|                       CSE 3442/5442: Embedded Systems 1                      |\n\r");
    print_msg("\r|Lab 7: Building a PIC18F4520 Standalone Alarm System with EUSART Communication|\n\r");
    print_msg("\r|------------------------------------------------------------------------------|\n\r");
    print_msg("\r|                                                                              |\n\r");
    print_msg("\r|                                                                              |\n\r");
    print_msg("\r|   NAME:   Prabesh Khadka                                                     |\n\r");
    print_msg("\r|   UTA ID: 100120007                                                          |\n\r");
    print_msg("\r|                                                                              |\n\r");
    print_msg("\r|------------------------------------------------------------------------------|\n\r");
    print_msg("\r|                                 INTIAL SIGNUP PAGE                           |\n\r");
    print_msg("\r--------------------------------------------------------------------------------\n\r");
    __delay_ms(50);
    print_msg("\n\nWelcome to the passcode setup page!\n\n");
    print_msg("\r|---------------------|\n\r");
    print_msg("\r| CREATE NEW PASSCODE |\n\r");
    print_msg("\r|---------------------|\n\r");
    __delay_ms(50);
    print_msg("\n\n\rEnter new Passcode for the alarm system :");
            
            
    passcode = get_userinput();     //call function to get user input
    while (RCREG != 13);            // wait until user hits 'ENTER'
    print_msg("\n\r");
    print_msg("\n\rPasscode created Successfullly, use it for next !!");
    print_msg("\n\r");
    __delay_ms(500);
    __delay_ms(500);

} 


void temp_triggered()
/*
 *  void temp_triggered() 
 *
 *  This function to displays the menu to enter the passcode and the reset the alarm once
 *  the temperature alarm has been triggerd
 */
{
    tempr_reading();                // gets the current temperature reading
  
    if (curr_temp >= threshld_temp)
    {                               // compares the current temperature with the threshold
        PORTBbits.RB5 = 1;          // if true, turns on the Yellow in steady state
        T0CONbits.TMR0ON = 0;       // stops the timer
        print_msg("\033[2J");
        print_msg("\r--------------------------------------------------------------------------------\n\r");
        print_msg("\r|                  !!!  Temperature Alarm Has Been Triggered  !!!              |\n\r");
        print_msg("\r--------------------------------------------------------------------------------\n\r");
        print_msg("\n\r");
   
        while(1) 
        {
            print_msg("\n\n\rEnter the passcode to reset the alarm: ");
            delay();
            password = get_userpasscode();    // gets the passcode input from the user
            counter = verify_pass();          // verifies the passcode and returns interger        
            if (counter == 4) 
            {
                PORTBbits.RB2 = 0;              //turn off the led
                print_msg("\033[2J");
                while(1)
                {
                    print_msg("\r\nDo you want to re-enable temperature Alarm?(Yes=1/No=2)\n\r");
                    print_msg("\n\rEnter your choice: ");
                    delay();
                
                                                // checks if the input from the keypad only mode is on
                    if (keypad_used == 1 && PORTBbits.RB4 == 1)
                    {
                        keypad();                   // calls the keypad funtion
                        PORTBbits.RB4 = 0;           //turn off the LED
                        __delay_ms(300);
                        while (TXSTAbits.TRMT == 0); // wait until transmission of data is done
                        TXREG = key_pressed;         // send one 8-byte
                        input = key_pressed;        // copy the input key pressed
                        delay();                     // calls a delay function for 1 sec delay
                        PORTBbits.RB4 = 1;
     
                        while (key_pressed != 'D') 
                        {                           // checks if the D key is pressed
                            keypad();
                        }
                    }
                    else
                    {
                        input = input_key_pressed();        // get the input from the user
                        while (TXSTAbits.TRMT == 0);        // wait for transmission
                        TXREG = input;                      // transfer data
                        while (RCREG != 13);
                    }
                    switch (input) 
                    {
                        case '1': 
                            tempr_state = "ENABLED ";
                            write_EEPROM(0x05, '1');        // saves the temperature sensor status to EEPROM
                            T0CONbits.TMR0ON = 1;            // stars the Timer0
                            PORTBbits.RB5 = 0;               // turns off the Yellow LED
                            print_msg("\n\rTemperature Sensor has been ENABLED.");
                        
                            delay();
                            print_msg("\n\r");
                            print_msg("\r\nDo you want to change the threshold temperature?(Yes=1/No=2)\n\r");
                            print_msg("\n\rEnter your choice: ");
                            delay();
                            get_input();                        //get single key input
                            switch (input) 
                            {
                                case '1':
                                    set_threshold_temp();          //set threshold tempreture
                                    delay();
                                    break;

                                case '2':
                                    delay();
                                    break;

                                default:
                                    delay();
                                    break;
                            }
                            exit(0);

                        case '2':   
                            tempr_state = "DISABLED";               //save state to EEPROM
                            write_EEPROM( 0x05, '0');                //temp alarm enabled
                            T0CONbits.TMR0ON = 0;                   // stops the timer
                            PIE1bits.ADIE = 0;                      // ADIE disabled
                            PORTBbits.RB5 = 0;                       // turns off the Yellow LED
                            print_msg("\n\rTemperature Sensor has been DISABLED.");
                            exit(0);

                        default: 
                            tempr_state = "ENABLED ";
                            write_EEPROM(0x05, '1');                   // temp alarm enabled
                            T0CONbits.TMR0ON = 1;                      // stars the Timer0
                            PORTBbits.RB5 = 0;                          // turns off the Yellow LED
                            print_msg("\n\rTemperature Sensor Alarm is ENABLED.");
                            exit(0);
                    }
                }
            }
            else 
            {
            
                print_msg("\n\r---Incorrect Password Entered!!---");
                print_msg("\n\r!!Try Again!!");
                delay();
                delay();
            }
        
        }
    
    } 
    else if (prev_temp != curr_temp)
    {
        PORTBbits.RB5 = 1;
        __delay_ms(100); // sets the delay of 20 milli seconds
        PORTBbits.RB5 = 0;
    }        
}

void get_input()
/*
 *  void get_input() 
 *
 *  This function get the single input character entered by the user either using the
 *  keypad or the keyboard
 */
{
    if (keypad_used == 1 && PORTBbits.RB4 == 1)
    {
         
        keypad();               // calls the keypad functipn
        PORTBbits.RB4 = 0;
        __delay_ms(300);
        while (TXSTAbits.TRMT == 0); // wait until transmission is done
        TXREG = key_pressed;         // send data
        input = key_pressed;        // get the input key pressed
        delay();                    // delays
        PORTBbits.RB4 = 1;          //turn on the led
       
        while (key_pressed != 'D') 
        {                           // checks if the D key is pressed
            keypad();
        }
          
    }
    else
    {
        input = input_key_pressed();        // get the input from the user
        while (TXSTAbits.TRMT == 0);        // wait for transmission
        TXREG = input;                      // transfer data
        while (RCREG != 13);                // wait until enter is hit
    }
}   

//a blank line at the end of the program

//

