#include <SPI.h>        
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Servo.h>
#include <OneWire.h> 
#include <DallasTemperature.h>

#define fry A0
#define mini A1
#define retract A2
#define temp_pin A3            //pH meter Analog output
#define pH_pin A4         //temp sensor pin

#define pH_offset 0.00            //deviation compensate
#define temp_offset 0.00
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth  40    //times of collection

//setting up ethernet
#define SD_SS 4//sets up ethernet communication ports, don't need to change this part
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };//connection for ethernet
IPAddress localIp(192, 168, 1, 251);    // local ip address
IPAddress destIp(192, 168, 1, 15);      // destination ip address
IPAddress ip(192, 168, 1, 15);
IPAddress remIp(92, 168, 1, 176);
unsigned int localPort = 5678;      // local port to listen on
unsigned int port = 5678;               // destination port
EthernetUDP Udp;//initializes objects

OneWire oneWire(temp_pin); 
DallasTemperature sensors(&oneWire);

Servo thrusterA;
Servo thrusterB;
Servo thrusterC;
Servo thrusterD;
Servo thrusterE;
Servo thrusterF;

const int halt = 1500, arraylength = 10;
int stick_value, go, reverse, up_down, a, b, c, d, e, f, pHArray[ArrayLenth], pHArrayIndex = 0, i, j;// value from the analog sticks from the controller
byte movement, fry_release, tilt, mini_launch, mini_retract, sense;//determines what direction the ROV needs to go
float thruster_adjust, temp, pH_value;//Changes the % of full thrust the
byte packetBuffer[arraylength], message[arraylength];//array that is recieved and

void setup(){
  Serial.begin(9600);//allows serial moniter

  //ethernet setup
  pinMode(SD_SS, OUTPUT);//sets up ethernet
  digitalWrite(SD_SS, HIGH);
  Ethernet.begin(mac,localIp);
  Ethernet.begin(mac,ip);
  Udp.begin(port);
  Udp.begin(localPort);

  sensors.begin();
  
  pinMode(fry, OUTPUT);//initializes output pins
  digitalWrite(fry, HIGH);
  pinMode(mini, OUTPUT);
  digitalWrite(mini, LOW);
  pinMode(retract, OUTPUT);
  digitalWrite(retract, LOW);

  thrusterA.attach(3);//sets up thruster pins and halts them
  thrusterB.attach(5);
  thrusterC.attach(6);
  thrusterD.attach(9); 
  thrusterE.attach(7);//up/down
  thrusterF.attach(8);//up/down
  thrusterA.writeMicroseconds(halt);
  thrusterB.writeMicroseconds(halt);  
  thrusterC.writeMicroseconds(halt);  
  thrusterD.writeMicroseconds(halt);  
  thrusterE.writeMicroseconds(halt);
  thrusterF.writeMicroseconds(halt);
}

void loop() 
{ 
  recieve();        //recieves the array and puts it inside 
  print_recieved();
  thruster_values();//sets up all the variables that will be used in the thrusters_go
  thrusters_go();   //Sends out the pulse to the ESC's
  use_tools();      //Determines if the variables from the array will cause any pin outputs
  //fill_message();
  //sendmessage();
  
  //print_values(); //prints out 
  //print_sent();
  
  delay(50);

}
void recieve(){//recieves data from ethernet and puts it in packetBuffer array
  int packetSize = Udp.parsePacket();
  IPAddress remote = Udp.remoteIP();
  Udp.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE);
  
  movement = packetBuffer[0];
  stick_value = packetBuffer[1];
  up_down = packetBuffer[2];
  tilt = packetBuffer[3];
  thruster_adjust = (float)packetBuffer[4]/10;//divides the value by 10
  fry_release = packetBuffer[5];
  mini_launch = packetBuffer[6];
  mini_retract = packetBuffer[7];
  sense = packetBuffer[8];
}

void thruster_values(){
  stick_value = map(stick_value, 0, 256, 1200, 1800);//mapping the values to the thruster values
  up_down = map(up_down, 0, 256, 1150, 1850);
  up_down+=3;//needs this to adjust so it's centered around 1500
  go = (stick_value-1500)*thruster_adjust + 1500;
  reverse = rev(go);
  up_down = (up_down-1500)*thruster_adjust+1500;
 
  if(tilt == 0){
    e = rev(up_down);
    f = up_down;
  }
  else if(tilt == 1){//allows the up and down to go in different directions
    e = up_down;
    f = up_down;
  }
  if(movement == 0){//all thrusters are off
    a = halt;
    b = halt;
    c = halt;
    d = halt; 
  }
  else if(movement == 1){//forwards and backwards
    a = reverse;
    b = go;
    c = reverse;
    d = go;
  }
  else if(movement == 2){//move left and right
    a = go;
    b = go;
    c = reverse;
    d = reverse;
  }
  else if(movement == 3){//twist left and right
    a = go;
    b = halt;
    c = halt;
    d = go;
  }
  else{
    Serial.println("Something went wrong");
  } 
}

