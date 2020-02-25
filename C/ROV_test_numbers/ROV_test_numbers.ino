#include <PS2X_lib.h> // Bill Porter's PS2 Library
#include <Ethernet.h>
#include <EthernetUdp.h>
#define SD_SS 4//sets up ethernet communication ports, don't need to change this part

//general Ethernet object declarations
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };//connection for ethernet
IPAddress localIp(192, 168, 1, 251);    // local ip address
IPAddress destIp(192, 168, 1, 15);      // destination ip address
IPAddress ip(192, 168, 1, 15);
IPAddress remIp(92, 168, 1, 176);
unsigned int localPort = 5678;      // local port to listen on
unsigned int port = 5678;               // destination port
EthernetUDP Udp;//object initialization

PS2X ps2x;

bool fry_release = false, mini_launch = false, mini_retract = false, tilt = false, sensors = false;
int timer1;
int timer2;
int A,B,C,D,E,F, twist, vertical, LY, LX, RY, RX, movementX, movementY;
const byte arraylength = 7, maximum = 255, minimum = 0, middle = 127, deadzone = 20;
const int NUTURAL =1500;
byte message[arraylength], packetBuffer[arraylength], offset = 10;
int left_magnitude, right_magnitude,  mag, temperature, i;
float temp, pH;





void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);//allows serial moniter
  ps2x.config_gamepad(5,3,6,2, false, false); //(clock, command, attention, data, Pressures, Rumble)
  pinMode(SD_SS, OUTPUT);
  digitalWrite(SD_SS, HIGH);
  Ethernet.begin(mac,localIp);    // static ip version
  Ethernet.begin(mac,ip);
  Udp.begin(port);
  Udp.begin(localPort);
  
}

void loop() {
  timer1 = millis();
//slave_data();   //prints out data from the temp and pH sensor when the triangle button is pushed  
  
//adjust_thrust();//adjusts thrust according to the D-pad
  //motor_values(); //converts PS2 vectors into values for the speed controllers
  //fillmessage();  //fills the array that is to be sent
  //sendmessage();  //sends the array to the slave arduino
  
  read_PS2();     //reads the PS2 values and adjusts the analog stick values to useful ranges
  print_sent(); //prints out all the information to be sent in the array
  //print_recieved();//prints out the data that was recieved
  delay(50);
}
int fix_input(int controller_value, int invert){//checks to see if the analog stick is within a center range and if it is sets the value to 0 and if not simply remaps the input value
    controller_value = (controller_value*2-255)*invert;
  return controller_value;
}
void read_PS2(){
  ps2x.read_gamepad(); //Needs to be called at least once a second
  
  LY = fix_input(ps2x.Analog(PSS_LY),-1); //left Stick Up and Down  //Analog Stick readings
  LX = fix_input(ps2x.Analog(PSS_LX),1); //Left Stick Left and Right // 0-255 return range
  LY = get_direction(LX,LY);//sets to middle if it's within dead zone


  if ( ps2x.ButtonPressed(PSB_L2)){
    movementY = -1;
  }
  else if ( ps2x.ButtonReleased(PSB_L2)){
    movementY = 0;
  }
 
 
  if (ps2x.ButtonPressed(PSB_R2)){
    movementY = 1;
  }
  else if (ps2x.ButtonReleased(PSB_R2)){
    movementY = 0;
  }
  
  if ( ps2x.ButtonPressed(PSB_L2) && ps2x.ButtonPressed(PSB_R2) ){
     movementY = 0;
 }
 //Serial.println(movementY);
 
}

int lowest(int num1, int num2) 
{
    return (num1 > num2 ) ? num2 : num1;
}

int joystick2thrust(int value, int sign){
  int thrust = value;
  if (sign == 0){
     thrust = thrust * -1;
  }
  thrust = thrust + NUTURAL;
  return thrust;
}
//int tilt(int RY){}

int get_direction(int LX, int LY){//if toggle is in deadband it sets the value to 0(middle variable)
  //returns the middle of the possible controller value if the controller value isn't past the deadzone area
  //Essentially making it so small unwanted movements in the controller won't cause the thrusters to move
  int motor_thrust = 0;
  int horz = NULL;
  int vert = NULL;
  int dead_zone = 100;
  int direct[4] = {NUTURAL,NUTURAL,NUTURAL,NUTURAL};
  
  if (LX >= dead_zone){//Right
    horz = 3;
  } 
  else if (LX <= -dead_zone){//Left
    horz = 12;
  } 
  if (LY >= dead_zone){//Up
    vert = 5;
  } 
  else if (LY <= -dead_zone){//Down
    vert = 10;
  } 
  if (abs(LX)<abs(LY)){
    motor_thrust = (min(abs(LY),200)/(float)200)*300;
  }else{
     //Serial.println(LX);
    motor_thrust = (min(abs(LX),200)/(float)200)*300;
  }
  
  if (horz != NULL and vert == NULL){
    for (i = 0;i<4; i++){
      direct[i] = joystick2thrust(motor_thrust,!!(horz & (1<<(3-i))));
      }
  }
  else if (vert != NULL and horz == NULL){
    for (i = 0;i<4; i++){
      direct[i] = joystick2thrust(motor_thrust,!!(vert & (1<<(3-i))));
      }
  }
  if (horz != NULL and vert != NULL){
    for (i = 0;i<4; i++){
      if ((vert & (1<<(3-i))) == (horz & (1<<(3-i)))){
        direct[i] = !!(horz & (1<<(3-i)));
        direct[i] = joystick2thrust(motor_thrust,!!(horz & (1<<(3-i))));
        
      }else {
        direct[i] = NUTURAL;
        }
    }
  }
 
 
  
  Serial.println(direct[0]);
  Serial.println(direct[1]);
  Serial.println(direct[2]);
  Serial.println(direct[3]);
  Serial.println("");
  
}
 
void print_sent(){
  /*
  Serial.println();
  for(i = 0; i < arraylength-1 ; i++){
    Serial.print(message[i]);
    Serial.print(" ");
  }
  */
  //Serial.println(movementY);
  //Serial.println(movementX);
  //Serial.println(LX,"x");
  
    
   
      
   //Serial.println(" ");
    //Serial.print(LX);
    //Serial.print(",");
    //Serial.print(LY);
 // Serial.println(RX);
  
}
