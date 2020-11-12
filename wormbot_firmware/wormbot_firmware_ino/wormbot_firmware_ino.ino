
/* 
 Firmware for the wormbot 3.0
 

 
 */
 
 //defines for AXIS
 
 #define TOP 1
 #define BOTTOM 2
 #define LEFT 3
 #define RIGHT 4
 #define XAXIS 5
 #define YAXIS 6
 
 
 
 //Declare pin functions on Arduino
#define stpPinY 23
#define dirPinY 25
#define MS1 31
#define MS2 33
#define MS3 35
#define EN  40

//pins for steppers
#define stpT 44
#define dirT 45
#define stpB 42
#define dirB 43
#define stpL 46
#define dirL 47
#define stpR 40
#define dirR 41

//pins for limit switches
#define TXMAX 23
#define BXMAX 22
#define TXMIN 25
#define BXMIN 24
#define LYMAX 29 
#define RYMAX 27
#define LYMIN 28
#define RYMIN 26

#define LAMPPIN 2
#define GFPPIN 4
#define CHERRYPIN 6


#define ACCELSTEPS 500
#define SLOWSPEED 800.0
#define TIMEOUT_COUNTER 10000000



#define stpPinX 22
#define dirPinX 24
#define ENX 41

#define MINMS 3 //11

#define NEGATIVE 0
#define POSITIVE 1




#include <SoftwareSerial.h>
#include <Wire.h>
#include <math.h>


const int stepsPerRev = 200;  // change this to fit the number of steps per revolution
                                     // for your motor



const unsigned long MAXCOMMANDWAIT=360000; 
                                     
unsigned long lastcommandtime=0;

unsigned long maxmotorspin=0;

int lampPin = LAMPPIN; 
int gfpPin = GFPPIN;
int cherryPin = CHERRYPIN;

int tempPin = A1; 

bool DEBUGLIMITS=false;

int newmove=false;
long moveDistance=0;
long moveDeccel=0;
float moveSpeed;

  /*                                   
int dirPinX = mePort[PORT_1].s1;//the direction pin connect to Base Board PORT1 SLOT1
int stpPinX = mePort[PORT_1].s2;//the Step pin connect to Base Board PORT1 SLOT2

int dirPinY = mePort[PORT_2].s1;
int stpPinY = mePort[PORT_2].s2;

MeLimitSwitch xplimitSwitch(PORT_6,1);
MeLimitSwitch xmlimitSwitch(PORT_6,2);
MeLimitSwitch yplimitSwitch(PORT_3,1);
MeLimitSwitch ymlimitSwitch(PORT_3,2);                                     
*/

long curr_x=0;
long curr_y=0;
long x_max=0;
long y_max=0;

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;


const int smallestDelay = MINMS; // inverse of motor's top speed
const int numAccelSteps = 1000; // number of steps before reaching top speed 2000
          
// distrubtions used in controlling motor speed
//double normal[100];
//int quadratic[100];
int inverse[numAccelSteps];


int setMS(boolean isNew, long dist){
 //returns MS for smooth acc and decel
   int wdelay=MINMS;
   if (isNew){
     newmove=false;
     moveDistance=dist;
     moveDeccel =moveDistance-ACCELSTEPS; 
     moveSpeed = SLOWSPEED;
     
   }
   
  return (wdelay);
}//end getMS 

int getMS(long currPos){
  if (currPos < ACCELSTEPS){
    
    moveSpeed= moveSpeed/1.01;
  }else // end if accel phase
  if (currPos > moveDeccel){
      moveSpeed=moveSpeed * 1.01;
  }//end if decel
  
  if (moveSpeed < MINMS) moveSpeed =  MINMS;
  if (moveSpeed > SLOWSPEED) moveSpeed = SLOWSPEED;
  return ((int)moveSpeed);
  
}//end getMS


boolean checkLimit(int switchpin){
  if (!digitalRead(switchpin)) {return true;} else {return false;}
}//end checklimitswitch




