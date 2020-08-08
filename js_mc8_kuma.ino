/*
 * RC adaptor
 *   emulation of Arduino HID controller
 *                         2019/09/27
 */

#include <PinChangeInterrupt.h>
#include <NewPing.h>
#include <Joystick.h>

// #define _DEBUG_

#ifndef _DEBUG_
  #include "U8glib.h"
  U8GLIB_SSD1306_128X32 u8g(U8G_I2C_OPT_NONE);  // I2C / TWI 
#endif

// https://github.com/NicoHood/PinChangeInterrupt/blob/master/Readme.md
// Arduino Micro: 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI)
//  0 D08 CH-B   steering 
//  1 D09 CH-C   throttle
//  2 D10 CH-G   increase/decrease max throttle
//  3 D11 CH-H   trigger mode

// Beetle D11, D10, D9, 15(SCK), 16(MO), 14(MI)
//  0 D11 CH-B   steering 
//  1 D10 CH-C   throttle
//  2 D09 CH-G   increase/decrease max throttle
//  3 D15 CH-H   trigger mode

#define NUM_BUTTON      4
#define BUTTON_MODE     0
#define R1              1
#define L1              2  

#define NUM_CHANNEL     7
#define BUTTON_PRESSED  1
#define BUTTON_RELEASED 0

#define MIN_VALUE16          -32767
#define MAX_VALUE16           32767
#define ERROR_RANGE           16

#define LED    13

const byte channel_pin[] = {8, 9, 10, 11,14,15,16};

Joystick_ Joystick = Joystick_(
  0x03,                    // reportid
  JOYSTICK_TYPE_JOYSTICK,  // type
  NUM_BUTTON,              // button count
  0,                       // hat switch count
  true, false, false,      // left x,y,z axis enable
  false, true, false,      // right x,y,z axis enable
  false,                   // rudder enable
  false,                   // throttle enable
  false,                   // accelerator enable
  false,                   // brake enable
  false                    // steering enable
);

#define PIN 4
#define MAX_DISTANCE 400
 
NewPing sonar(PIN,PIN, MAX_DISTANCE);


//  RC受信機チャンネルクラス
class RCRecieverChannel
{
    private:
    //  pin番号
    byte m_byChannelPin;
    //  ON期間
    //  Base
    unsigned long m_ulBasePeriod;
    //  ON/OFFタイミング
    volatile unsigned long m_st_time;
    volatile unsigned long m_ed_time;
    volatile unsigned long m_ulOnPeriod;

    public:
    void Init(byte byChannel);
    void OnTrigger();
    void InitBaseValue();
    void OutputTime();
    unsigned long GetOnPeriod(){  return m_ulOnPeriod;};

};

void RCRecieverChannel::Init(byte byChannel)
{
  m_byChannelPin = byChannel;
}
void RCRecieverChannel::InitBaseValue()
{
    m_ulBasePeriod = m_ulOnPeriod;
}

void RCRecieverChannel::OnTrigger()
{
  uint8_t trigger;

  trigger = getPinChangeInterruptTrigger(digitalPinToPCINT(m_byChannelPin));

  if(trigger == RISING)
  {
    m_st_time = micros();
  }
  else
  {
    m_ed_time = micros();  
    m_ulOnPeriod = m_ed_time - m_st_time;
  }

}

void RCRecieverChannel::OutputTime()
{
  char s[40];
  sprintf(s,"%u peri\n",m_ulOnPeriod);
  Serial.print(s);

}


RCRecieverChannel _aRCRevCh[NUM_CHANNEL];

void pinSelect(int nPin)
{
  _aRCRevCh[nPin].OnTrigger();
}

void OnTrigger_00(){  pinSelect(0);};
void OnTrigger_01(){  pinSelect(1);};
void OnTrigger_02(){  pinSelect(2);};
void OnTrigger_03(){  pinSelect(3);};
void OnTrigger_04(){  pinSelect(4);};
void OnTrigger_05(){  pinSelect(5);};
void OnTrigger_06(){  pinSelect(6);};

