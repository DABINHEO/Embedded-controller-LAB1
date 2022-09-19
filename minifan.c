#include "STM32TimerInterrupt.h"
#include "STM32_ISR_Timer.h"

// State definition
#define S0    0   // Fan OFF
#define S1    1   // Fan vel = 50%
#define S2    2   // Fan vel = 100%

#define HW_TIMER_INTERVAL_MS  100
#define TIMER_INTERVAL        1000L

// Address number of output in array
#define PWM 0
#define LED 1

STM32Timer ITimer(TIM1);    // Init STM32 timer TIM1
STM32_ISR_Timer ISR_Timer;  // Init STM32_ISR_Timer

// State table definition
typedef struct {
  uint32_t out[4][2];      // output = FSM[state].out[PWM or LED]
  uint32_t next[2];  // nextstate = FSM[state].next[input X][input Y]
} State_t;

State_t FSM[3] = {
  { { {0, LOW  }, {0  , LOW  }, {0, HIGH }, {50 , HIGH} }, {S0, S1} },
  { { {0, HIGH }, {50 , HIGH }, {0, HIGH }, {100, HIGH} }, {S1, S2} },
  { { {0, HIGH }, {100, HIGH }, {0, LOW  }, {0  , LOW } }, {S2, S0} },
};

// Pin setting
const int ledPin = 13;
const int pwmPin = 11;
const int btnPin = 3;
const int trigPin = 10;
const int echoPin = 7;

unsigned char state = S0;
unsigned char input[2] = {0, 0};
unsigned char pwmOut = 0;
unsigned char ledOut = LOW;

unsigned long duration;
float distance;
int thresh = 5;

void setup() {  
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);

  // Initialize pwm pin as an output:
  pinMode(pwmPin, OUTPUT);
  
  // initialize the pushbutton pin as an interrupt input:
  pinMode(btnPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(btnPin), pressed, FALLING);

  // Initialize the trigger pin as an output
  pinMode(trigPin, OUTPUT);

  // Initialize the echo pin as an input
  pinMode(echoPin, INPUT);

  ITimer.attachInterruptInterval(HW_TIMER_INTERVAL_MS * 1000, TimerHandler);
  ISR_Timer.setInterval(TIMER_INTERVAL, timerInterrupt);
  
  Serial.begin(9600);
}

void loop() {
  // Generate pwm singal on the trigger pin.
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(10);

  // Distance is calculated using how much time it takes.
  duration = pulseIn(echoPin, HIGH);
  distance = (float)duration / 58.0;

  // Calculate next state. then update State
  nextState();

  // Output of states
  pwmOut = FSM[state].out[ input[0] * 2 + input[1] ][0];
  ledOut = FSM[state].out[ input[0] * 2 + input[1] ][1];
  analogWrite(pwmPin, pwmOut);
  digitalWrite(ledPin, ledOut);

  Serial.print("distance = ");
  Serial.print(distance);
  Serial.println(" [cm]");
  Serial.println(state);
  Serial.println(pwmOut);
  Serial.println(ledOut);
  delay(1000);
}

void pressed(){
  input[0] = 1;
  nextState();
  input[0] = 0;
}

void nextState(){
  if (distance < thresh)
    input[1] = 1;
  else
    input[1] = 0;
    
  // get nextState
  state = FSM[state].next[input[0]];
}

void TimerHandler(){
  ISR_Timer.run();
}

void timerInterrupt(){
  if (ledOut == HIGH)
  digitalWrite(ledPin, !digitalRead(ledPin));
}