boolean checkAxisLimit(int axis, int adir){
  
  switch(axis){
  
    case XAXIS:
      if (adir==POSITIVE) {
        if (!digitalRead(TXMAX)) {return true;} 
        if (!digitalRead(BXMAX)) {return true;}
        return false; //both aren't at limit 
      }//end if positive
      
      if (adir==NEGATIVE) {
        if (!digitalRead(TXMIN)) {return true;} 
        if (!digitalRead(BXMIN)) {return true;}
        return false; //both aren't at limit 
      }//end if positive
      break;
      
   case YAXIS:
     if (adir==POSITIVE) {
        if (!digitalRead(LYMAX)) {return true;} 
        if (!digitalRead(RYMAX)) {return true;}
        return false; //both aren't at limit 
      }//end if positive
      
      if (adir==NEGATIVE) {
        if (!digitalRead(LYMIN)) {return true;} 
        if (!digitalRead(RYMIN)) {return true;}
        return false; //both aren't at limit 
      }//end if positive
      break;
    
  }//end of switch
  
}//end checkaxislimit

// *** Functions for initializing distributions ***

void initInverse() {
  double a = 10000.0;
  double b = 4;
  //double c = a / (numAccelSteps + b) - smallestDelay;
  
  for (int i = 0; i < numAccelSteps; i++) {
   long num = (long) (a / (i + b) );
   
  // long num = numAccelSteps  - (i*b);
   if (num < smallestDelay) num = smallestDelay;
  
    inverse[i] = num;
   // Serial.println(num);
  }
}
/*
void initNormal() {
  // setup normal distribution
  double pi = 3.14;
  double e = 2.72;
  double a = 2.5 / sqrt(2 * pi);
  for (int i = 0; i < 100; i++) {
    double b = -1 * pow(0.03 * i - 1.5, 2) / 2;
    normal[i] = 1 / (a * pow(e, b));
    //Serial.println(normal[i]);
  }
}

void initQuadratic() {
  // setup quadratic distribution
  for (int i = 0; i < 100; i++) {
    quadratic[i] = (int) (pow(i - 50, 4) / 6000) + 200;
  }
}
*/

/* //june11
void calibrate() {
  int ms = MINMS;
  
  // find zero
  digitalWrite(dirPinX,MINUS);
  delay(ms);
  digitalWrite(dirPinY,POSITIVE);
  delay(ms);
  maxmotorspin=0;  
  while (!checkLimit(XMIN) && maxmotorspin++ < TIMEOUT_COUNTER){
    digitalWrite(stpPinX, HIGH);
    delayMicroseconds(ms);
    digitalWrite(stpPinX, LOW);
    delayMicroseconds(ms);
  }//while not at x==0
  maxmotorspin=0;
  while (!checkLimit(YMIN) && maxmotorspin++ < TIMEOUT_COUNTER){
    digitalWrite(stpPinY, HIGH);
    delayMicroseconds(ms);
    digitalWrite(stpPinY, LOW);
    delayMicroseconds(ms);  
  }//while not at x==0
  
  curr_x = 0;
  curr_y = 0;
  
  // calibrate max
  digitalWrite(dirPinX,POSITIVE);
  delay(ms);
  digitalWrite(dirPinY,MINUS);
  delay(ms);
  maxmotorspin=0;
  while (!checkLimit(XMAX) && maxmotorspin++ < TIMEOUT_COUNTER){
    digitalWrite(stpPinX, HIGH);
    delayMicroseconds(ms);
    digitalWrite(stpPinX, LOW);
    delayMicroseconds(ms);
    curr_x++;
  }//while not at x==max
  maxmotorspin=0;
  while (!checkLimit(YMAX) && maxmotorspin++ < TIMEOUT_COUNTER){
    digitalWrite(stpPinY, HIGH);
    delayMicroseconds(ms);
    digitalWrite(stpPinY, LOW);
    delayMicroseconds(ms);
    curr_y++;
  }//while not at y==max
  
  x_max = curr_x;
  y_max = curr_y;
  
  goto_machine_zero();
}
*/ //june11

void goto_machine_zero(void){

 // move_to_xy(-100,-100);
  
  curr_x=0;
  curr_y=0;
  //Serial.println(curr_x);
  //Serial.println(curr_y);
  
}//end goto maching zero

void goto_machine_max(void){
  
 // move_to_xy(x_max + 100, y_max + 100);

  x_max = curr_x;
  y_max = curr_y;
  
  
}//end goto maching zero



void move_to_x(int x) {
  move_to_xy(x, curr_y);
}

void move_to_y(int y) {
 move_to_xy(curr_x, y);
}