void setup()
{
    _aRCRevCh[0].Init(channel_pin[0]);
    _aRCRevCh[1].Init(channel_pin[1]);
    _aRCRevCh[2].Init(channel_pin[2]);
    _aRCRevCh[3].Init(channel_pin[3]);
    _aRCRevCh[4].Init(channel_pin[4]);
    _aRCRevCh[5].Init(channel_pin[5]);
    _aRCRevCh[6].Init(channel_pin[6]);
    
    //  ピンを入力モードにし、ピンの変化にコールバック関数を設定する
    pinMode(channel_pin[0], INPUT);    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(channel_pin[0]), OnTrigger_00, CHANGE);    
    pinMode(channel_pin[1], INPUT);    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(channel_pin[1]), OnTrigger_01, CHANGE);    
    pinMode(channel_pin[2], INPUT);    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(channel_pin[2]), OnTrigger_02, CHANGE);    
    pinMode(channel_pin[3], INPUT);    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(channel_pin[3]), OnTrigger_03, CHANGE);    
    pinMode(channel_pin[4], INPUT);    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(channel_pin[4]), OnTrigger_04, CHANGE);    
    pinMode(channel_pin[5], INPUT);    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(channel_pin[5]), OnTrigger_05, CHANGE);    
    pinMode(channel_pin[6], INPUT);    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(channel_pin[6]), OnTrigger_06, CHANGE);    
  
    // setup pin mode
    pinMode(LED, OUTPUT);

#ifndef _DEBUG_
    // Initialize Joystick
    Joystick.begin();
    Joystick.setXAxisRange(MIN_VALUE16, MAX_VALUE16);
    Joystick.setRyAxisRange(MIN_VALUE16, MAX_VALUE16);

    // assign default color value
    if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
      u8g.setColorIndex(255);     // white
    }
    else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
      u8g.setColorIndex(3);         // max intensity
    }
    else if ( u8g.getMode() == U8G_MODE_BW ) {
      u8g.setColorIndex(1);         // pixel on
    }
    else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
      u8g.setHiColorByRGB(255,255,255);
    }    

    Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
    
#else
    Serial.begin(115200);
#endif

    for(int i=0; i<10; i++)
    {
        delay(200);
        digitalWrite(LED, LOW);
        delay(200);
        digitalWrite(LED, HIGH);
    }

    // set base value 
    for(int i=0; i<NUM_CHANNEL; i++)
    {
        _aRCRevCh[i].InitBaseValue();
    }

}

void loop() {
  
  int steering_value;
  int slottle_value;
  int mode_value;

  steering_value = map(_aRCRevCh[0].GetOnPeriod(), 1012, 2008,MAX_VALUE16,MIN_VALUE16);
  slottle_value  = map(_aRCRevCh[1].GetOnPeriod(), 1012, 2008,MAX_VALUE16,MIN_VALUE16);
  mode_value  = map(_aRCRevCh[4].GetOnPeriod(), 1012, 2008,-10,10);

#ifndef _DEBUG_
  Joystick.setXAxis(steering_value);
  Joystick.setRyAxis(slottle_value);

  if(mode_value<0)
  {
    Joystick.pressButton(BUTTON_MODE);
  }else{
    Joystick.releaseButton(BUTTON_MODE);    
  }
  
//  Joystick.pressButton(BUTTON_MODE);
//  Joystick.pressButton(L1);
//  Joystick.pressButton(R1);
  
  char szData[50];

  int n;
  char byData;
  
  n = sonar.ping_cm();

  if(n<50)
  {
    byData = 'E';    
  }
  else if(n < 100)
  {
    byData = 'S';    
  }
  else if(n < 150)
  {
    byData = 'M';    
  }
  else if(n < 400)
  {
    byData = 'F';
  }

  
  Serial.print(byData);
  sprintf(szData,"%06d",n);
  
  sprintf(szData,"CH0 %06d,%06d",steering_value,_aRCRevCh[0].GetOnPeriod());
  sprintf(szData,"CH4 %06d,%06d",mode_value,_aRCRevCh[4].GetOnPeriod());
  
  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_unifont);
    
    sprintf(szData,"CH0 %06d,%05d",steering_value,_aRCRevCh[0].GetOnPeriod());
    u8g.drawStr( 0, 10, szData);

    sprintf(szData,"CH1 %06d,%05d",slottle_value,_aRCRevCh[1].GetOnPeriod());
    u8g.drawStr( 0, 21, szData);
    
    sprintf(szData,"CH4 %06d,%05d",mode_value,_aRCRevCh[4].GetOnPeriod());
    u8g.drawStr( 0, 32, szData);
    
  } while( u8g.nextPage() );
  
#else
  char szData[50];
  sprintf(szData,"%d,%d",mode_value,_aRCRevCh[2].GetOnPeriod());
  
  Serial.println(szData);
  delay(300);
  Serial.println(slottle_value);
#endif

  delay(10);
  
   
}
