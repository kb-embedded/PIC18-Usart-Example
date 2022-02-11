/*
 * File:   main.c
 * Author: Pooria.Norouzi
 *
 * Created on 14 Ocak 2022 Cuma, 16:55
 */


#include <xc.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define _XTAL_FREQ 8000000 //8MHZ 

//#define Red_LED 49
//#define Green_LED 50
//#define Blue_LED 51
//#define Red = LATDbits.LATD0
//#define Green = LATDbits.LATD1
//#define Blue = LATDbits.LATD2



//Abstract Interrupts declaration
void __interrupt(high_priority) ISR(void);
void __interrupt (low_priority) low_isr(void);

void UART_RX_INIT(void);    //UART Initialization
void UART_TX(uint8_t *s);   //TX Func for CHAR!
void usart_send_str(uint8_t s[]);   //TX Func for STRING!

//RX Buffers Declaration
uint8_t rx_buffer[100];
uint8_t UART_Buffer = 0;


int rx_index=0;     //Buffer index
bool rx_reg_ok = false; // Buffer reg permission


void main(void) {

    OSCCONbits.IRCF = 0X07; //8 MHz Osc Freq 
    OSCCONbits.SCS = 0X03; //Internal Osc Selection

    while(OSCCONbits.HFIOFS !=1);   //wait until osc freq stable

    RCONbits.IPEN = 1; //Enable Interrupt Priority
    INTCONbits.GIE = 1;  //Activate peripheral interrupts =ei(); 
    

    UART_RX_INIT();//Init UART

    TRISD = 0X00; //LED Output
    LATD = 0X00;    //LED Off

    while(1)
    {
        
    }

  
}

void UART_RX_INIT(){
    
    //Set Tris Reg for USART RX and TX
    TRISCbits.TRISC7 = 1; 
    TRISCbits.TRISC6 = 0;
    
    
    SPBRG = 51;//9600 Baudrate
    
    RCSTAbits.CREN = 1; //Enable Cont. Rec.
    RCSTAbits.SPEN = 1; //Serial Port Enable
    BAUDCON1bits.BRG16 = 0; //8Bit
    
    TXSTA1bits.SYNC = 0;    //Asynchronous mode
    TXSTA1bits.BRGH = 1;    //High speed 
    TXSTA1bits.TXEN = 1;    //Transmit enabled   
    
    IPR1bits.RCIP = 1;      //Receiver Interrupt set HIGH
    PIE1bits.RCIE = 1;      //Receive interrupt enabled
    
    IPR1bits.TXIP = 0;      //Transmit Interrupts set LOW
    PIE1bits.TXIE = 1;      //Transmit interrupt enabled
}


void __interrupt(high_priority) ISR(void){
    if(PIR1bits.RC1IF==1){//Check for RCREG full?
        UART_Buffer = RCREG1; //Copy data from RCREG
        
        if(UART_Buffer == 36 && !rx_reg_ok)//When '$' char comes 
        {
            memset(rx_buffer, 0, sizeof(rx_buffer)); //Clear Buffer
            rx_buffer[0] = UART_Buffer;              //Add first index '$'
            rx_reg_ok=true;                          //Adding to buffer                        
        }
        else if(UART_Buffer == 37 && rx_reg_ok  ) //when '%' char comes
        {
            rx_index++; 
            rx_buffer[rx_index] = UART_Buffer; //Add '%' char last index of buffer
            usart_send_str(rx_buffer);         //Sending all buffer
            rx_reg_ok=false;                   //Ending Buffer
            rx_index=0;
        }
        else if(rx_reg_ok && UART_Buffer != 36) //When char is not '$' and Register permission ok
        {
            rx_index++;
            rx_buffer[rx_index] = UART_Buffer;//Add char to buffer
            usart_send_str(rx_buffer);
            usart_send_str("\r\n-----");//Print
            
        }
        else
        {
            usart_send_str("\r\nNot Registered\r\n");
        }

        PIR1bits.RC1IF = 0;
        
    }
}

//Detailed setup
//void __interrupt(low_priority) low_isr(void){
//    INTCONbits.GIEH = 0;
//    if(PIR1bits.TXIF){//Check for the TXREG full
//        PIR1bits.TXIF=0;   //Clear 
//    }
//    INTCONbits.GIEH = 1;
//}

//--------TRANSMIT CHAR--------
void UART_TX(uint8_t *s){
    //while(TXSTA1bits.TRMT==1);
    TXREG = *s;//Add to pointer
    while(TXSTA1bits.TRMT==0);
    
}
//--------TRANSMIT STRING--------
void usart_send_str(uint8_t s[]){

    int i=0;
    while(s[i]!='\0')//wait until end last char
    {
        UART_TX(&s[i]);
        i++;
    }
}