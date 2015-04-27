/*********************************************
Project : ATmega128_MT
Version : 01
limex                            
Comments: Demo program


Chip type           : ATmega128
Program type        : Application
Clock frequency     : 16,000000 MHz
Memory model        : Small
External SRAM size  : 0
Data Stack size     : 1024
*********************************************/
#include <avr/io.h>
#include <avr/iom128.h>
#include "system.h"
#include "bits.h"
#include "lcd.h"
#include "delay.h"
#include "eeprom.h"
#include <stdlib.h>                                                        

#define		B1				(PINA&BIT0)  
#define		B2				(PINA&BIT1)
#define		B3				(PINA&BIT2)
#define		B4				(PINA&BIT3)
#define		B5				(PINA&BIT4)


#define 	RELAY_HIGH		PORTA |= BIT6
#define 	RELAY_LOW		PORTA &= ~BIT6	                      

#define		TXD				(PIND&BIT3)	
#define		RXD				(PIND&BIT2)	
#define		DALLAS			(PINA&BIT5)
     

unsigned char ch;
int led=0;
int ftc,fth;

void EEPROM_write(unsigned int uiAddress, unsigned char ucData)
{
/* Wait for completion of previous write */
while(EECR & (1<<EEWE))
;
/* Set up address and data registers */
EEAR = uiAddress;
EEDR = ucData;
/* Write logical one to EEMWE */
EECR |= (1<<EEMWE);
/* Start eeprom write by setting EEWE */
EECR |= (1<<EEWE);
}


unsigned char EEPROM_read(unsigned int uiAddress)
{
/* Wait for completion of previous write */
while(EECR & (1<<EEWE))
;
/* Set up address register */
EEAR = uiAddress;
/* Start eeprom read by writing EERE */
EECR |= (1<<EERE);
/* Return data from data register */
return EEDR;
}

#define	__AVR_ATMEGA128__	1
#define OSCSPEED	16000000		/* in Hz */
unsigned char data, Line = 0;
char Text[16], Ch;
unsigned int Bl = 1, LCD_State = 0, i, j;


void LCDSendInt(long a)
{
	int C[20];
	unsigned char Temp=0, NumLen = 0;
	if (a < 0)
	{
		LCDSendChar('-');
		a = -a;
	}
	do
	{	
		Temp++;
		C[Temp] = a % 10;
		a = a/10;
	}
	while (a>0);
	NumLen = Temp;
	for (Temp = NumLen; Temp>0; Temp--) 
	  SEND_CHAR(C[Temp] + 48);
}


void Delay(unsigned int a)
{
	while (a) 
	{
		a--;
	}
}

void E_Pulse()
{
	PORTC = PORTC | 0b00000100;	//set E to high
	Delay(2000); 				//delay ~110ms
	PORTC = PORTC & 0b11111011;	//set E to low
}


void LCDSendChar(unsigned char a)
{
	data = 0b00001111 | a;					//get high 4 bits
	PORTC = (PORTC | 0b11110000) & data;	//set D4-D7
	PORTC = PORTC | 0b00000001;				//set RS port to 1
	E_Pulse();                              //pulse to set D4-D7 bits

	data = a<<4;							//get low 4 bits
	PORTC = (PORTC | 0b11110000) & data;	//clear D4-D7
	PORTC = PORTC | 0b00000001;				//set RS port to 1 -> display set to command mode
	E_Pulse();                              //pulse to set d4-d7 bits
}
void LCD_Init()
{
	//LCD initialization
	//step by step (from Gosho) - from DATASHEET

	PORTC = PORTC & 0b11111110;
	
	Delay(10000);


	PORTC = 0b00110000;						//set D4, D5 port to 1
	E_Pulse();								//high->low to E port (pulse)
	Delay(1000);

	PORTC = 0b00110000;						//set D4, D5 port to 1
	E_Pulse();								//high->low to E port (pulse)
	Delay(1000);

	PORTC = 0b00110000;						//set D4, D5 port to 1
	E_Pulse();								//high->low to E port (pulse)
	Delay(1000);

	PORTC = 0b00100000;						//set D4 to 0, D5 port to 1
	E_Pulse();								//high->low to E port (pulse)
}

void LCDSendCommand(unsigned char a)
{
	data = 0b00001111 | a;					//get high 4 bits
	PORTC = (PORTC | 0b11110000) & data;	//set D4-D7
	PORTC = PORTC & 0b11111110;				//set RS port to 0
	E_Pulse();                              //pulse to set D4-D7 bits

	data = a<<4;							//get low 4 bits
	PORTC = (PORTC & 0b00001111) | data;	//set D4-D7
	PORTC = PORTC & 0b11111110;				//set RS port to 0 -> display set to command mode
	E_Pulse();                              //pulse to set d4-d7 bits

}