void thrusters_go(){
  thrusterA.writeMicroseconds(a);
  thrusterB.writeMicroseconds(b);  
  thrusterC.writeMicroseconds(c);  
  thrusterD.writeMicroseconds(d);
  thrusterE.writeMicroseconds(e);
  thrusterF.writeMicroseconds(f);
}

void use_tools(){
  if(fry_release == 0){//Grout Fry release, will start on because in order to release we need to turn the electromagnet off
    digitalWrite(fry, LOW);
  }
  else if(fry_release == 1){
    digitalWrite(fry, HIGH);
  }
  if(mini_launch == 0){//Mini launch will start off
    digitalWrite(mini, HIGH);
  }
  else if(mini_launch == 1){
    digitalWrite(mini, LOW);
  }
  if(mini_retract == 0){
    digitalWrite(retract, HIGH);
  }
  else if(mini_retract == 1){
    digitalWrite(retract, LOW);
  }
  if(sense == 1){
    for(i = 0; i < 10; i++){
      get_temp();
      get_pH();
      fill_message();
      sendmessage();
    }
  }
}

void get_temp(){
  sensors.requestTemperatures(); // Send the command to get temperature readings 
  temp = sensors.getTempCByIndex(0) + temp_offset;
}

void get_pH(){  //taken from pH sensor website
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue,voltage;
  if(millis()-samplingTime > samplingInterval){
      pHArray[pHArrayIndex++]=analogRead(pH_pin);
      if(pHArrayIndex==ArrayLenth)pHArrayIndex=0;
      voltage = avergearray(pHArray, ArrayLenth)*5.0/1024;
      pHValue = 3.5*voltage+pH_offset;
      samplingTime=millis();
  }
  pH_value = pHValue;
  printTime=millis();
  //Serial.println(pHValue);
}

void fill_message(){
    message[0] = temp * 2;//Adjusts the number so I can extrapolate the decimal when getting it out of the byte array
    message[1] = pH_value * 18;
}

void sendmessage(){//sends the message
  Udp.beginPacket(destIp, port);
  Udp.write(message,arraylength);
  Udp.endPacket();
  delay(50);
}

void print_recieved(){
  Serial.println();
  for(i = 0; i < arraylength; i++){
    Serial.print(packetBuffer[i]);
    Serial.print(" ");
  }
}

void print_values(){
  Serial.print(a);
  Serial.print(" ");
  Serial.print(b);
  Serial.print(" ");
  Serial.print(c);
  Serial.print(" ");
  Serial.print(d);
  Serial.print(" ");
  Serial.print(e);
  Serial.print(" ");
  Serial.print(f);
  Serial.print(" ");
  Serial.print(fry_release);
  Serial.print(" ");
  Serial.print(mini_launch);
  Serial.print(" ");
  Serial.print(mini_retract);
  Serial.print("\n");
}

void print_sent(){
  Serial.println();
  for(i = 0; i < arraylength; i++){
    Serial.print(message[i]);
    Serial.print(" ");
  }
}

int rev(int thruster){
  //returns the value that will cause the thrusters to go in
  //reverse of the thruster value given by the parameters
  if(thruster>halt){
    int num = thruster-halt;
    return (halt-num);
  }
  else{
    int num = halt-thruster;
    return(halt+num);
  }
}

double avergearray(int* arr, int number){//Need this for the pH sensor, taken directly off of pH sensor website
  int max,min;
  double avg;
  long amount=0;
  if(number<=0){
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if(number<5){   //less than 5, calculated directly statistics
    for(j=0;j<number;j++){
      amount+=arr[j];
    }
    avg = amount/number;
    return avg;
  }else{
    if(arr[0]<arr[1]){
      min = arr[0];max=arr[1];
    }
    else{
      min=arr[1];max=arr[0];
    }
    for(j=2;j<number;j++){
      if(arr[j]<min){
        amount+=min;        //arr<min
        min=arr[j];
      }else {
        if(arr[j]>max){
          amount+=max;    //arr>max
          max=arr[j];
        }else{
          amount+=arr[j]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount/(number-2);
  }//if
  return avg;
}
