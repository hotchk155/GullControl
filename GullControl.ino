#define NUM_PINS 16
/*
// Array maps physical input pins to 
// logical inputs 1-16
byte which_pin[NUM_PINS] = {
  5,  6,  9, 10, 
  11, 12, 13,14,    
  4, 21, 20, 19,
  18,17,16,15
};

// Array maps logical inputs 1-16 to
// MIDI channels
byte which_channel[NUM_PINS] = {
  0,1,2,3,
  4,5,6,7,
  8,9,10,11,
  12,13,14,15
};

// Array maps logical inputs 1-16 to
// MIDI notes
byte which_note[NUM_PINS] = {
  36,36,36,36,
  36,36,36,36,
  36,36,36,36,
  36,36,36,36, 
};
*/
unsigned long ledOffTime = 0;

class CInputPort 
{
  public:
    int pin;
    int channel;
    int note;
    byte state;
    
    CInputPort(int p, int c, int n) : pin(p), channel(c), note(n) {
      pinMode(this->pin, INPUT_PULLUP);
      this->state = 0;
    }
    
    void run(unsigned long milliseconds) {    
      if(digitalRead(this->pin) == LOW)
      {
        if(!this->state)
        {
          this->state = 1;
          noteOn(this->channel, this->note, 127);
          ledOffTime = milliseconds + 100;
        }
      }
      else
      {
        if(this->state)
        {
          this->state = 0;
          noteOff(this->channel, this->note);
        }
      }
    }    
};


CInputPort InputPort[NUM_PINS] = {
  CInputPort(  5,  0,  60 ),
  CInputPort(  6,  1,  60 ),
  CInputPort(  9,  2,  60 ),
  CInputPort( 10,  3,  60 ),
  CInputPort( 11,  4,  60 ),
  CInputPort( 12,  5,  60 ),
  CInputPort( 13,  6,  60 ),
  CInputPort( 14,  7,  60 ),
  CInputPort(  3,  8,  60 ),
  CInputPort( 21,  9,  60 ),
  CInputPort( 20, 10,  60 ),
  CInputPort( 19, 11,  60 ),
  CInputPort( 18, 12,  60 ),
  CInputPort( 17, 13,  60 ),
  CInputPort( 16, 14,  60 ),
  CInputPort( 15, 15,  60 )
};

// State of a given pin
//byte state[NUM_PINS] = {0};

#define P_LED 4

void noteOn(int channel, int note, int velocity) {  
  Serial1.write(0x90|channel);
  Serial1.write(note);
  Serial1.write(velocity);
  usbMIDI.sendNoteOn(note, velocity, channel);
}

void noteOff(int channel, int note) {
  Serial1.write(0x90|channel);
  Serial1.write(note);
  Serial1.write((byte)0x00);
  usbMIDI.sendNoteOff(note, 0x00, channel);
}

void setup() {
  // Set MIDI baud rate on both 
  // USB serial and hardware serial
  Serial1.begin(31250);
  pinMode(P_LED, OUTPUT);
  digitalWrite(P_LED,LOW);
}

void loop() {
  
  unsigned long milliseconds = millis();
  for(int i=0; i<NUM_PINS; ++i)
  {
    CInputPort& port = InputPort[i];
    port.run(milliseconds);
  }
  
  if(ledOffTime > millis())
    digitalWrite(P_LED, HIGH);
  else
    digitalWrite(P_LED, LOW); 
}


