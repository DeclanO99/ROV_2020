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

int timer1;
int timer2;
int A,B,C,D,E,F, twist, vertical, LY, LX, RY, RX, movementY;
const byte arraylength = 6, middle = 0, deadzone = 20;
byte message[arraylength], packetBuffer[arraylength], offset = 10;
int left_magnitude, right_magnitude,  mag, temperature, i;
float temp, pH;
bool L2P, L2R, R2P, R2R;

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
  timer1 = millis();
//slave_data();   //prints out data from the temp and pH sensor when the triangle button is pushed  
  read_PS2();     //reads the PS2 values and adjusts the analog stick values to useful ranges
//adjust_thrust();//adjusts thrust according to the D-pad
//  motor_values(); //converts PS2 vectors into values for the speed controllers
  fillmessage();  //fills the array that is to be sent
  sendmessage();  //sends the array to the slave arduino
//  print_sent(); //prints out all the information to be sent in the array
  delay(50);
}


void read_PS2(){
  ps2x.read_gamepad(); //Needs to be called at least once a second
  
  LY = ps2x.Analog(PSS_LY); //left Stick Up and Down  //Analog Stick readings
  LX = ps2x.Analog(PSS_LX); //Left Stick Left and Right
  RY = ps2x.Analog(PSS_RY); //Right Stick Up and Down
  RX = ps2x.Analog(PSS_RX); //Right Stick Left and Right
  
  LY = fix_input(LY);//sets to middle if it's within dead zone
  LX = fix_input(LX);
  RY = fix_input(RY);
  RX = fix_input(RX);

  L2P = ps2x.ButtonPressed(PSB_L2);
  R2P = ps2x.ButtonPressed(PSB_R2);
  L2R = ps2x.ButtonReleased(PSB_L2);
  R2R = ps2x.ButtonReleased(PSB_R2);
  
  if ( L2P ){
    movementY = -1;
  }
  else if ( L2R ){
    movementY = 0;
  }
  if ( R2P ){
    movementY = 1;
  }
  else if ( R2R ){
    movementY = 0;
  }
 Serial.print(movementY);
 }



void motor_values(){ //this converts the controller input into the needed values for each thruster




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
  message[0] = A;
  message[1] = B;
  message[2] = C;
  message[3] = D;
  message[4] = E;
  message[5] = F;
  }



void sendmessage(){//sends the message
  Udp.beginPacket(destIp, port);
  Udp.write(message,arraylength);
  Udp.endPacket();
  delay(50);
  }



void print_sent(){
  Serial.println();
  Serial.print(movementY);
  for(i = 0; i < arraylength-1 ; i++){
    Serial.print(message[i]);
    Serial.print(" ");
  }
}


int fix_input(int controller_value){//checks to see if the analog stick is within a center range and if it is sets the value to 0 and if not simply remaps the input value
  if(abs(controller_value-middle)>deadzone){
    controller_value = controller_value*2-255;
  }
  else{
    controller_value = middle;
  }
  return controller_value;
}