void zero_all_axes(void){
  
  //set all motors to negative
  digitalWrite(dirT,NEGATIVE);
  digitalWrite(dirB,NEGATIVE);
  digitalWrite(dirL,NEGATIVE);
  digitalWrite(dirR,NEGATIVE);
  
  boolean allZero,topZero,botZero,leftZero,rightZero = false;
 
  boolean done = false;
  
  
  while(!done){
        
  
    

    
    //if (!topZero) 
    if (!checkLimit(TXMIN)) digitalWrite(stpT, HIGH);
    //if (!botZero) 
    if (!checkLimit(BXMIN)) digitalWrite(stpB, HIGH);

   
    delayMicroseconds(MINMS);
    
   //if (!topZero) 
    if (!checkLimit(TXMIN)) digitalWrite(stpT, LOW);
    //if (!botZero) 
    if (!checkLimit(BXMIN)) digitalWrite(stpB, LOW);

    
    delayMicroseconds(MINMS);
    

        
    if (checkLimit(TXMIN) && checkLimit(BXMIN)) {
     //Serial.println("all are zero");
        done=true;
    }
  }//end while all switches not at zero 
  
  done=false;
  
  while(!done){
        
  
    

    
    
   // if (!leftZero) 
    if (!checkLimit(LYMIN)) digitalWrite(stpL, HIGH); 
   // if (!rightZero) 
    if (!checkLimit(RYMIN)) digitalWrite(stpR, HIGH);
   
    delayMicroseconds(MINMS);

   // if (!leftZero) 
    if (!checkLimit(LYMIN)) digitalWrite(stpL, LOW); 
   // if (!rightZero) 
    if (!checkLimit(RYMIN)) digitalWrite(stpR, LOW);
    
    delayMicroseconds(MINMS);
    

        
    if (checkLimit(LYMIN) && checkLimit(RYMIN)) {
     //Serial.println("all are zero");
        done=true;
    }
  }//end while all switches not at zero 
  
  
  
  curr_x=0;
  curr_y=0;
  
  
}//end zero_all axes


void max_all_axes(void){
  
 
  
 
  while(1){
        
         //set all motors to negative
  digitalWrite(dirT,POSITIVE);
  digitalWrite(dirB,POSITIVE);
  digitalWrite(dirL,POSITIVE);
  digitalWrite(dirR,POSITIVE);

    
    //if (!topZero) 
    if (!checkLimit(TXMAX)) {
      digitalWrite(stpT, HIGH);
      curr_x++;
      delayMicroseconds(MINMS);
      digitalWrite(stpT, LOW);
            delayMicroseconds(MINMS);

    }
    if (!checkLimit(BXMAX)) {
      digitalWrite(stpB, HIGH);
      
      delayMicroseconds(MINMS);
      digitalWrite(stpB, LOW);
            delayMicroseconds(MINMS);

    }
    if (!checkLimit(LYMAX)) {
      digitalWrite(stpL, HIGH);
      curr_y++;
      delayMicroseconds(MINMS);
      digitalWrite(stpL, LOW);
            delayMicroseconds(MINMS);

    }
    if (!checkLimit(RYMAX)) {
      digitalWrite(stpR, HIGH);
      
      delayMicroseconds(MINMS);
      digitalWrite(stpR, LOW);
            delayMicroseconds(MINMS);

    }
       

 

        
    if (checkLimit(TXMAX) && checkLimit(BXMAX) && checkLimit(LYMAX) && checkLimit(RYMAX)) {
     //Serial.print (curr_x);
      //Serial.print (",");
      //Serial.println(curr_y);
        break;
    }
  }//end while all switches not at zero 
  
  
  
  
  
  
  
}//end zero_all axes


void motion(int motor, long d){
  
  int dir=0;
  
  if (d < 0) dir =0; //negative
  if (d > 0) dir =1; //positive
  
  int delta =0; //how to increment the movement counter
  
  
  
  d=abs(d);
  
  
  
  switch (motor) {
    
    case TOP:
        digitalWrite(dirT,dir);
     for (int i=0; i < d; i++){
        digitalWrite(stpT, HIGH);
        delayMicroseconds(MINMS);
        digitalWrite(stpT, LOW);
        delayMicroseconds(MINMS);
     }//end for each step     
    
    
      break;
      
      case XAXIS:
        digitalWrite(dirT,dir);
        digitalWrite(dirB,dir);
        for(int i=0; i < d; i++){
          digitalWrite(stpT, HIGH);
          digitalWrite(stpB, HIGH);
          
          delayMicroseconds(MINMS);
          digitalWrite(stpT, LOW);
          digitalWrite(stpB, LOW);
          
          delayMicroseconds(MINMS);
          
        }//end while motion left
        break;
        
        case YAXIS:
        digitalWrite(dirL,dir);
        digitalWrite(dirR,dir);
        boolean ltog=false;
        
        for(int i=0; i < d; i++){
          //if (ltog) 
          digitalWrite(stpL, HIGH);
          digitalWrite(stpR, HIGH);
          
          delayMicroseconds(MINMS);
          //if (ltog) {
          digitalWrite(stpL, LOW);
         // ltog=false;} else ltog=true;
          digitalWrite(stpR, LOW);
          
          delayMicroseconds(MINMS);
          
        }//end while motion left
        break;
        
      
    
    
    
  }//end motor switch
  
  
}//end motion




