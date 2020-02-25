#include <SPI.h>        
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Servo.h>

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

Servo thrusterA;
Servo thrusterB;
Servo thrusterC;
Servo thrusterD;
Servo thrusterE;
Servo thrusterF;

const int halt = 1500, arraylength = 7;
int stick_value, go, reverse, up_down, a, b, c, d, e, f, i;// value from the analog sticks from the controller
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
  //print_recieved(); // diagnostic tool to see the values being sent to the slave arduino
  set_thrusters();   //takes the data from the array and sends out the respective pulse to the individual ESC's
  //print_values(); //diagnostic tool to see the motor values being output to the motor ESC's
  delay(50);

}

void recieve(){//recieves data from ethernet and puts it in packetBuffer array
  int packetSize = Udp.parsePacket();
  IPAddress remote = Udp.remoteIP();
  Udp.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE);
  
  a = packetBuffer[4]*100;
  b = packetBuffer[1]*100;
  c = packetBuffer[2]*100;
  d = packetBuffer[3]*100;
  e = packetBuffer[0]*100;
  f = packetBuffer[5]*100;
}

void set_thrusters(){
  thrusterA.writeMicroseconds(a);
  thrusterB.writeMicroseconds(b);  
  thrusterC.writeMicroseconds(c);  
  thrusterD.writeMicroseconds(d);
  thrusterE.writeMicroseconds(e);
  thrusterF.writeMicroseconds(f);
}


void print_recieved(){
  Serial.println();
  for(i = 0; i < arraylength; i++){
    Serial.print(packetBuffer[i]);
    Serial.print(" ");
  }
}

void print_values(){  //diagnostic tool to see the motor values being output to the motor ESC's
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
}
