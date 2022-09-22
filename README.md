### LAB : Smart mini-fan with STM32-duino

**Date:** 2022.09.22

**Author:** Heo Dabin

**Github:** [Github code](https://github.com/DABINHEO/tutorial-NM.git)

**Demo Video:** [Youtube Link](https://youtu.be/gpzFaWjyOL8)

##            



### Introduction

In this LAB, I create a program to control the operate of the motor with NUCLEO-F401RE hardware and additional device, the ultrasonic distance sensor and button. The speed of the motor can be adjusted by operating the button. The speed of the motor consists of three stages: Off (0%), Medium (50%), High (100%), and Repeat Off - Medium - High - Off as the button is pressed. The motor is designed to receive an operation signal if the distance between the ultrasonic sensor and the object (reflecting sound) is less than 100mm, and the motor is designed to operate without the operation signal. In addition, if the motor is not in the 'off' stage, the LED is designed to blink every 1 second, which operates regardless of the motion signal received from the ultrasonic sensor.

##           



### Requirement

#### Hardware

* MCU
  
* NUCLEO-F401RE
  
* Sensor:
  
  * Ultrasonic distance sensor(HC-SR04) x 1
* Actuator/Display
  * LED x 1
  * DC motor (RK-280RA) x 1
  
  

#### Software

* Arduino IDE



##          



### Problem



#### Procedure

The motor's output changes according to the duty ratio of the received PWM signal, and in this experiment, the duty ratio of PWM can be changed using NUCLEO's Button. Accordingly, the motor's operation modes were set to be duty ratio of PWM signal(period is 1msec) 0% : S0 (off), 50% : S1 (middle), and 100% : S2 (high), and the operation modes were designed to be changed in the order of S0 - S1 - S2 - S0 as the button was pressed when the initial state was S0.

In this LAB, the motor operates according to the output of the ultrasonic distance sensor and the state(operation mode of the motor). Regardless of the state, if the distance measured by the ultrasonic distance sensor is 100mm or more, the motor does not operate, and if the operation mode of the motor is less than 50mm and the operation mode of the motor is S1 or S2, the motor operates.

The distance calculation method of the ultrasonic distance sensor is as follows. When a specific length of HIGH signal is input through the Trigger terminal, the sensor sends out ultrasonic waves, and when the ultrasonic waves hit an object that reflects sound, the sensor receives them again and outputs a round trip time through the Echo terminal. For the formula for round trip time and distance between objects, 1[us]/58 = distance[cm]. In this LAB, a PWM signal with a pulse width of 10[us] with a period of 1 second was entered on the Trigger terminal.

The LED blinks every one second if the motor's operating mode is S1 or S2, regardless of the output of the ultrasonic distance sensor.

#### Configuration

**LED**

Triger:

* State of System
* Pin: **D13** 
* State: S0, S1, S2 

**DC motor**

Triger:

* PWM signal
* Pin: **D11** 
* PWM out: 0%, 50%, 100% of duty ratio

**Ultrasonic distance sensor**

Triger:

* Generate a trigger pulse as PWM to the sensor
* Pin: **D10** 
* PWM out: 1s period, 10us pulse-width



#### Circuit/Wiring Diagram

External circuit diagram that connects MCU pins to peripherals(sensor/actuator)

![image](https://user-images.githubusercontent.com/113574284/191644106-1f9ae194-6756-404d-9404-b0183d32f452.png)

##           



### Algorithm

**Input:**

* Button Pressed {0, 1}
* Signal of Ultrasonic Distance Sensor {F, T}

**Output:**

* VEL {0%, 50%, 100%}
* LED {ON, OFF}

**State:**

* S0: FAN OFF / LED OFF State
* S1: If Signal of Ultrasonic Distance Sensor if T, FAN ON(50%) / LED ON State
* S2: If Signal of Ultrasonic Distance Sensor if T, FAN ON(100%) / LED ON State

#### 

#### Overview

**Mealy FSM Diagram**

![image](https://user-images.githubusercontent.com/113574284/191646900-c00d3f10-6362-4001-a82e-e886d67faf74.png)

**Mealy FSM Table**

![image](https://user-images.githubusercontent.com/113574284/191059234-8145f0f5-b27d-4e0b-bf33-f108aa939670.png)

#### Description with Code

* Lab source code: [Github code](https://github.com/DABINHEO/tutorial-NM.git)
* Description1


```c++
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
```
* Description2

```c++
// Pin setting
const int ledPin = 13;
const int pwmPin = 11;
const int btnPin = 3;
const int trigPin = 10;
const int echoPin = 7;
```

* Description3

```c++
// Default State setting
unsigned char state = S0;
unsigned char input[2] = {0, 0};
unsigned char pwmOut = 0;
unsigned char ledOut = LOW;

unsigned long duration;
float distance;
int thresh = 10;    // Setting the operating distance(10cm)
```

* Description4


```c++
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
  
  // Setting Baud Rate
  Serial.begin(9600);
}
```

* Description5


```c++
// Generate pwm singal on the trigger pin.
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(10);
```

```c++
// Distance is calculated using how much time it takes.
  duration = pulseIn(echoPin, HIGH);
  distance = (float)duration / 58.0;
```

```c++
// Output of states
  pwmOut = FSM[state].out[ input[0] * 2 + input[1] ][0];
  ledOut = FSM[state].out[ input[0] * 2 + input[1] ][1];
  analogWrite(pwmPin, pwmOut);
  if (ledOut == HIGH)
    digitalWrite(ledPin, !digitalRead(ledPin));		// Blink LED in 1sec period
```

```c++
void pressed(){
  // The state changes when the button is pressed.
  input[0] = 1;
  nextState();
  input[0] = 0;
}
```

```c++
void nextState(){
  // The motor operates only when the measured distance is less than the set value(10cm).
  if (distance < thresh)
    input[1] = 1;
  else
    input[1] = 0;
    
  // get nextState
  state = FSM[state].next[input[0]];
}
```

##         



### Results and Analysis



#### Results

In this LAB, the operation of a DC motor using an ultrasonic distance sensor and PWM was tested using Miley FSM, and as confirmed through FSM, it was confirmed that it generates appropriate output depending on the State(Motor operation mode) and input.



#### Demo Video

[Youtube Link](https://youtu.be/gpzFaWjyOL8)

​	

#### Analysis

The structure was constructed to produce the appropriate output with two inputs, and it was difficult to access the output in the structure, but the two inputs were converted into binary numbers and converted into four cases: 00,01,10,11 and coded to access the first, second, third, and the appropriate output was found according to the input.

##          



### Reference

[Tutorial: arduino-stm32 Part1](https://ykkim.gitbook.io/ec/course/tutorial/tutorial-arduino-stm32-part1)

[Tutorial: arduino-stm32 Part2](https://ykkim.gitbook.io/ec/course/tutorial/tutorial-arduino-stm32-part-2)

[LAB Report Template](https://ykkim.gitbook.io/ec/course/lab/lab-report-template)

[LAB: Smart mini-fan with STM32-duino](https://ykkim.gitbook.io/ec/course/lab/lab-smart-mini-fan-with-stm32-duino)