void LCDSendTxt(char* a)
{
	int Temp;
	for(Temp=0; Temp<strlen(a); Temp++)
  {
    LCDSendChar(a[Temp]);
  }
}



void PORT_Init()
{
	PORTA = 0b00000000;		DDRA = 0b01000000;	//Relay set as output (Bit6 = 1)
	PORTB = 0b00000000;		DDRB = 0b00000000;
	PORTC = 0b00000000;		DDRC = 0b11110111;
	PORTD = 0b11000000;		DDRD = 0b00001000;	//TX set as output (Bit3 = 1)
	PORTE = 0b00000000;		DDRE = 0b00110000;	//Buzzer set as output (Bit4 = 1, Bit5 = 1)
	PORTF = 0b00000000;		DDRF = 0b00000000;
	PORTG = 0b00000000;		DDRG = 0b00000000;
}

void UART_Init(uint32_t Baud)
{
	unsigned int BaudRate = OSCSPEED / (16 * Baud) - 1;	/* as per pg. 173 of the user manual */

	//set BaudRate to registers UBRR1H and UBRR1L
	UBRR1H = (unsigned char) (BaudRate>>8);
	UBRR1L = (unsigned char) BaudRate;

	UCSR1B = UCSR1B | 0b00011000;	//enable Receiver and Transmitter (Bit3 = 1, Bit4 = 1)

	UCSR1C = UCSR1C | 0b10000110;	//Set frame format: 8 data (Bit1 = 1, Bit2 = 1), 1 stop bit (Bit3 = 0)
}

unsigned char UART_Receive()
{
	if (UCSR1A & 0b10000000)	//if there is unreaded data
		return UDR1;
	else						//no unreaded data
		return 0;
}




void UART_Transmit(unsigned char data)
{
	while (!(UCSR1A & 0b00100000));	//waiting until buffer is ready to receive

	UDR1 = data;
}



int get_values()
{   
   char ch,c,t;
    
    
    while(1) 
	{ 
     ch=UART_Receive();
     if(ch)
	 {   
	   switch(ch)
	   {  case 'x' :  do
	                  {c=UART_Receive();}while(!c);
					  if(c=='!')
					  {
					  		EEPROM_write(20,0);
					  }
					  else
					  {	
	                  		EEPROM_write(20,c);
					  }          
                      break;	   
	    
		  case 'c' :  do
	                  {c=UART_Receive();}while(!c);
	                  if(c=='!')
					  {
					  		EEPROM_write(21,0);
					  }
					  else
					  {	
	                  		EEPROM_write(21,c);
					  }          
                      break;	   
	   	  case 'd' :  do
	                  {c=UART_Receive();}while(!c);
	                  if(c=='!')
					  {
					  		EEPROM_write(22,0);
					  }
					  else
					  {	
	                  		EEPROM_write(22,c);
					  }          
                      break;	   
	   
	      case 'z' :  do
	                  {c=UART_Receive();}while(!c);
	                  if(c=='!')
					  {
					  		EEPROM_write(23,0);
					  }
					  else
					  {	
	                  		EEPROM_write(23,c);
					  }          
                      break;	   
	   
	      case 't' :  do							//new
	                  {c=UART_Receive();}while(!c);
	                  if(c=='!')
					  {
					  		EEPROM_write(24,0);
					  }
					  else
					  {	
	                  		EEPROM_write(24,c);
					  }          
                      break;	   
	   
	      case 'q' :   EEPROM_write(10,0);					//finish
		  			   EEPROM_write(25,0);				//new
						LCDSendTxt("Q");
					  return 0;   
		  
		  
	   }    
      }
	  	if(led==0)								//LED blinking code										
		{
			led=1;
		}
		else
		{
			led=0;
		}
		if(led==1)
		{
			PORTA=0b01000000;
			delay_ms(10);
		}
		else
		{
			PORTA=0b00000000;
			delay_ms(10);
		}
	  }
     
return 1;
}
 
unsigned long s,h,ctr=0;

unsigned int hr,min,sec,lt;
	