void move_to_xy(long x, long y) {
 
  long dx = x - curr_x;
  long dy = y - curr_y; 
  if (dx == 0 && dy == 0) {
    delay(1);
    return;
  }
  
  int limitSwitchX;
  int limitSwitchY;
  
  long numStepsX;
  long numStepsY;
  
  int xdir,ydir=0;
  
  if (dx < 0) {
    xdir = NEGATIVE;
    numStepsX = -1 * dx;
  } else {
    xdir = POSITIVE;
    numStepsX = dx;
  }
  
  if (dy < 0) {
    ydir = NEGATIVE;
    numStepsY = -1 * dy;
  } else {
    ydir = POSITIVE;
    numStepsY = dy;
  }
  
  
  long i_x = 0;
  long i_y = 0;
  boolean x_reached = (numStepsX == 0) || checkAxisLimit(XAXIS,xdir);
  boolean y_reached = (numStepsY == 0) || checkAxisLimit(YAXIS,ydir);
  
  //set motor directions
  digitalWrite(dirT,xdir);
  digitalWrite(dirB,xdir);
  digitalWrite(dirL,ydir);
  digitalWrite(dirR,ydir);
  
 maxmotorspin=0;
 newmove=true;
 setMS(newmove,numStepsX);
 
    while(!x_reached && maxmotorspin++ < TIMEOUT_COUNTER) {
      int ms =getMS(i_x);
      //if (i_x < numAccelSteps)                  ms = inverse[i_x];
      //else if (i_x > numStepsX - numAccelSteps) ms = inverse[numStepsX - i_x];
      //else                                      ms = inverse[numAccelSteps - 1];
    
      digitalWrite(stpT, HIGH);
      delayMicroseconds(MINMS);
      digitalWrite(stpT, LOW);
      delayMicroseconds(MINMS);
      digitalWrite(stpB, HIGH);
      delayMicroseconds(MINMS);
      digitalWrite(stpB, LOW);
      delayMicroseconds(ms);
      
      i_x++;
      x_reached = (i_x == numStepsX) || checkAxisLimit(XAXIS,xdir);
    }
    
    delay(MINMS);
    
     newmove=true;
     setMS(newmove,numStepsY);
    maxmotorspin=0;
    while (!y_reached && maxmotorspin++ < TIMEOUT_COUNTER) {
      int ms=getMS(i_y);
      //if (i_y < numAccelSteps)                  ms = inverse[i_y];
      //else if (i_y > numStepsY - numAccelSteps) ms = inverse[numStepsY - i_y];
      //else                                      ms = inverse[numAccelSteps - 1];
    
      digitalWrite(stpL, HIGH);
      delayMicroseconds(MINMS);
       digitalWrite(stpL, LOW);
       delayMicroseconds(MINMS);
      digitalWrite(stpR, HIGH);
      delayMicroseconds(MINMS);
      digitalWrite(stpR, LOW);
      delayMicroseconds(ms);
      
      i_y++;
      y_reached = (i_y == numStepsY) || checkAxisLimit(YAXIS,ydir);
    }
    

  
  if (dx < 0) curr_x -= i_x;
  else        curr_x += i_x;
  
  if (dy < 0) curr_y -= i_y;
  else        curr_y += i_y;
  
  if (checkLimit(TXMIN)) curr_x=0;
  if (checkLimit(LYMIN)) curr_y=0;

  
  
}//end 








