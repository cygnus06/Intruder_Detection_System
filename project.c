// FINAL-> 
// trig is right 
// BUZZER NIS ABOVE STEPPER MOTOR 2ND COL LAST BUT 1 ROW
// P 0.4 -> BUZZER IS CNA ROW 2 COL 1
#include <LPC17xx.h>
#include <stdio.h>
#include <string.h>
void scan(void);

unsigned long int col, row, flag ;
//unsigned long int var1,temp_key,temp_key1,temp_key2;
#define RS_CTRL 0x08000000 //P0.27
#define EN_CTRL 0x10000000 //P0.28
#define DT_CTRL 0x07800000 //P0.23 to P0.26 data lines
#define PRESCALE (25000-1) 
#define TRIGGER_PIN (1 << 15)  // P0.15 (Trigger Pin)
#define ECHO_PIN (1 << 16)     // P0.16 (Echo Pin)

unsigned long int temp1=0, temp2=0,i,j ;
unsigned long int temp3 = 0;
unsigned long int temp = 0;
unsigned char flag1 =0, flag2 =0;
unsigned char msg[20] = {""};
unsigned char initialize_system[] = {"Set Password\0"};
unsigned char welcome[] = {"Enter 4 digits\0"};
unsigned char can_enter[] = {"Welcome In\0"};
unsigned char cannot_enter[] = {"Wrong Password\0"};
unsigned char ALERT[] = {"ALERT MODE\0"}; 
int digits = 0;
int attempts = 0;
void initTimer0(void);
void delayinUS(unsigned int microseconds);
void delayMS(unsigned int milliseconds);
void scan_till_four_digits(void);
void check_pass(char *a, char *b);
void lcd_write(void);
void port_write(void);
void LCD_interface(unsigned char *a);
void clearLCD(void);
void initLCD(void);
void SetPassword(void);
void appendToArray(unsigned char* arr, unsigned char ch);
 void timer_start(void);
void init_ultrasonic(void);
float measure_distance(void);
void check_proximity(void);
void check_proximity(void) ;
void buzzer_buzz(void);
int count = 0;
unsigned long int init_command[] = {0x30,0x30,0x30,0x20,0x28,0x0c,0x06,0x01,0x80};
unsigned char keypad[4][4] = {
    {'0', '1', '2', '3'},  // Row 0
    {'4', '5', '6', '7'},  // Row 1
    {'8', '9', 'A', 'B'},  // Row 2
    {'C', 'D', 'E', 'F'}   // Row 3
};
unsigned char curr_char ;
unsigned char password[] = {"1234"};
unsigned char buff[50];

int main(){
		LPC_PINCON->PINSEL3 = 0; // 1.23 TO 1.26 GPIO
		LPC_PINCON->PINSEL4 = 0; //2.10 TO 2.13 GPIO
		LPC_GPIO2->FIODIR = 0XF << 10; // 2.10 TO 2.13 OUTPUT
	   LPC_PINCON->PINMODE3 |= 0xAAAA0000;  // Enable pull-down for P1.23-P1.26 NOT SURE
	//lcd part
		LPC_GPIO0->FIODIR = DT_CTRL | RS_CTRL | EN_CTRL; //Config output
		LPC_GPIO0->FIODIR = LPC_GPIO0->FIODIR |(1<<4);
		initLCD();
		initTimer0();
		init_ultrasonic();
		//set Pass
		LCD_interface(initialize_system);
		delayMS(4000);
		//clearLCD();
		scan_till_four_digits();
		strcpy((char*)password,(char*) buff);
		digits = 0;
		strcpy((char*) buff, "\0");
		clearLCD();
	//attempts
		LCD_interface(welcome);
		while(attempts < 3){
			//while(1){
			//check_proximity();
			//}
			//while(1);
		scan_till_four_digits();
		delayMS(2000);
		clearLCD();
	//new addittion
		check_pass((char*)buff, (char*)password);
		strcpy((char*) buff, "\0");
		digits = 0;
		}
		while(1);
		
	
}

