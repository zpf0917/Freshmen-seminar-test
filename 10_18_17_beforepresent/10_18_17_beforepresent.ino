
// Super-basic UNTZtrument MIDI example.  Maps buttons to MIDI note
// on/off events; this is NOT a sequencer or anything fancy.
// Requires an Arduino Leonardo w/TeeOnArdu config (or a PJRC Teensy),
// software on host computer for synth or to route to other devices.

#include <Wire.h>
#include <Adafruit_Trellis.h>
#include <Adafruit_UNTZtrument.h>
#include "MIDIUSB.h"

#define LED     13 // Pin for heartbeat LED (shows code is working)
#define CHANNEL 1  // MIDI channel number
#define DRUMCHANNEL 2
#define BUTTON_BEFORE_KEYBOARD 0
#define LOWDRUM 36

#define HELLA

#ifndef HELLA
// A standard UNTZtrument has four Trellises in a 2x2 arrangement
// (8x8 buttons total).  addr[] is the I2C address of the upper left,
// upper right, lower left and lower right matrices, respectively,
// assuming an upright orientation, i.e. labels on board are in the
// normal reading direction.
Adafruit_Trellis     T[4];
Adafruit_UNTZtrument untztrument(&T[0], &T[1], &T[2], &T[3]);
const uint8_t        addr[] = { 0x70, 0x71,
                                0x72, 0x73 };
#else
// A HELLA UNTZtrument has eight Trellis boards...
Adafruit_Trellis     T[8];
Adafruit_UNTZtrument untztrument(&T[0], &T[1], &T[2], &T[3],
                                 &T[4], &T[5], &T[6], &T[7]);
const uint8_t        addr[] = { 0x70, 0x71, 0x72, 0x73,
                                0x74, 0x75, 0x76, 0x77 };
#endif // HELLA

// For this example, MIDI note numbers are simply centered based on
// the number of Trellis buttons; each row doesn't necessarily
// correspond to an octave or anything.
#define WIDTH     ((sizeof(T) / sizeof(T[0])) * 2)
#define WIDTH_NEW 12
#define N_BUTTONS ((sizeof(T) / sizeof(T[0])) * 16)
#define LOWNOTE   12      //((128 - N_BUTTONS) / 2)

uint8_t       heart        = 0;  // Heartbeat LED counter
unsigned long prevReadTime = 0L; // Keypad polling timer








int Octave[7][12]={
  //{0, 1, 2, 3, 4, 5, 6, 7, 8, 9 ,10 ,11},
  {12, 13 , 14,  15,  16,  17,  18,  19,  20 , 21 , 22 , 23},
  {24 , 25,  26 , 27 , 28 , 29 , 30 , 31 , 32 , 33 , 34 , 35},
  {36 ,  37  ,38 , 39 , 40 , 41,  42,  43,  44 , 45 , 46 , 47},
  {48 ,  49 , 50 , 51 , 52 , 53 , 54 , 55 , 56 , 57 , 58 , 59},
  {60 ,  61 , 62 , 63 , 64 , 65  ,66  ,67 , 68 , 69 , 70 , 71},
  {72 ,  73 , 74 , 75 , 76 , 77 , 78 , 79 , 80 , 81 , 82 , 83},
  {84 ,  85 , 86  ,87 , 88 , 89 , 90 , 91 , 92 , 93 , 94 , 95} //,
  //{96 ,  97 , 98  ,99 , 100 ,  101  , 102 ,  103 ,  104  , 105 ,  106 ,  107},
  //{107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118}
};



int scale_number = 7;
int Blues_scale[7] = {0, 2, 4 ,5 ,7, 9, 11}; 

// C major
// backing track: https://www.youtube.com/watch?v=ih01x2kpXHY

// int Blues_scale[6] = {1, 4, 6, 8, 9, 11}; 
// A major

//int Blues_scale[6] = {0, 3, 5, 6, 7, 10};
// C blues





//–––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––                       Metro
//––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––

unsigned long prevMetroTime = 0L;
unsigned long BPM = 500L;
int metroNowOn = 0L;
uint8_t timeSig = 4;

//–––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––                       Metro
//–––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––






