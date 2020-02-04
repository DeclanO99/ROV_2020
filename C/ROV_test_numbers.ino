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
const byte arraylength = 7, maximum = 255, minimum = 0, middle = 127, deadzone = 20, halt =1500;
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

void read_PS2(){
  ps2x.read_gamepad(); //Needs to be called at least once a second
  
  LY = ps2x.Analog(PSS_LY); //left Stick Up and Down  //Analog Stick readings
  LX = ps2x.Analog(PSS_LX); //Left Stick Left and Right // 0-255 return range
  //RY = ps2x.Analog(PSS_RY); //Right Stick Up and Down
  //RX = ps2x.Analog(PSS_RX); //Right Stick Left and Right
  
  LY = check_deadzone(LY,-1);//sets to middle if it's within dead zone
  LX = check_deadzone(LX,1);
//  RY = check_deadzone(RY);
//  RX = check_deadzone(RX);
  
  //LY = swap(LY);
  //RY = swap(RY);
//  left_magnitude = magnitude(LY, LX);
}
int swap(int controller_value){//switches the values from 255-0 to 0-255
  controller_value = abs(controller_value - maximum)-1;
  if(controller_value<0){
    controller_value=0;
  }
  return controller_value;
}

int check_deadzone(int controller_value, int inverse){//if toggle is in deadband it sets the value to 0(middle variable)
  //returns the middle of the possible controller value if the controller value isn't past the deadzone area
  //Essentially making it so small unwanted movements in the controller won't cause the thrusters to move
  return (controller_value * 2 - 255) * inverse; 
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
  if (abs(LX) >=50  || abs(LY) >= 50){
    Serial.println(" ");
    Serial.print(LX);
    Serial.print(",");
    Serial.print(LY);
 // Serial.println(RX);
  }
}