/* //june11

void newMachineZero(void){
  
  digitalWrite(dirPinX,MINUS); //set x pin minus
  while (!checkLimit(XMIN)){
    
      digitalWrite(stpPinX, HIGH);
      delayMicroseconds(MINMS);
      digitalWrite(stpPinX, LOW);
      delayMicroseconds(MINMS);
    
  }//end while not at x ==0  
  
  digitalWrite(dirPinY,MINUS); //set x pin minus
  while (!checkLimit(YMIN)){
    
      digitalWrite(stpPinY, HIGH);
      delayMicroseconds(MINMS);
      digitalWrite(stpPinY, LOW);
      delayMicroseconds(MINMS);
    
  }//end while not at y ==0  
  
  curr_x=curr_y=0;
  
}//end newmachinezero

*/ //june 11

void setLamp(int intensity){
  //if (intensity >= 99) digitalWrite(lampPin, HIGH);
 // if (intensity <= 0) digitalWrite(lampPin,LOW);
   analogWrite(lampPin, intensity);
  
  
}//end setLamp

void setGFP(int intensity){
  //if (intensity >= 99) digitalWrite(lampPin, HIGH);
 // if (intensity <= 0) digitalWrite(lampPin,LOW);
   analogWrite(gfpPin, intensity);
  
  
}//end setLamp

void setCherry(int intensity){
  //if (intensity >= 99) digitalWrite(lampPin, HIGH);
 // if (intensity <= 0) digitalWrite(lampPin,LOW);
   analogWrite(cherryPin, intensity);
  
  
}//end setLamp

/* //june11

void newMachineMax(void){
  
   digitalWrite(dirPinX,POSITIVE); //set x pin positive
  while (!checkLimit(XMAX)){
    
      digitalWrite(stpPinX, HIGH);
      delayMicroseconds(MINMS);
      digitalWrite(stpPinX, LOW);
      delayMicroseconds(MINMS);
      curr_x++;
    
  }//end while not at x ==max  
  
  digitalWrite(dirPinY,POSITIVE); //set x pin positive
  while (!checkLimit(YMAX)){
    
      digitalWrite(stpPinY, HIGH);
      delayMicroseconds(MINMS);
      digitalWrite(stpPinY, LOW);
      delayMicroseconds(MINMS);
      curr_y++;
    
  }//end while not at y ==max  

  Serial.println("xmax="+ String(curr_x) + " ymax=" + String(curr_y));
  
}//end newmachine max


void jasonCalibrate(void){
  newMachineZero();
  newMachineMax();
  
}//end jason calibrate
*/ //june11


void quickCalibrate(void){
  
  zero_all_axes();
  delay(3);
  move_to_xy(8000,8000);
  delay(3);
  zero_all_axes();
  
  
}//end quick calibrate



void setup() {
  
  
  // initialize the serial port:
  Serial.begin(9600);
  
  //setup stepper pins
  pinMode(stpT, OUTPUT);
  pinMode(dirT, OUTPUT);
  pinMode(stpB, OUTPUT);
  pinMode(dirB, OUTPUT);
  pinMode(stpL, OUTPUT);
  pinMode(dirL, OUTPUT);
  pinMode(stpR, OUTPUT);
  pinMode(dirR, OUTPUT);
    
  
  //setup lamp
  pinMode(lampPin, OUTPUT);
   pinMode(gfpPin, OUTPUT);
    pinMode(cherryPin, OUTPUT);
  
 
  //setup limit switches
  pinMode(TXMAX, INPUT_PULLUP);
  pinMode(BXMAX, INPUT_PULLUP);
  pinMode(TXMIN, INPUT_PULLUP);
  pinMode(BXMIN, INPUT_PULLUP);
  pinMode(LYMAX, INPUT_PULLUP);
  pinMode(RYMAX, INPUT_PULLUP);
  pinMode(LYMIN, INPUT_PULLUP);
  pinMode(RYMIN, INPUT_PULLUP);
  
  
  //initNormal();
  //initQuadratic();
  //Serial.println("Initializing movement curves...");
  initInverse();
  
 // calibrate(); //un comment
 //jasonCalibrate();
 zero_all_axes(); //2020
 quickCalibrate();// 2020
 //goto_machine_zero();
 //goto_machine_max();
 //goto_machine_zero();
  //Serial.println("RR");
  
}//end setup

void readTemp(void){
  long volts; 
  for (int i=0; i < 99; i++) {
    volts+= analogRead(tempPin);
  }
  float aveVolt = (float)volts / 100.000;
  
  float mvlt= (aveVolt/1024.000) * 5000.000;
  float ten=10.000;
  float f= mvlt/ten;

float c= (f - 32.000) * (0.55555555556);

Serial.print ("C*");
Serial.println(c);

}//end readtemp

