/********************************************************
 * PID Basic Example
 * Reading analog input 0 to control analog PWM output 3
 ********************************************************/
#include <max6675.h>
#include <PIDv1.h>

#define RELAY1 6
#define ktcS0 7
#define ktcCS 8
#define ktcCLK 9

//Define Variables we'll be connecting to
double lowpoint = 0.0 , highpoint = 0.0, Input = 0.0, Output = 0.0;

// last time offset of last run
unsigned long previousMillis = 0;
// current time offset
unsigned long currentMillis = 0;

// const time interval
const long interval = 1000;

MAX6675 ktc(ktcCLK, ktcCS, ktcS0);

//Specify the links and initial tuning parameters
double Kp=2, Ki=5, Kd=1;
PID lowPID(&Input, &Output, &lowpoint, Kp, Ki, Kd, DIRECT);
PID highPID(&Input, &Output, &highpoint, Kp, Ki, Kd, DIRECT);

// Buffer for serial
const byte numChars = 32;
char input[numChars] = {0};
boolean newData = false;

void setup()
{
  Serial.begin(9600);
  while(!Serial);
  //initialize the variables we're linked to
  Input = ktc.readFahrenheit();
  lowpoint = 183;
  highpoint = 193;
  pinMode(RELAY1, OUTPUT);
  
  //turn the PID on
  lowPID.SetOutputLimits(0,1);
  highPID.SetOutputLimits(0,1);
  lowPID.SetMode(AUTOMATIC);
  highPID.SetMode(AUTOMATIC);
}

void loop()
{
  currentMillis = millis();
  
  if(currentMillis - previousMillis >= interval){
    previousMillis = currentMillis;
    Input = ktc.readFahrenheit();
    if(Input > highpoint)
      Output=HIGH;
    else if(Input > lowpoint && Output != HIGH)
      Output=HIGH;
    else
      Output=LOW;
    lowPID.Compute();
    highPID.Compute();
    //Serial.println(Input);
    digitalWrite(RELAY1, Output);
  }

  recv();
  if(newData != 0)
    parse();
}

void recv(){
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;
  while(Serial.available() > 0 && newData == false){    
    rc = Serial.read();
    if(rc != endMarker){
      input[ndx] = rc;
      ndx++;
      if(ndx >= numChars)
        ndx = numChars - 1;
    }else{
      input[ndx] = '\0'; //terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

void snd(){
  if(newData == true){
    Serial.println(input);
    newData = false;
  }
}

void parse(){
  boolean set = false;
  if(!strncmp(input, "Get", 3))
    set = false;
  else if(!strncmp(input, "Set", 3))
    set = true;

  // Get Tempurature (no set)
  if(!strncmp(input+4, "Temp", 10 )){
    if(!set){
      Serial.print("Temp:");
      Serial.println(Input);
    }else{ 
    }
  }
  
  // Get/Set lowTemp
  if(!strncmp(input+4, "low", 3 )){
    if(!set){
      Serial.print("Low: ");
      Serial.println(lowpoint);
    }else{
      //subString(input, 6, 3);
      int low = atoi(input+8);
      if(low > 0){
        lowpoint = low;
        Serial.print("Low:");
        Serial.println(low);
      }
    }
  }
  
  
  newData = false;
}

