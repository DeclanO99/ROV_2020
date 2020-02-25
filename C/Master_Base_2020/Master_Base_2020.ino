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
int A,B,C,D,E,F, twist, vertical, LY, LX, RY, RX, movementX, movementY;

const byte arraylength = 7, DEAD_ZONE = 100,EMPTY = -1;
const int MOTOR_NUTURAL = 1500, MOTOR_REVERSE = 1200, MOTOR_FORWARD = 1800;

int motor_values[4]={MOTOR_NUTURAL,MOTOR_NUTURAL,MOTOR_NUTURAL,MOTOR_NUTURAL};
int z_axis_motors[2] = {MOTOR_NUTURAL,MOTOR_NUTURAL};
byte message[arraylength], packetBuffer[arraylength], offset = 10;
int left_magnitude, right_magnitude,  mag, temperature, i;
float temp, pH;

void setup() {  
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
  
//slave_data();   //prints out data from the temp and pH sensor when the triangle button is pushed  
  read_PS2();     //reads the PS2 values and adjusts the analog stick values to useful ranges
//adjust_thrust();//adjusts thrust according to the D-pad
  get_motor_values(); //converts PS2 vectors into values for the speed controllers
  
  get_z_axis();
  //get_rotation();
  get_tilt();
  //get_tilt();
  //debug_inputs();
  fillmessage();  //fills the array that is to be sent
  sendmessage();  //sends the array to the slave arduino
  print_sent(); //prints out all the information to be sent in the array
  //delay(600);
}
int fix_input(int controller_value, int invert){//checks to see if the analog stick is within a center range and if it is sets the value to 0 and if not simply remaps the input value
    controller_value = (controller_value*2-255)*invert;
  return controller_value;
}

void read_PS2(){
  ps2x.read_gamepad(); //Needs to be called at least once a second
  
  LY = fix_input(ps2x.Analog(PSS_LY),-1); //left Stick Up and Down  //Analog Stick readings
  LX = fix_input(ps2x.Analog(PSS_LX),1); //Left Stick Left and Right
  RY = fix_input(ps2x.Analog(PSS_RY),1); //Right Stick Up and Down
  RX = fix_input(ps2x.Analog(PSS_RX),1); //Right Stick Left and Right
  
}



int joystick2thrust(int value, int sign){
  int thrust = value;
  if (sign == 0){
     thrust = thrust * -1;
  }
  thrust = thrust + MOTOR_NUTURAL;
  return thrust;
}

void get_rotation(){
  if (RX >= DEAD_ZONE){
    motor_values[0] = MOTOR_FORWARD;
    motor_values[1] = MOTOR_FORWARD;
    motor_values[2] = MOTOR_FORWARD;
    motor_values[3] = MOTOR_FORWARD;
  }else if (RX <= -DEAD_ZONE){
    motor_values[0] = MOTOR_REVERSE;
    motor_values[1] = MOTOR_REVERSE;
    motor_values[2] = MOTOR_REVERSE;
    motor_values[3] = MOTOR_REVERSE;
  }
}
void get_tilt(){
  if (RY >= DEAD_ZONE){
    z_axis_motors[0] = MOTOR_FORWARD;
    z_axis_motors[1] = MOTOR_FORWARD;
    
  }else if (RY <= -DEAD_ZONE){
    z_axis_motors[0] = MOTOR_REVERSE;
    z_axis_motors[1] = MOTOR_REVERSE;
   
  }
 }

void get_z_axis(){
  z_axis_motors[0] = MOTOR_NUTURAL;
  z_axis_motors[1] = MOTOR_NUTURAL;
  if ( ps2x.Button(PSB_L2)){
    z_axis_motors[0] = MOTOR_FORWARD;
    z_axis_motors[1] = MOTOR_REVERSE;
  }
  if (ps2x.Button(PSB_R2)){
    z_axis_motors[0] = MOTOR_REVERSE;
    z_axis_motors[1] = MOTOR_FORWARD;
  }
  if ( ps2x.Button(PSB_L2) && ps2x.Button(PSB_R2) ){
     z_axis_motors[0] = MOTOR_NUTURAL;
     z_axis_motors[1] = MOTOR_NUTURAL;
 }
 //Serial.println(z_axis_motors[0]);
}

