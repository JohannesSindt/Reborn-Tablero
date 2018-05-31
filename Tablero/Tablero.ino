#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include <mcp_can.h>
#include <SPI.h>

#define Pin_Tac_High 6
#define Pin_Tac_Low 7
#define Pin_RPM_High 8
const int SPI_CS_PIN = 9;

int SOC = 50;
int periodo_tac = (unsigned int)(49050/1000);
int periodo_rpm = (unsigned int)(24000/1000);
int vel_rpm = 0;
int counter1 = 0;
int counter2 = 0;
bool estadoTac = 0;
bool estadoRPM = 0;
unsigned char len = 0;
unsigned char buf[8];

MCP_CAN CAN(SPI_CS_PIN);    
void setup(void) {
    while (CAN_OK != CAN.begin(CAN_250KBPS))              // init can bus : baudrate = 250k
    {
        delay(100);
    }
  
  pinMode(Pin_Tac_High,OUTPUT);
  pinMode(Pin_Tac_Low,OUTPUT);
  pinMode(Pin_RPM_High,OUTPUT);
  
}

void loop(void) {
    if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
        unsigned long canId = CAN.getCanId();
        if (canId==0x1EF70300){
           vel_rpm=(int)((buf[5] + ((buf[6]&0x7F)<<8))-16384);
           if (vel_rpm<50 && vel_rpm>=-50){
            vel_rpm=50;
           }
           if (vel_rpm<-50){
            vel_rpm=-vel_rpm;
           }
           periodo_tac=(unsigned int)(49050/vel_rpm);
           periodo_rpm=(unsigned int)(24000/vel_rpm);
           
        }
        len = 0;
    }

    if (counter1>=periodo_tac){
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

    if (counter2>=periodo_rpm){
      if (estadoRPM==1){
        digitalWrite(Pin_RPM_High,HIGH);
        estadoRPM=0;
      }
      else{
        digitalWrite(Pin_RPM_High,LOW);
        estadoRPM=1;
      }
      counter2=0;
  }
  delayMicroseconds(90);
  counter1++;
  counter2++;
}



