/*
 * RC adaptor
 *   emulation of Arduino HID controller
 *                         2019/09/27
 */

#include <Arduino.h>
#include <PinChangeInterrupt.h>
#include <NewPing.h>
#include <Joystick.h>

#include "U8glib.h"
U8GLIB_SSD1306_128X32 u8g(U8G_I2C_OPT_NONE);  // I2C / TWI 

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
#define BUTTON_EMG      1
#define BUTTON_ERASE    2

#define R1              1
#define L1              2  

#define NUM_CHANNEL     7
#define BUTTON_PRESSED  1
#define BUTTON_RELEASED 0

#define MIN_VALUE16          -32767
#define MAX_VALUE16           32767
#define MIN_PWM               1012
#define MAX_PWM               2008
#define ERROR_RANGE           16

#define LED    13

enum T_KeyType
{
  ANALOG_IN = 0,
  KEY2STATE_IN = 1,
  KEY3STATE_IN = 2,
};

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

    long m_constain_value;
    long m_value;

    T_KeyType m_keytype;

    public:
    void Init(byte byChannel);
    void OnTrigger();
    void InitBaseValue();
    void OutputTime();
    void CalcPWM();
    void SetKeyType(T_KeyType keytype){ m_keytype = keytype;};
    unsigned long GetOnPeriod(){  return m_ulOnPeriod;};
    long GetValue(){ return m_value;};
    long GetConstainValue(){return m_constain_value;};

};

void RCRecieverChannel::CalcPWM()
{
  long tmp;

  switch(m_keytype)
  {
    case ANALOG_IN:
      m_constain_value = m_ulOnPeriod;
      if(m_ulOnPeriod >= MAX_PWM) m_constain_value = MAX_PWM;
      if(m_ulOnPeriod <= MIN_PWM) m_constain_value = MIN_PWM;

      m_value = map(m_constain_value, MIN_PWM, MAX_PWM,MAX_VALUE16,MIN_VALUE16);
      break;

    case KEY2STATE_IN:
      m_constain_value = m_ulOnPeriod;
      if(m_ulOnPeriod >= MAX_PWM) m_constain_value = MAX_PWM;
      if(m_ulOnPeriod <= MIN_PWM) m_constain_value = MIN_PWM;
      
      //  1/0で数値を取りたいが、PWMが検出失敗したときに中間の1500を検出すると、ONを検出して暴走するので、
      //  3ステートで検出し、中間値を検出したら0にセットする。
      tmp = map(m_constain_value, MIN_PWM, MAX_PWM, 2, 0);

      switch(tmp)
      {
        case 2: m_value = 1; break;
        case 1: m_value = 0; break;
        case 0: m_value = 0; break;
        default:m_value = 0; break;
      }
      break;

    case KEY3STATE_IN:
      m_constain_value = m_ulOnPeriod;
      if(m_ulOnPeriod >= MAX_PWM) m_constain_value = MAX_PWM;
      if(m_ulOnPeriod <= MIN_PWM) m_constain_value = MIN_PWM;

      m_value = map(m_constain_value, MIN_PWM, MAX_PWM, 2, 0);
      break;
  }
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

    _aRCRevCh[0].SetKeyType(ANALOG_IN);   //スロットル
    _aRCRevCh[1].SetKeyType(ANALOG_IN);   //ステアリン
    _aRCRevCh[2].SetKeyType(KEY2STATE_IN);   //非常停止
    _aRCRevCh[3].SetKeyType(KEY2STATE_IN);   //モード切替
    _aRCRevCh[4].SetKeyType(KEY3STATE_IN);   //ｎレコード消去
    _aRCRevCh[5].SetKeyType(KEY3STATE_IN);   //予備
    _aRCRevCh[6].SetKeyType(KEY3STATE_IN);   //予備

    
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

    // Initialize Joystick
    Joystick.begin(false);
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
  int emg_value;
  int erase_value;

  for(int i=0; i < NUM_CHANNEL; i++)
  {
    _aRCRevCh[i].CalcPWM();
  }

  slottle_value  = _aRCRevCh[0].GetValue();
  steering_value = _aRCRevCh[1].GetValue();
  emg_value      = _aRCRevCh[2].GetValue();
  mode_value     = _aRCRevCh[3].GetValue();
  erase_value    = _aRCRevCh[4].GetValue();

  Joystick.setXAxis(steering_value);
  Joystick.setRyAxis(slottle_value);
  //  非常停止
  if(emg_value==1)
  {
    Joystick.pressButton(BUTTON_EMG);
  }else{
    Joystick.releaseButton(BUTTON_EMG);    
  }

  //  モード切替
  if(mode_value==0)
  {
//    if(mode_value != nBack)
    {
      Joystick.pressButton(BUTTON_MODE);
    }
  }else{
//    if(mode_value != nBack)
    {
      Joystick.releaseButton(BUTTON_MODE);
    }
  }

  //  nレコード消去
  if(erase_value==1)
  {
    Joystick.pressButton(BUTTON_ERASE);
  }else{
    Joystick.releaseButton(BUTTON_ERASE);    
  }
  
//  Joystick.pressButton(BUTTON_MODE);
//  Joystick.pressButton(L1);
//  Joystick.pressButton(R1);
  Joystick.sendState();
  
  char szData1[50];
  char szData2[50];
  char szData3[50];

  int n;
  char byData;
    

  long ch1;
  long ch2;
  long ch3;
  long ch4;
  long ch5;
  long ch6 = 0;

  //USBに出力するデータを表示する
  ch1 = _aRCRevCh[0].GetValue();
  ch2 = _aRCRevCh[1].GetValue();
  ch3 = _aRCRevCh[2].GetValue();
  ch4 = _aRCRevCh[3].GetValue();
  ch5 = _aRCRevCh[4].GetValue();

  #if 1
  sprintf(szData1,"CH1 %04d", ch1 );
  sprintf(szData2,"CH2 %04d", ch2 );
  sprintf(szData3,"CH3 %04d", ch3 );
  #else
  sprintf(szData1,"convalue %04u", _aRCRevCh[4].GetConstainValue() );
  sprintf(szData2,"value %04d", _aRCRevCh[4].GetValue() );  
  sprintf(szData3,"period %04u", _aRCRevCh[4].GetOnPeriod());
  #endif

  static int _nCnt = 0;

  if(_nCnt > 200)
  {
    _nCnt = 0;
    u8g.setFont(u8g_font_unifont);
    u8g.firstPage();  
    do {
      u8g.drawStr( 0, 10, szData1);
      u8g.drawStr( 0, 21, szData2);
      u8g.drawStr( 0, 32, szData3);
      
    } while( u8g.nextPage() );
  }else{
    _nCnt ++;
  }
}