void get_motor_values(){ //this converts the controller input into the needed values for each thruster

  int motor_thrust = 0;
  int horz = EMPTY;
  int vert = EMPTY;
  
  motor_values[0] = MOTOR_NUTURAL ;
  motor_values[1] = MOTOR_NUTURAL ;
  motor_values[2] = MOTOR_NUTURAL ;
  motor_values[3] = MOTOR_NUTURAL ;
  
  if (LX >= DEAD_ZONE){//Right
    horz = 3;
  } 
  else if (LX <= -DEAD_ZONE){//Left
    horz = 12;
  } 
  if (LY >= DEAD_ZONE){//Up
    vert = 5;
  } 
  else if (LY <= -DEAD_ZONE){//Down
    vert = 10;
  } 
  if (abs(LX)<abs(LY)){
    motor_thrust = (min(abs(LY),200)/(float)200)*300;
  }else{
     //Serial.println(LX);
    motor_thrust = (min(abs(LX),200)/(float)200)*300;
  }
  
  if (horz != EMPTY and vert == EMPTY){
    for (i = 0;i<4; i++){
      motor_values[i] = joystick2thrust(motor_thrust,!!(horz & (1<<(3-i))));
      }
  }
  else if (vert != EMPTY and horz == EMPTY){
    for (i = 0;i<4; i++){
      motor_values[i] = joystick2thrust(motor_thrust,!!(vert & (1<<(3-i))));
      }
  }
  if (horz != EMPTY and vert != EMPTY){
    for (i = 0;i<4; i++){
      if ((vert & (1<<(3-i))) == (horz & (1<<(3-i)))){
        motor_values[i] = !!(horz & (1<<(3-i)));
        motor_values[i] = joystick2thrust(motor_thrust,!!(horz & (1<<(3-i))));
        
      }else {
        motor_values[i] = MOTOR_NUTURAL;
        }
    }
  }
  
  
}


void adjust_thrust(){//sets the thruster adjust variable if the up or down buttons are pressed
  if(ps2x.ButtonPressed(PSB_PAD_UP)){
    offset ++;
    if(offset>10){
      offset = 10;
    }
  }
  if(ps2x.ButtonPressed(PSB_PAD_DOWN)){
    offset --;
    if(offset<0){
      offset = 0;
    }
  }
}


void fillmessage(){//fills the array that will be sent over ethernet
  message[0] = motor_values[0]/100;//A
  message[1] = motor_values[1]/100;//B
  message[2] = motor_values[2]/100;//C
  message[3] = motor_values[3]/100;//D
  message[4] = z_axis_motors[0]/100;//E
  message[5] = z_axis_motors[1]/100;//F
  }



void sendmessage(){//sends the message
  Udp.beginPacket(destIp, port);
  Udp.write(message,arraylength);
  Udp.endPacket();
  //delay(50);
  }


void debug_inputs(){
  Serial.print("E   ");
  Serial.print(" | ");
  Serial.print("A   ");
  Serial.print(" | ");
  Serial.print("B   ");
  Serial.print(" | ");
  Serial.print("C   ");
  Serial.print(" | ");
  Serial.print("D   ");
  Serial.print(" | ");
  Serial.print("F   ");
  Serial.println("");
  Serial.print(z_axis_motors[0]);
  Serial.print(" | ");
  Serial.print(motor_values[0]);
  Serial.print(" | ");
  Serial.print(motor_values[1]);
  Serial.print(" | ");
  Serial.print(motor_values[2]);
  Serial.print(" | ");
  Serial.print(motor_values[3]);
  Serial.print(motor_values[3]);
  Serial.print(" | ");
  Serial.print(z_axis_motors[1]);
  Serial.println("");
  Serial.println("\n\n\n\n\n\n\n\n\n\n"); 
}
void print_sent(){
  Serial.println(" ");
  for(i = 0; i < arraylength ; i++){
    Serial.print(message[i]);
    
    Serial.print(" ");
  }
  Serial.println(motor_values[0]);
  Serial.println(motor_values[1]);
  Serial.println(motor_values[3]);
  Serial.println(motor_values[2]);
}
