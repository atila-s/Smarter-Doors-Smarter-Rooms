#include <Vector.h>
#include <LiquidCrystal.h>//Setting Up Liquid Crystal

//PINS AND VARIABLES
//---------------------------

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//PhotoResistor_Inside
int photoRes_Ins = A5;
bool laserInsBreak = false;

//PhotoResistor_Outside
int photoRes_Out = A4;
bool laserOutBreak = false;

//Ultrasonic Sensor 1
int echoPin_1 = 9;
int trigPin_1 = 8;

//Ultrasonic Sensor 2
int echoPin_2 = 7;
int trigPin_2 = 6;

//Global Variables for our system
int initialInside, initialOutside;
int initDist_1, initDist_2, doorWidth;

int peopleCount = 0;
int peopleThreshold = 3; // Threshold for width of one person
int deltaPerson;

bool *x1, *x2; // Laser order in terms of break time
int laserAssign; // Keeps track of *x1 and *x2 assignment

void setup() {
  //Setting up Pins
  pinMode(echoPin_1, INPUT);
  pinMode(trigPin_1, OUTPUT);
  pinMode(echoPin_2, INPUT);
  pinMode(trigPin_2, OUTPUT);
  pinMode(photoRes_Ins,INPUT);
  pinMode(photoRes_Out,INPUT);
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.clear();
  calibrateValues();
  lcd.clear();
  displayCount();
}

void loop() {
  //THIS IS STATE_0 
  resetParameters();
  checkLasers();
  if(laserInsBreak){
    x1 = &laserInsBreak;
    x2 = &laserOutBreak;
    laserAssign = -1; // Inside laser broken first
    state_1();
  } else if(laserOutBreak){ 
    x1 = &laserOutBreak;
    x2 = &laserInsBreak;
    laserAssign = 1; // Outside laser broken first
    state_1();
  }
  displayCount();
  }

void state_1(){ 
  //Serial.println("STATE 1");
  //Serial.println(*x1);
  //Serial.println(*x2);
  while(*x1 && !*x2){
    checkLasers();
  }
  if(*x1 && *x2){
    state_2();
    return;
  } else if(!*x1 && !*x2){
    //BAKIP CIKMA veya Ters Gecme yani sayi degismiyor
    //Serial.println("SAYI DEGISMEDI!!");
    resetParameters();
    return;
  }
  Serial.println("ERROR : State 1 got out bound");
}

void state_2(){
  //Serial.println("STATE 2");
  //Serial.println(*x1);
  //Serial.println(*x2);
  while(*x1 && *x2){
    checkLasers();
    checkDistance();
  }
  if(!*x1 && *x2){
    state_3();
    return;
  } else if(*x1 && !*x2) {
    //BAKIP CIKMA veya Ters Gecme olabilir biz gene bi bakalim
    state_1(); 
    return;
  }
  Serial.println("ERROR : State 2 got out bound");
}

void state_3(){
  //Serial.println("STATE 3");
  //Serial.println(*x1);
  //Serial.println(*x2);
  while(!*x1 && *x2){
    checkLasers();
  }
  if(*x1 && *x2){
  	deltaPerson = 2;
    state_4();
    return;
  } else if (!*x1 && !*x2) {
    //Bir veya iki kisi beraber gecti
    peopleCount +=  laserAssign * deltaPerson;
    resetParameters();
    return;
  }
  Serial.println("ERROR : State 3 got out bound");
}

void state_4(){
  //Serial.println("STATE 4");
  //Serial.println(*x1);
  //Serial.println(*x2);
  while(*x1 && *x2){
    checkLasers();
  }
  if(!*x1 && *x2){
    state_5();
    return;
  } else if (*x1 && !*x2){
    state_6();
    return;
  }
  Serial.println("ERROR : State 4 got out bound");
}

void state_5(){
  //Serial.println("STATE 5");
  //Serial.println(*x1);
  //Serial.println(*x2);
  while(!*x1 && *x2){
    checkLasers();
  }
  if(!*x1 && !*x2){
    //Iki veya daha fazla kisi az zaman farkiyla gecti
    peopleCount +=  laserAssign * deltaPerson;
    resetParameters();
    return;
  } else if (*x1 && *x2){
  	deltaPerson++;
  	return;
  }
  Serial.println("ERROR : State 5 got out bound");
}

void state_6(){
  //Serial.println("STATE 6");
  //Serial.println(*x1);
  //Serial.println(*x2);
  while(*x1 && !*x2){
    checkLasers();
  }
  if(!*x1 && !*x2){
    //Ters yonde gecildi
    resetParameters();
    return;
  } else if (*x1 && *x2){
    state_4();
    return;
  }
  Serial.println("ERROR : State 6 got out bound");
}