void initTimer0(void)
 { 
/*Assuming that PLL0 has been setup with CCLK = 100Mhz and PCLK = 25Mhz.*/ 
		LPC_SC->PCONP |= (1<<1); //Power up TIM0. By default TIM0 and TIM1 are enabled. 
		LPC_SC->PCLKSEL0 &= ~(0x3<<3); //Set PCLK for timer = CCLK/4 = 100/4 (default) 
		LPC_TIM0->CTCR = 0x0; 
		LPC_TIM0->PR = PRESCALE; //Increment TC at every 24999+1 clock cycles 
//25000 clock cycles @25Mhz = 1 mS 
		LPC_TIM0->TCR = 0x02; //Reset Timer
 }
 
 void delayinUS(unsigned int microseconds){
		LPC_TIM0->TCR = 0x02;
		LPC_TIM0->PR = 0; // Set prescaler to the value of 0
		LPC_TIM0->MR0 = microseconds - 1; // Set match register for 10us
		LPC_TIM0->MCR = 0x01; // Interrupt on match
		LPC_TIM0->TCR = 0x01; // Enable timer
		while ((LPC_TIM0->IR & 0x01) == 0); // Wait for interrupt flag
		LPC_TIM0->TCR = 0x00; // Stop the timer
		LPC_TIM0->IR = 0x01; // Clear the interrupt flag
 }
 void delayMS(unsigned int milliseconds) //Using Timer0
 { 
		delayinUS(1000 * milliseconds);
 }
 
void appendToArray(unsigned char* arr, unsigned char ch) {
    char tempStr[2];  // Temporary string to hold one char (plus '\0')
    tempStr[0] = ch;
    tempStr[1] = '\0';  // Null-terminate the string
    strcat((char*)arr, tempStr);  // Append the character to the array
}
void buzzer_buzz(){
	while(1){
			LPC_GPIO0->FIOSET = 1<<4;
			delayMS(2000);
			LPC_GPIO0->FIOCLR = 1<<4;
			delayMS(2000);
			}
}

void check_pass(char *a, char *b){
	if(strcmp(a,b) == 0){
		LCD_interface(can_enter);
	}else{
		attempts++;
		if(attempts == 3){
			LCD_interface(cannot_enter); 
			delayMS(1500);
			clearLCD();
			LCD_interface(ALERT);
			while(1){
				check_proximity();
			}			
				
		}else{
			LCD_interface(cannot_enter);
			delayMS(1500);
			clearLCD();
			LCD_interface(welcome);			
		}		
	}
}
void scan(void){
	//unsigned long temp3;
	temp3 = LPC_GPIO1->FIOPIN;
	temp3 &= 0x07800000;
	if(temp3 != 0x00){
			flag = 1;
			if(temp3 == 1<< 23) col = 0;
			else if(temp3 == 1<<24) col = 1;
			else if (temp3 == 1<< 25) col = 2;
			else if( temp3 == 1<< 26) col = 3;
	}
	delayMS(2000);
}

void scan_till_four_digits(void){
	 while(digits < 4){
		 for(row = 0; row <4; row++){
			 if(row == 0) temp = 1<<10;
			 else if(row == 1) temp = 1<< 11;
			 else if(row == 2) temp = 1<< 12;
			 else if(row == 3) temp = 1<< 13;
			 LPC_GPIO2->FIOPIN = temp; //WRITING TO PORT 2
			 flag = 0;
			 delayMS(1000);
			 scan();
			 if(flag == 1){
				 digits++;
				 curr_char = keypad[row][col];
				 //LCD_interface(msg);
				 clearLCD();
				 appendToArray(buff,curr_char);
				 LCD_interface(buff);
				 delayMS(2000);
			 }				 
		 }
	 }
}