void delay_msa(unsigned long d,int z) {
 long i=d;
	while (--i!=0)
	{           
	            delay_ms(93);      
	            SEND_CMD(CLR_DISP); 
	            SEND_CMD(DISP_ON);
	            SEND_STR("Time Left::");
                s=(z)-ctr;
				h=s/3600;
				SEND_CMD (DD_RAM_ADDR2);
				
				
				LCDSendInt(s/3600);
				SEND_STR(":");
				LCDSendInt((s/60)-(h*60));
				SEND_STR(":");
				LCDSendInt(s%60);
				ctr++;
                 

	 
	}
    
	 
				


}
 

void delay_msb(unsigned long d) {
 long i=d;
	while ((i--)!=0)
	{           
	           if(B1==0)
				{ fth=EEPROM_read(24);
                  EEPROM_write(25,EEPROM_read(24));
				  				  
				} 
				
				if(fth==0)
				{ SEND_CMD(CLR_DISP); 
	              SEND_CMD(DISP_ON);
	              SEND_STR("fragrance over");
				  Buzzer(); 
				 }
				else
                { SEND_CMD(CLR_DISP); 
	              SEND_CMD(DISP_ON);
	              SEND_STR("Time Left::");
                  }

                SEND_CMD (DD_RAM_ADDR2);
				if(sec==0)
				{  if(min==0 && hr==0)
				   { sec=0; min=0; hr=0; }
				
				   else if(min==0)
				   {   min=59;
				       sec=60;
	                   hr=hr-1;
					   if(fth>0)
                      { 
					   fth=fth-1;			   
				       EEPROM_write(25,fth);
				       }
				   
				   }

                   else 
				   {  min=min-1;
				      sec=60;
				   }				
				
				}
				sec=sec-1;
		    	 				
				LCDSendInt(hr);
				SEND_STR(":");
				LCDSendInt(min);
				SEND_STR(":");
				LCDSendInt(sec);
				delay_ms(91);      
	            
				ctr++;
                 

	 
	}
    
	 
				


}



void delay_msc(unsigned long d,int z) {
 long i=d,j;

	while ((i--)!=0)
	{           
	            if(B1==0)
				{ SEND_CMD(CLR_DISP); 
	              SEND_CMD(DISP_ON);
				  SEND_STR("New");
				  fth=EEPROM_read(24);
                  EEPROM_write(25,EEPROM_read(24));
				   				  
				} 
				
				
				if(fth==0)
				{ 
				   
				  SEND_CMD(CLR_DISP); 
	              SEND_CMD(DISP_ON);
	              SEND_STR("fragrance over");
				  SEND_CMD (DD_RAM_ADDR2);
				 Buzzer();
                  
				 
			    j=z-lt;		
				LCDSendInt(j);
				delay_ms(93);
				lt=lt+1;
				    
				 }
				else
                { SEND_CMD(CLR_DISP); 
	              SEND_CMD(DISP_ON);
	              SEND_STR("Time Left::");
                  
				 	
			 		 
			     
				   SEND_CMD (DD_RAM_ADDR2);
				 
				  j=z-lt;		
				  LCDSendInt(j);
				  lt=lt+1;		
				  delay_ms(93);
				  
				  }
			   
				        
	            if(fth>0)
				{
				fth=fth-1;
                EEPROM_write(25,fth);
				}
				ctr++;
                 

	 
	}
    
	 
				


}



