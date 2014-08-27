#define NUM_PINS 16

byte which_pin[NUM_PINS] = {
  
  
  
  
  
  5,  6,  9, 10, 
  11, 12, 13,14,  
  
  4, 21, 20, 19,
  18,17,16,15
  
//  15, 16, 17,
//  18, 19, 20, 21
};

byte which_channel[NUM_PINS] = {
  0,1,2,3,
  4,5,6,7,
  8,9,10,11,
  12,13,14,15
};

byte which_note[NUM_PINS] = {
  36,36,36,36,
  36,36,36,36,
  36,36,36,36,
  36,36,36,36, 
};

byte state[NUM_PINS] = {0};

#define P_LED 3

//  plays a MIDI note.  Doesn't check to see that
//  cmd is greater than 127, or that data values are  less than 127:
void noteOn(int cmd, int pitch, int velocity) {
  Serial1.write(cmd);
  Serial1.write(pitch);
  Serial1.write(velocity);
}
unsigned long ledOffTime = 0;
void setup() {
  //  Set MIDI baud rate:
  Serial1.begin(31250);
  pinMode(P_LED, OUTPUT);
  digitalWrite(P_LED,LOW);
  
  for(int i = 0; i<NUM_PINS; ++i)
    pinMode(which_pin[i], INPUT_PULLUP);
    
}

void loop() {
  
  for(int i=0; i<NUM_PINS; ++i)
  {
    if(digitalRead(which_pin[i]) == LOW)
    {
      if(!state[i])
      {
        state[i] = 1;
        noteOn(0x90|which_channel[i], which_note[i], 0x7f);
        ledOffTime = millis() + 100;
      }
    }
    else
    {
      if(state[i])
      {
        state[i] = 0;
        noteOn(0x90|which_channel[i], which_note[i], 0x00);
      }
    }
  }
  
  if(ledOffTime > millis())
    digitalWrite(P_LED, HIGH);
  else
    digitalWrite(P_LED, LOW);
 
}


