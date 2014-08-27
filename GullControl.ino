#define NUM_PINS 16
unsigned long ledOffTime = 0;

void noteOn(int channel, int note, int velocity) {  
  Serial1.write(0x90|channel);
  Serial1.write(note);
  Serial1.write(velocity);
  usbMIDI.sendNoteOn(note, velocity, channel+1);
}
void noteOff(int channel, int note) {
  Serial1.write(0x90|channel);
  Serial1.write(note);
  Serial1.write((byte)0x00);
  usbMIDI.sendNoteOff(note, 0x00, channel+1);
}
void controlChange(int channel, int controller, int value) {
  Serial1.write(0xB0|channel);
  Serial1.write((byte)controller);
  Serial1.write((byte)value);
  usbMIDI.sendControlChange(controller, value, channel);
}

class CContinuousController
{
  public:
    enum {
       ST_IDLE,
       ST_RISE,
       ST_FALL
    };
    enum {
      MODE_LINEAR,
      MODE_LOG,
    };
    byte channel;
    byte controller;
    byte value;
    byte state;
    byte mode;
    unsigned long step_delay;
    unsigned long next_step;

    CContinuousController(byte m, byte ch, byte cc, unsigned long t) {
        this->channel = ch;
        this->controller = cc;
        this->step_delay = t;
        this->value = 0;
        this->state = ST_FALL; // ensure controller is set to 0
        this->next_step = 1;
        this->mode = m;
    }    
    void run(unsigned long milliseconds)
    {
      if(this->next_step)
      {
        if(milliseconds >= this->next_step)
        {
          if(this->state == ST_RISE)
          {
            if(this->value < 127)
            {
              if(this->mode == MODE_LOG) {
                this->value += (127 - this->value) / 2;
                if(this->value > 125)
                  this->value = 127;
              }                
              else {
                ++this->value;
              }
              controlChange(this->channel, this->controller, this->value);
              this->next_step = milliseconds + this->step_delay;
            }
            else
            {
              this->value = 127;
              this->state = ST_IDLE;
              this->next_step = 0;
            }
          }
          else if(this->state == ST_FALL)
          {
            if(this->value > 0)
            {
              if(this->mode == MODE_LOG) {
                this->value -= this->value / 2;
                if(this->value < 2)
                  this->value = 0;
              }                
              else {
                --this->value;
              }
              controlChange(this->channel, this->controller, this->value);
              this->next_step = milliseconds + this->step_delay;
            }
            else
            {
              this->value = 0;
              this->state = ST_IDLE;
              this->next_step = 0;
            }
          }
          else
          {
            this->next_step = 0;
          }
        }
      }
    }
    void rise()
    {
      this->state = ST_RISE;
      this->next_step = 1;
    }
    void fall()
    {
      this->state = ST_FALL;
      this->next_step = 1;
    }
};

class CInputPort 
{
  public:
    int pin;
    int channel;
    int note;
    byte state;
    CContinuousController SlowPress;
    CContinuousController LogPress;
    
    CInputPort(int p, int c, int n) : 
      SlowPress(CContinuousController::MODE_LINEAR, c, 71, 50),
      LogPress(CContinuousController::MODE_LOG, c, 72, 50) {
      this->pin = p;
      this->channel = c;
      this->note = n;
      this->state = 0;
      pinMode(p, INPUT_PULLUP);
    }
    
    void run(unsigned long milliseconds) {    
      if(digitalRead(this->pin) == LOW)
      {
        if(!this->state)
        {
          this->state = 1;
          SlowPress.rise();
          LogPress.rise();
          noteOn(this->channel, this->note, 127);
          ledOffTime = milliseconds + 100;
        }
      }
      else
      {
        if(this->state)
        {
          this->state = 0;
          SlowPress.fall();
          LogPress.fall();
          noteOff(this->channel, this->note);
        }
      }
      SlowPress.run(milliseconds);
      LogPress.run(milliseconds);
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