int main(void) {     

    ctr=0;   
	unsigned char a,w,str[3];
	int val,ft;
	hr=0;min=0;sec=0,ft=0,fth=0,ftc;
	lt=0;
	 
	InitPorts();
	UART_Init(9600);


 
	 
	     char ch,x,c,d,z;
		 int flag,y;           	 
	
	
	LCD_Ini(); 					     
	  		
 
 	SEND_CMD(DISP_ON);
	 delay_ms(10);  						
	SEND_CMD(CLR_DISP); 
	SEND_CMD(DISP_ON);
	
	SEND_STR("****Welcome****");	

delay_ms(300);	
if(B1==0)								//programming mode
{ 
	
	SEND_CMD(CLR_DISP); 
	SEND_CMD(DISP_ON);
	delay_ms(100);
	SEND_STR("Programming...");
	delay_ms(50);
	 
	   {    
	   		again: 
			SEND_CMD(CLR_DISP); 
	        SEND_CMD(DISP_ON);
	        delay_ms(100); 
			
	        SEND_STR("Waiting...");    
          	get_values();								//getting values
            
			SEND_CMD(CLR_DISP); 
	        SEND_CMD(DISP_ON);
	        delay_ms(100); 
			SEND_STR("Done...");
	        delay_ms(100);
	    }

		 PORTA=0b01000000;	
         SEND_CMD(CLR_DISP); 
	     SEND_CMD(DISP_ON);
	     delay_ms(100);
	     SEND_STR("-Data Values-");  
		 delay_ms(150);
		 
		 x=EEPROM_read(20);
	 	 SEND_CMD(CLR_DISP); 
	     SEND_CMD(DISP_ON);
	     
		 SEND_STR("X=");
         val=x;
		  
		 LCDSendInt(val);
		 
          	 
	     c=EEPROM_read(21);
		 SEND_STR(" TON=");
         val=c;
		 LCDSendInt(val);
		 	  
	 
		 d=EEPROM_read(22);
		 SEND_CMD(DD_RAM_ADDR2);
		 SEND_STR("TOFF=");
         val=d;
		 LCDSendInt(val);
		 
		 
	 		  
         z=EEPROM_read(23);
		 SEND_STR("Tot");
         val=z;
		 LCDSendInt(val);
			
		 SEND_STR(",");			 

		 ft=EEPROM_read(24);
         val=ft;
		 LCDSendInt(val);
		  delay_ms(500);

		 SEND_CMD(CLR_DISP); 
	     SEND_CMD(DISP_ON); 
		 SEND_STR("Press B4 for new");
		  
		 fth=0;
		 delay_ms(500);
		 while(1)						//continuos check
		 {
		 	if(B1==0)
		 	{
	        	goto again;	 
		 	}	 
         	else
         	SEND_CMD(CLR_DISP); 
	     	SEND_CMD(DISP_ON); 
		 	SEND_STR("Complete!");
		}


}	     

	else if(EEPROM_read(10))			//values check 
	{
		while(1)
		{
				PORTA=0b00000000;
				if(B1==0)
				{
					goto again;
				}
				 
		}
	}
		
	else 	
		
		{ 
        	
		fth=EEPROM_read(25);	  //new
	 		 
		
		 
		 
		
		SEND_CMD(CLR_DISP); 
	     SEND_CMD(DISP_ON);
	     delay_ms(100);
	     SEND_STR("-Data Values-");  
		 delay_ms(150);
		 
		 x=EEPROM_read(20);
	 	 SEND_CMD(CLR_DISP); 
	     SEND_CMD(DISP_ON);
	     
		 SEND_STR("X=");
         val=x;
		  
		 LCDSendInt(val);
		 
          	 
	     c=EEPROM_read(21);
		 SEND_STR(" TON=");
         val=c;
		 LCDSendInt(val);
		 	  
	 
		 d=EEPROM_read(22);
		 SEND_CMD(DD_RAM_ADDR2);
		 SEND_STR("TOFF=");
         val=d;
		 LCDSendInt(val);
		 
		 
	 		  
         z=EEPROM_read(23);
		 SEND_STR(" Tot=");
         val=z;
		 LCDSendInt(val);

		 SEND_STR(",");			 	//new

		 ft=EEPROM_read(24);
         val=ft;
		 LCDSendInt(val);
		 delay_ms(400);
		  
		 SEND_CMD(CLR_DISP); 
	     SEND_CMD(DISP_ON); 

		 

	 	
	if(B2==0)	
	  {	
                SEND_CMD(CLR_DISP); 
	            SEND_CMD(DISP_ON);
	            SEND_STR("Test");
                delay_ms(150); 	
	            
                    
		while(1)
		 {      
		 		        
				PORTA=0b01000000;
				delay_msc(x,z);
				

		 		while(lt<z)
				{        
						PORTA=0b00000000;
						delay_msc(d,z);
						
						if(lt>=z)
						{ PORTA=0b00000000;
						  SEND_CMD(CLR_DISP); 
				          return 0;
						}
						
						PORTA=0b01000000;
						delay_msc(c,z); 
	     				 
				
				}
				PORTA=0b00000000;
				SEND_CMD(CLR_DISP); 
				return 0;
				
			}				
		} 
		 
		 
	else	 
		 
	{	 while(1)
		 {
		 		hr=z;
				PORTA=0b01000000;
				delay_msb(x*60);
			 	
				while(ctr<z*3600)
				{
				         
						PORTA=0b00000000;
						delay_msb(d*60);
						
						if(ctr>=(z*3600))
						{ PORTA=0b00000000;
						  SEND_CMD(CLR_DISP); 
				          return 0;
						}
						
						PORTA=0b01000000;
						delay_msb(c*60);
	                     
	     				 			
				}
				PORTA=0b00000000;
				SEND_CMD(CLR_DISP); 
				return 0;
			}				
		 			
	  }  
		
		
		 } 
   
    	

}
