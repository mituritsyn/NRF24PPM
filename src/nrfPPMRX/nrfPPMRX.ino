/*
  DIY Arduino based RC ppm receiver
  by Mikhail Turicyn, MITuritsyn@yandex.ru
  Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//////////////////////PPM CONFIGURATION//////////////////////////
#define channel_number 12 //we will have 12 channels
#define sigPin 2 //The PPM output will be pin D2
#define PPM_FrLen 27000 //frames per second of the PPM (1ms = 1000µs)
#define PPM_PulseLen 400 //pulse width config
//////////////////////////////////////////////////////////////////

int ppm[channel_number];

const uint64_t pipeIn = 0x1234567890LL; //EThis part of code should be the same as the transmitter

//RF24 radio(9, 10); //pin 10 is CSN!/
RF24 radio(10, 9);

struct MyData {
  byte throttle;
  byte pitch;
  byte roll;
  byte yaw;
  byte arm;
  byte aux1;
  byte aux2;
  byte aux3;
  byte aux4;
  byte aux5;
  byte aux6;
  byte aux7;
};

MyData data;

void resetData(){
  data.throttle = 0;
  data.yaw = 127;
  data.pitch = 127;
  data.roll = 127;
  data.arm = 1;
  data.aux1 = 127;
  data.aux2 = 127;
  data.aux3 = 127;
  data.aux4 = 127;
  data.aux5 = 127;
  data.aux6 = 127;
  data.aux7 = 127;
}

void setPPMValuesFromData(){
  ppm[0] = map(data.pitch, 0, 255, 1000, 2000);
  ppm[1] = map(data.roll, 0, 255, 1000, 2000);
  ppm[2] = map(data.throttle, 0, 255, 1000, 2000);
  ppm[3] = map(data.yaw, 0, 255, 1000, 2000);
  ppm[4] = map(data.arm, 1, 0, 1000, 2000);
  ppm[5] = 1500;
  ppm[6] = 1500;
  ppm[7] = 1500;
  ppm[8] = 1500;
  ppm[9] = 1500;
  ppm[10] = 1500;
  ppm[11] = 1500;
}

void setupPPM() {
  pinMode(sigPin, OUTPUT);
  digitalWrite(sigPin, 0);
  //interrupt setup
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = 100;
  TCCR1B |= (1 << WGM12); 
  TCCR1B |= (1 << CS11);
  TIMSK1 |= (1 << OCIE1A);
  sei();
}

void setup(){
  resetData();
  setupPPM();

  radio.begin();
  radio.setChannel(5);
  //radio.setAutoAck(false);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.openReadingPipe(1, 0x1234567890LL);
  radio.startListening();
}

unsigned long lastRecvTime, now = 0;

void recvData()
{
  while ( radio.available() ) {
    radio.read(&data, sizeof(MyData));
    lastRecvTime = millis();
  }
}

void loop()
{
  recvData();

  now = millis();
  if ( now - lastRecvTime > 500 ) {
    resetData();
  }
  setPPMValuesFromData();
}


////////////////////////////interrupt part don't touch it and don't take in mind/////////////////////////////////////////////


#define clockMultiplier 2

ISR(TIMER1_COMPA_vect) {
  static boolean state = true;

  TCNT1 = 0;

  if ( state ) {
    //end pulse
    PORTD = PORTD & ~B00000100; // turn off D2
    OCR1A = PPM_PulseLen * clockMultiplier;
    state = false;
  }
  else {
    //start pulse
    static byte cur_chan_numb;
    static unsigned int calc_rest;

    PORTD = PORTD | B00000100; // turn on D2
    state = true;

    if (cur_chan_numb >= channel_number) {
      cur_chan_numb = 0;
      calc_rest += PPM_PulseLen;
      OCR1A = (PPM_FrLen - calc_rest) * clockMultiplier;
      calc_rest = 0;
    }
    else {
      OCR1A = (ppm[cur_chan_numb] - PPM_PulseLen) * clockMultiplier;
      calc_rest += ppm[cur_chan_numb];
      cur_chan_numb++;
    }
  }
}
