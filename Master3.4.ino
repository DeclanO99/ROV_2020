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
const byte arraylength = 10, maximum = 255, minimum = 0, middle = (maximum-minimum)/2, deadzone = 30;
byte message[arraylength], packetBuffer[arraylength], offset = 10;
int left_magnitude, right_magnitude, LY, LX, RY, RX, mag, temperature, i;
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
  timer1 = millis();
  recieve();  //recieves the array from the slave arduino
//slave_data();   //prints out data from the temp and pH sensor when the triangle button is pushed  
  read_PS2();     //reads the PS2 values and adjusts the analog stick values to useful ranges
  buttons();      //allows the digital buttons to control tools, such as triangle, square, circle
  tilt_thrust();  //programs the thruster values to be able to tilt when needed
  adjust_thrust();//adjusts thrust according to the D-pad
  fillmessage();  //fills the array that is to be sent
  sendmessage();  //sends the array to the slave arduino
  
  
  //print_sent(); //prints out all the information to be sent in the array
  //print_recieved();//prints out the data that was recieved
  delay(50);
}

void recieve(){
  int packetSize = Udp.parsePacket();
  IPAddress remote = Udp.remoteIP();
  Udp.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE);
  temp = (float)packetBuffer[0] / 2;
  pH = (float)packetBuffer[1] / 18;
}

void slave_data(){
  if(ps2x.ButtonPressed(PSB_TRIANGLE)){
    sensors = 1;
    for(i = 0; i < 30; i++){
      Serial.print("Temperature: ");
      Serial.println(temp);
      Serial.print("pH: ");
      Serial.println(pH);
    }
  }
  else{
    sensors = 0;
  }
}


void read_PS2(){
  ps2x.read_gamepad(); //Needs to be called at least once a second
  
  LY = ps2x.Analog(PSS_LY); //left Stick Up and Down  //Analog Stick readings
  LX = ps2x.Analog(PSS_LX); //Left Stick Left and Right
  RY = ps2x.Analog(PSS_RY); //left Stick Up and Down
  RX = ps2x.Analog(PSS_RX); //Left Stick Left and Right
  
  LY = check_deadzone(LY);//sets to 0 if it's within a certain range
  LX = check_deadzone(LX);
  RY = check_deadzone(RY);
  RX = check_deadzone(RX);
  
  LY = swap(LY);
  RY = swap(RY);
  left_magnitude = magnitude(LY, LX);
}

void buttons(){
  if(ps2x.ButtonPressed(PSB_CROSS)){
    fry_release = !fry_release;
    Serial.print("Grout Fry Electromagnetic is: ");
    Serial.println(fry_release);
  }
  if(ps2x.ButtonPressed(PSB_CIRCLE)){
    if(mini_launch == false && mini_launch == true){
      Serial.println("You can't turn on and retract the mini rov at the same time");
    }
    else{
      mini_launch = !mini_launch;
      Serial.print("Mini bilge pump: ");
      Serial.println(mini_launch);
    }
  }
  if (ps2x.ButtonPressed(PSB_R2)){
    sensors =! sensors;
    timer2= millis();
  }
  
  if ((sensors==1) && ((timer1-timer2)<1000)){
    Serial.println(temp);
    Serial.println(pH);
  }
  if((sensors==1) && ((timer1-timer2)>1000)){
    sensors=0;
  }
  if(ps2x.ButtonPressed(PSB_SQUARE)){
    if(mini_retract == false && mini_retract == true){
      Serial.println("You can't turn on and retract the mini rov at the same time");
    }
    else{
      mini_retract = !mini_retract;
      Serial.print("Mini retract: ");
      Serial.println(mini_retract);
    }
  }
}

void tilt_thrust(){//changes the tilt variable if L2 is pressed
  if(ps2x.ButtonPressed(PSB_L2)){
    if(tilt == 1){
      tilt = 0;
    }
    else{
      tilt = 1;
    }
    Serial.print("Tilt: ");
    Serial.println(tilt);
  }
}

void adjust_thrust(){//sets the thruster adjust variable if the up or down buttons are pressed
  if(ps2x.ButtonPressed(PSB_PAD_UP)){
    offset ++;
    if(offset>10){
      offset = 10;
    }
    Serial.print("Thruster Offset: ");
    Serial.println(offset);
  }
  if(ps2x.ButtonPressed(PSB_PAD_DOWN)){
    offset --;
    if(offset<0){
      offset = 0;
    }
    Serial.print("Thruster Offset: ");
    Serial.println(offset);
  }
}

void fillmessage(){//fills the array that will be sent over ethernet
  if(LX == middle && LY == middle && RX == middle && RY == middle){
    message[0] = 0;
  }
  else if(left_magnitude>abs(RX-middle)){
    if(abs(LY-middle)>abs(LX-middle)){
      message[0] = 1;
      message[1] = LY;
    }
    else{
      message[0] = 2;
      message[1] = LX;
    }
  }
  else{
    message[0] = 3;
    message[1] = RX;
  }
  message[2] = RY;
  message[3] = tilt;
  message[4] = offset;
  message[5] = fry_release;
  message[6] = mini_launch;
  message[7] = mini_retract;
  message[8] = sensors;
}

void sendmessage(){//sends the message
  Udp.beginPacket(destIp, port);
  Udp.write(message,arraylength);
  Udp.endPacket();
  
  delay(50);
}

void print_sent(){
  Serial.println();
  for(i = 0; i < 9; i++){
    Serial.print(message[i]);
    Serial.print(" ");
  }
}

void print_recieved(){
  Serial.println();
  for(i = 0; i < arraylength; i++){
    Serial.print(packetBuffer[i]);
    Serial.print(" ");
  }
  
}

int check_deadzone(int controller_value){//if toggle is in deadband it sets the value to 0(middle variable)
  //returns the middle of the possible controller value if the controller value isn't past the deadzone area
  //Essentially making it so small unwanted movements in the controller won't cause the thrusters to move
  if(abs(controller_value-middle)>deadzone){
    controller_value = controller_value;
  }
  else{
    controller_value = middle;
  }
  return controller_value;
}

int magnitude(byte X, byte Y){
  //calculates the magnitude of the controller values
  mag = sqrt(sq(X- maximum/2)+sq(Y - maximum/2));  //calculates the magnitude of the directional vector using distance formula
  return mag;//gets it back to the actual value of the maximum
}//End Magnitude function

int swap(int controller_value){//switches the values from 255-0 to 0-255
  controller_value = abs(controller_value - maximum)-1;
  if(controller_value<0){
    controller_value=0;
  }
  return controller_value;
}
