#define LED 13
#define FOOTSWITCH 12
#define TOGGLE 2
#define PUSHBUTTON_1 A5
#define PUSHBUTTON_2 A4
 
// PWM Global Settings
#define PWM_FREQ 0x00FF // PWM freq - 31.3 кГц
#define PWM_MODE 0      // Fast (1) или Phase Correct (0)
#define PWM_QTY 2       // 2 PWMS in parallel
 
void setup() {
  pinMode(FOOTSWITCH, INPUT_PULLUP);
  pinMode(TOGGLE, INPUT_PULLUP);
  pinMode(PUSHBUTTON_1, INPUT_PULLUP);
  pinMode(PUSHBUTTON_2, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  
  // ADC Settings
  ADMUX = 0x60;  // lefthand bit order, adc0, inner vcc
  ADCSRA = 0xe5; // Enable ADC, ck/32, automode
  ADCSRB = 0x07; // Timer 1 ISR on
  DIDR0 = 0x01;  // Disable digital ports adc0
 
   // PWM Settings
  TCCR1A = (((PWM_QTY - 1) << 5) | 0x80 | (PWM_MODE << 1));
  TCCR1B = ((PWM_MODE << 3) | 0x11);
  TIMSK1 = 0x20; 
  ICR1H = (PWM_FREQ >> 8);
  ICR1L = (PWM_FREQ & 0xff);
  DDRB |= ((PWM_QTY << 1) | 0x02); // enable PWM outputs
  sei();
}
 
void loop()
{
  // LED on if effect is on
  if (digitalRead(FOOTSWITCH)) 
    digitalWrite(LED, HIGH);
  else  
    digitalWrite(LED, LOW);
}

int input, vol_variable = 512;
int counter = 0;
unsigned int ADC_low, ADC_high;

ISR(TIMER1_CAPT_vect) { // ISR Timer 1
  // Reading data from ADC
  ADC_low = ADCL;
  ADC_high = ADCH;
  // Combine in one
  input = ((ADC_high << 8) | ADC_low) + 0x8000; // to 16 signed bits
 
  // Check buttons
  counter++; 
  if(counter == 100) { // To save resources - check one time in a 100
    counter=0;
    if (!digitalRead(PUSHBUTTON_1)) 
    {
      if (vol_variable<1024) 
        vol_variable=vol_variable+1; // increase volume
    }
    if (!digitalRead(PUSHBUTTON_2)) 
    {
      if (vol_variable>0) 
        vol_variable=vol_variable-1; // decrease volume
    }
  }
 
  input = map(input, 0, 1024, 0, vol_variable);
 
  // Write PWM signal
  OCR1AL = ((input + 0x8000) >> 8);
  OCR1BL = input;
}