void checkLasers(){
  if(analogRead(photoRes_Ins)*(0.9) < initialInside*0.8){
    laserInsBreak = true;
  } else {
    laserInsBreak = false;
  }
  if(analogRead(photoRes_Out)*(0.9) < initialOutside*0.8){
    laserOutBreak = true;
  } else {
    laserOutBreak = false;
  }
}

void checkDistance(){
  if(deltaPerson == 2){ 
      return;
  }
  
  int deltaDist = 0;
  int dist_1 = ultrasonDist(trigPin_1, echoPin_1);
  while(dist_1 > initDist_1*0.9){
    dist_1 = ultrasonDist(trigPin_1, echoPin_1);
  }
  Serial.println("---------");
  int dist_2 = ultrasonDist(trigPin_2, echoPin_2);
  while(dist_2 > initDist_1*0.9){
    dist_2 = ultrasonDist(trigPin_2, echoPin_2);
  }
  deltaDist = doorWidth - (dist_1 + dist_2); 
  if(deltaDist < 0) {
    deltaDist = 0; 
  }
  Serial.print("deltaDist: ");
  Serial.println(deltaDist);
  int tempDelta;
  if(deltaDist > peopleThreshold){
    tempDelta = 2;
  } else {
    tempDelta = 1;
  }
  if(tempDelta > deltaPerson){
    deltaPerson = tempDelta;
  }
}

void resetParameters(){
  laserInsBreak = false;
  laserOutBreak = false;
  deltaPerson = 0;
  laserAssign = 0;
}

void displayCount(){
  Serial.print("SHERLOCK COUNT: ");
  Serial.println(peopleCount);

  lcd.setCursor(0, 0);
  lcd.print("Sherlock Counter");
  lcd.setCursor(7, 1);
  lcd.print("        ");
  lcd.setCursor(7, 1);
  lcd.print(peopleCount);
}

void calibrateValues(){
  lcd.setCursor(2, 0);
  lcd.print("Calibrating...");

  Serial.println("CALIBRATING");
  //Calibrating Ultrasonic Sensors
  Vector<int> tempData;
  for(int i=0;i<5;i++){
    int temp = ultrasonDist(trigPin_1, echoPin_1);
    if(temp == 0){
      i--;
    }else{
      tempData.push_back(temp);
    }
    tempData.push_back(temp);
  }
  initDist_1 = mostRepeatedElement(tempData);
  
  Vector<int> tempData_2;
  for(int i=0;i<5;i++){
    int temp = ultrasonDist(trigPin_2, echoPin_2);
    if(temp == 0){
      i--;
    }else{
      tempData_2.push_back(temp);
    }
  }
  initDist_2 = mostRepeatedElement(tempData_2);
  doorWidth = (initDist_1 + initDist_2) / 2 ;
  //Calibrating Lasers and Photoresistors
  Vector<int> tempData_3;
  for(int i=0;i<5;i++){
    int temp = analogRead(photoRes_Ins)*(0.95);
    if(temp == 0){
      i--;
    }else{
      tempData_3.push_back(temp);
    }
    tempData_3.push_back(temp);
  }
  initialInside = mostRepeatedElement(tempData_3);
  
  Vector<int> tempData_4;
  for(int i=0;i<5;i++){
    int temp = analogRead(photoRes_Out)*(0.95);
    if(temp == 0){
      i--;
    }else{
      tempData_4.push_back(temp);
    }
  }
  initialOutside = mostRepeatedElement(tempData_4); 
  Serial.println("BEGIN");
  Serial.print("Door Width:");
  Serial.println(doorWidth);

  lcd.setCursor(1, 1);
  lcd.print("Door Width:");
  lcd.setCursor(13, 1);
  lcd.print(doorWidth);
  delay(5000);
}

int ultrasonDist(int trigPin, int echoPin){
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    long duration = pulseIn(echoPin, HIGH);
    int distance = duration/58.2;
    delay(60);
    return distance;
}

int mostRepeatedElement(Vector<int> &myVec){
  int bestIndex = 0;
  int maxRep = 0;
  for(int i=0; i<myVec.size()-maxRep; i++){
    int tempRep = 1;
    for(int j=i+1; j<myVec.size();j++){
       if(myVec[i]*(1.10) >= myVec[j] && myVec[i]*(0.9) <= myVec[j]){
        tempRep++;
       }
      }
    if(tempRep > maxRep){
      maxRep = tempRep;
      bestIndex = i;
    }
    if(maxRep >= myVec.size()/2){
      return myVec[bestIndex];
    }
  }
  return myVec[bestIndex];
}