void loop(){
  //CHECK LIMIT SWITCHES
 if (DEBUGLIMITS){ 
   if (checkLimit(TXMAX)) Serial.println("top X MAX");
    if (checkLimit(BXMAX)) Serial.println("bot X MAX");
     if (checkLimit(TXMIN)) Serial.println("top X Min");
     if (checkLimit(BXMIN)) Serial.println("bot X Min");
     if (checkLimit(LYMAX)) Serial.println("left y max");
     if (checkLimit(RYMAX)) Serial.println("right y max");
     if (checkLimit(LYMIN)) Serial.println("left y min");
     if (checkLimit(RYMIN)) Serial.println("right y min");

 }

  
  
  if (stringComplete){
    //parse command
    //
    
    
    if (inputString.indexOf("D") >=0){
      String moveamnt = inputString.substring(inputString.indexOf("D")+1);
      motion(XAXIS,moveamnt.toInt());
     // move_px(moveamnt.toInt());
    } else //end if D
    if  (inputString.indexOf("A") >=0){
      String moveamnt = inputString.substring(inputString.indexOf("A")+1);
      motion(XAXIS,-moveamnt.toInt());
     // move_mx(moveamnt.toInt());
    }else //end if A
    if  (inputString.indexOf("W") >=0){
      String moveamnt = inputString.substring(inputString.indexOf("W")+1);
       motion(YAXIS,-moveamnt.toInt());
     // move_my(moveamnt.toInt());
    } else //end if W
    if  (inputString.indexOf("S") >=0){
      String moveamnt = inputString.substring(inputString.indexOf("S")+1);
      motion(YAXIS,moveamnt.toInt());
      //move_py(moveamnt.toInt());
    } else //end if S 
    if  (inputString.indexOf("ZZ") >=0){
   //   goto_machine_zero();
      //newMachineZero();
      zero_all_axes();
    } else 
    if  (inputString.indexOf("LL") >=0){
      //newMachineMax();
      max_all_axes();
      //goto_machine_max();
    } else
    if (inputString.indexOf("CC")>=0){
	quickCalibrate();
    } else
    if (inputString.indexOf("IL") >=0){
        String lightamount = inputString.substring(inputString.indexOf("IL")+2);
        int lumos = lightamount.toInt();
        setLamp(lumos);
        
    } else
    if (inputString.indexOf("GL") >=0){
        String lightamount = inputString.substring(inputString.indexOf("GL")+2);
        int lumos = lightamount.toInt();
        setGFP(lumos);
        
    } else
    if (inputString.indexOf("TR") >=0){
        readTemp();
    } else
    if (inputString.indexOf("CL") >=0){
        String lightamount = inputString.substring(inputString.indexOf("CL")+2);
        int lumos = lightamount.toInt();
        setCherry(lumos);
        
    } else
    if (inputString.indexOf("P")>=0){ 
      Serial.print (curr_x);
      Serial.print (",");
      Serial.println(curr_y);
    }else //end if p
    if(inputString.indexOf("MX") >=0){
      String moveamnt = inputString.substring(inputString.indexOf("MX")+2);
      long mv_x=moveamnt.toInt();
      if (mv_x <0) mv_x=0;
      move_to_x(mv_x);
    } //
    else if(inputString.indexOf("MY") >=0){
      String moveamnt = inputString.substring(inputString.indexOf("MY")+2);
      long mv_y=moveamnt.toInt();
      if (mv_y <0) mv_y=0;
      move_to_y(mv_y);
    } else if (inputString.indexOf("M") >= 0 && inputString.indexOf(",") >= 0) {
      long x = inputString.substring(inputString.indexOf("M") + 1, inputString.indexOf(",")).toInt();
      String y_str = inputString.substring(inputString.indexOf(",") + 1);
      long y = y_str.toInt();
      move_to_xy(x, y);
    }
    
    
    
    //Serial.println(inputString);
    inputString="";
    stringComplete=false;
    Serial.println("RR");
    //reset command timer
    lastcommandtime=millis();
    
  }//end if complete
  
  unsigned long currtime;
  currtime = millis();
  
  //if ((currtime - lastcommandtime) > MAXCOMMANDWAIT){
   // Serial.println("RR");
   // lastcommandtime=millis();
 // }//end if timeout  
  
}//end loop

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    } 
  }
}