void LCD_interface(unsigned char *msgtodisplay){
		flag1 =1;//Data
		i =0;
	while (msgtodisplay[i] != '\0')
		{
		temp1 = msgtodisplay[i];
		lcd_write();//Send data bytes
			i++;
		}	
}
//3 times send 8-bit mode command, followed by 4- bit mode
void lcd_write(void)
{
		flag2 = (flag1 == 1) ? 0 :((temp1 == 0x30) || (temp1 == 0x20)) ? 1 : 0;//If command is 0x30 (Working in 8-bit
//		mode initially), send ‘3’ on D7-D4 (D3-D0 already grounded)
		temp2 = temp1 & 0xf0;//
		temp2 = temp2 << 19;//data lines from 23 to 26. Shift left (26-8+1) times so that higher digit is sent on P0.26 to
	//	P0.23
		port_write(); // Output the higher digit on P0.26-P0.23
		if (!flag2) // Other than command 0x30, send the lower 4-bt also
		{
		temp2 = temp1 & 0x0f; //26-4+1
		temp2 = temp2 << 23;
		port_write(); // Output the lower digit on P0.26-P0.23
		}
}

void port_write(void)
{
		LPC_GPIO0->FIOPIN = temp2;
		if (flag1 == 0)
		LPC_GPIO0->FIOCLR = RS_CTRL; // Select command register
		else
		LPC_GPIO0->FIOSET = RS_CTRL; //Select data register
		LPC_GPIO0->FIOSET = EN_CTRL; //Apply -ve edge on Enable
		delayMS(50);
		LPC_GPIO0->FIOCLR = EN_CTRL;
		delayMS(2000);
}

void clearLCD(void){
		flag1 = 0; //Command mode
		temp1 = init_command[7];
		lcd_write(); //send Init commands to LCD
		flag1 = 1;
}

void initLCD(void){
	flag1 =0;//Command
		for (i=0; i<9;i++)
		{
		temp1 = init_command[i];
		lcd_write(); //send Init commands to LCD
		}
		//LCD_interface(welcome);
		//delayMS(2000);
}
 void timer_start(void){
	LPC_TIM0->TCR = 0x02; // Reset Timer
	LPC_TIM0->TCR = 0x01; // Enable timer
}

float timer_stop(){
	LPC_TIM0->TCR = 0x0;
	return LPC_TIM0->TC;
}

// Initialize pins
void init_ultrasonic(void) {
    // Configure pins
    LPC_PINCON->PINSEL0 &= 0x3fffffff;     // Configure TRIG P0.15
    LPC_PINCON->PINSEL1 &= 0xfffffff0;     // Configure ECHO P0.16
    LPC_GPIO0->FIODIR |= TRIGGER_PIN;      // Set TRIGGER as output
    LPC_GPIO1->FIODIR |= 0 << 16;          // Set ECHO as input
}


// Function to measure distance
float measure_distance(void) {
    int echoTime;
    float distance;
    
    // Send 10us trigger pulse
    LPC_GPIO0->FIOSET |= TRIGGER_PIN;      // Set TRIGGER pin HIGH
    delayinUS(50);                       // Wait for 10 microseconds
    LPC_GPIO0->FIOCLR |= TRIGGER_PIN;      // Set TRIGGER pin LOW
    
    // Wait for echo pulse
    while (!(LPC_GPIO0->FIOPIN & ECHO_PIN)); // Wait for ECHO to go HIGH
    timer_start();                           // Start timing
    
    while (LPC_GPIO0->FIOPIN & ECHO_PIN);    // Wait for ECHO to go LOW
    echoTime = timer_stop();                 // Stop timing
    
    // Calculate distance in cm (using speed of sound = 343 m/s)
    distance = (0.00343 * echoTime) / 2;
    
    return distance;
}

// Example usage in main loop
void check_proximity(void) {
    float distance = measure_distance();
    sprintf((char *)msg, "%.2f", distance);
		clearLCD();
			LCD_interface(msg);
    // Check if object is closer than 7cm
    if (distance < 10 ) {
        // Object is near - take action
        //LPC_GPIO0->FIOSET = 1 << 22;       // Turn on warning LED/buzzer
			sprintf((char *)msg, "%.2f", distance);
				clearLCD();
			LCD_interface(ALERT);
			//LPC_GPIO0->FIOSET = 1<<4;
				buzzer_buzz();
				//while(1);			
    }
		
    
    delayinUS(500);  // Wait before next measurement
}


