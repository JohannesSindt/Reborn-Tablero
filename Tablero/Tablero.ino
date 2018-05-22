/**************************************************************************/
/*! 
    @file     trianglewave.pde
    @author   Adafruit Industries
    @license  BSD (see license.txt)

    This example will generate a triangle wave with the MCP4725 DAC.   

    This is an example sketch for the Adafruit MCP4725 breakout board
    ----> http://www.adafruit.com/products/935
 
    Adafruit invests time and resources providing this open source code, 
    please support Adafruit and open-source hardware by purchasing 
    products from Adafruit!
*/
/**************************************************************************/
#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include <mcp_can.h>
#include <SPI.h>

#define Pin_Tac_High 5
#define Pin_Tac_Low 6
#define Pin_RPM_High 7
#define Pin_RPM_Low 8

const int SPI_CS_PIN = 9;

int SOC = 50;
int periodo_tac = 1;
int periodo_rpm = 1;
int vel_rpm = 500;
int counter1 = 0;
int counter2 = 0;
bool estadoTac = 0;
bool estadoRPM = 0;

MCP_CAN CAN(SPI_CS_PIN);    

Adafruit_MCP4725 dac;

void setup(void) {
    while (CAN_OK != CAN.begin(CAN_250KBPS))              // init can bus : baudrate = 500k
    {
        delay(100);
    }

  pinMode(Pin_Tac_High,OUTPUT);
  pinMode(Pin_Tac_Low,OUTPUT);
  pinMode(Pin_RPM_High,OUTPUT);
  pinMode(Pin_RPM_Low,OUTPUT);

  // For Adafruit MCP4725A1 the address is 0x62 (default) or 0x63 (ADDR pin tied to VCC)
  // For MCP4725A0 the address is 0x60 or 0x61
  // For MCP4725A2 the address is 0x64 or 0x65
  dac.begin(0x60);
  dac.setVoltage(3000,false);
  

  
  //set timer0 interrupt at 10kHz
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 10khz increments
  OCR0A = 24;// = (16*10^6) / (10000*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR0B |= (1 << CS01) | (1 << CS00);   
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);

  sei();
}

void loop(void) {
    unsigned char len = 0;
    unsigned char buf[8];
    if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
        unsigned int canId = CAN.getCanId();

        if (canId==0x1EF70300){
           vel_rpm=buf[6] + buf[7]<<8;
           periodo_tac=(int)97725/vel_rpm;//2*pi*0.522/6*3.6*3.12/60/10000)
        }
        if (canId==0x1EF72000){
          SOC=buf[4] + buf[5]<<8;
          dac.setVoltage(SOC*4095/100,false);
        }
    }
}

ISR(TIMER0_COMPA_vect){//timer0 interrupt 10kHz
  if (counter1==periodo_tac){
    if (estadoTac==1){
      digitalWrite(Pin_Tac_High,HIGH);
      digitalWrite(Pin_Tac_Low,LOW);
      estadoTac=0;
    }
    else{
      digitalWrite(Pin_Tac_High,LOW);
      digitalWrite(Pin_Tac_Low,HIGH);
      estadoTac=1;
    }
    counter1=0;
  }

    if (counter2==periodo_rpm){
      if (estadoRPM==1){
        digitalWrite(Pin_RPM_High,HIGH);
        digitalWrite(Pin_RPM_Low,LOW);
        estadoRPM=0;
      }
      else{
        digitalWrite(Pin_RPM_High,LOW);
        digitalWrite(Pin_RPM_Low,HIGH);
        estadoRPM=1;
      }
      counter2=0;
  }
  
  counter1++;
  counter2++;
}