void setup() {

  Serial.begin(9600);
  Serial.println("Trellis Demo");
  
  pinMode(LED, OUTPUT);
#ifndef HELLA
  untztrument.begin(addr[0], addr[1], addr[2], addr[3]);
#else
  untztrument.begin(addr[0], addr[1], addr[2], addr[3],
                    addr[4], addr[5], addr[6], addr[7]);
#endif // HELLA
  // Default Arduino I2C speed is 100 KHz, but the HT16K33 supports
  // 400 KHz.  We can force this for faster read & refresh, but may
  // break compatibility with other I2C devices...so be prepared to
  // comment this out, or save & restore value as needed.
#ifdef ARDUINO_ARCH_SAMD
  Wire.setClock(400000L);
#endif
#ifdef __AVR__
  TWBR = 12; // 400 KHz I2C on 16 MHz AVR
#endif
  untztrument.clear();

//–––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––                       Metro
//––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
  untztrument.setLED((untztrument.xy2i(metroNowOn,0)));
  prevMetroTime = millis();
//–––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––                       Metro
//––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––  
  
  untztrument.writeDisplay();

 int state = 0; 

  for (int i = 0; i < 128; i++)
  {
    state = 0;
    uint8_t a, b;
    untztrument.i2xy(i, &a, &b);
    for (int j = 0; j < scale_number; j++)
    {
      if (a == Blues_scale[j])
      {
        state = 1;
      }      
    }

    if (state == 1 && b > 0)
    {
      untztrument.setLED(i);
    }
        
  }
  
}








void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity}; // 1001 0000
  MidiUSB.sendMIDI(noteOn);
  MidiUSB.flush();
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};  // 1000 0000
  MidiUSB.sendMIDI(noteOff);
  MidiUSB.flush();
}


/////////////////////////////                    ALL END NOT WORKING
//////////////////////////////////////////////////////////////////////////////////////
void noteAllOff(byte channel) {
  midiEventPacket_t noteAllOff = {0x0B, 0xB0 | channel, 123, 0};
  MidiUSB.sendMIDI(noteAllOff);
  MidiUSB.flush();
}
/////////////////////////////                    DRUM END NOT WORKING
//////////////////////////////////////////////////////////////////////////////////////
///////https://www.midi.org/specifications/item/table-1-summary-of-midi-message
//////






void loop() {
  unsigned long t = millis();
  if((t - prevReadTime) >= 20L) { // 20ms = min Trellis poll time

//–––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––                       Metro
//––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––

if((t-prevMetroTime) > BPM)
  {
    Serial.println(metroNowOn);
    if(metroNowOn < timeSig - 1)
    {
      untztrument.clrLED(untztrument.xy2i(metroNowOn, 0));
      metroNowOn++;
    }
    else if(metroNowOn == timeSig - 1) 
    {
      untztrument.clrLED(untztrument.xy2i(metroNowOn, 0));
      metroNowOn = 0;
    }
    else
    {
      metroNowOn = 0; 
    }

    uint8_t l = untztrument.xy2i(metroNowOn, 0);
    untztrument.setLED(l);
    prevMetroTime = t;
    untztrument.writeDisplay();
  }
//–––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––                       Metro
//––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––

    
    if(untztrument.readSwitches()) { // Button state change?

      
      for(uint8_t i=0; i<N_BUTTONS; i++) { // For each button...
        // Get column/row for button, convert to MIDI note number    
        uint8_t x, y, note;
        untztrument.i2xy(i, &x, &y);
        note = LOWNOTE + y * WIDTH_NEW + x - BUTTON_BEFORE_KEYBOARD;



/////////////////////////////                    ALL OFF NOT WORKING
//////////////////////////////////////////////////////////////////////////////////////
        if(untztrument.justPressed(55)) 
        {
          
          for (int close_i = 0; close_i < 16; close_i++)
          {
            
              noteAllOff(close_i);
          }
          untztrument.setLED(55);
          untztrument.writeDisplay();
          delay(1000);
          untztrument.clrLED(55);
          untztrument.writeDisplay();
          return;    
          
        } 

/////////////////////////////                    All OFF
//////////////////////////////////////////////////////////////////////////////////////


        if(x<12 && y>0){
        
          if(untztrument.justPressed(i)) {
            noteOn(CHANNEL, note, 127);
            Serial.print(note);         
            Serial.println("   on");
            //untztrument.setLED(i);
          } 
          else if(untztrument.justReleased(i)) {
            noteOff(CHANNEL, note, 0);
            Serial.print(note);            
            Serial.println("   off");
            //untztrument.clrLED(i);
          }
        }  
        
      }




/////////////////////////////                    DRUM START
//////////////////////////////////////////////////////////////////////////////////////

      for(uint8_t y=5; y<8; y++) {
        for(uint8_t x=12; x<16; x++){// For each button...
          // Get column/row for button, convert to MIDI note number
          uint8_t drum_note;
          drum_note = LOWDRUM + (7-y) * 4 + (x - 12);
          uint8_t k = untztrument.xy2i(x, y);
          if(untztrument.justPressed(k)) {
            noteOn(DRUMCHANNEL, drum_note, 127);
            untztrument.setLED(k);
          } else if(untztrument.justReleased(k)) {
            noteOff(DRUMCHANNEL, drum_note, 0);
            untztrument.clrLED(k);
          }
        }
      }
      
/////////////////////////////                    DRUM END
//////////////////////////////////////////////////////////////////////////////////////


  
      untztrument.writeDisplay();
    }
    prevReadTime = t;
    digitalWrite(LED, ++heart & 32); // Blink = alive
  }
}
