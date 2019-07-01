///////////////////////////////////////////////////////////////////////////////////////
/*   Written by Ivan Scordato
*   Search for "ADAFRUIT_USERNAME" and "ADAFRUIT TOKEN" and edit with your personal data.
*   Many serial output has been disabled. Uncomment for debug.
*/
///////////////////////////////////////////////////////////////////////////////////////

/*________________________ INCLUDE LIBRARIES  _____________________________*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/*________________________ PIN DEFINITIONS  _____________________________*/
#define relay1 D2
#define input1 D3
/*________________________ RELAY INITIAL STATUS  _____________________________*/
int relay1_status = 0;

/*________________________ BUTTONS VARIABLES  _____________________________*/
long unsigned int lowIn;         
long unsigned int pausa = 100;  

boolean lockLow1 = true;
boolean takeLowTime1;  
//////////////////////////////////////////////////////////////////////////////////////////////////////

/*________________________ MIX  _____________________________*/
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// Input the SSID and password of WiFi
const char* ssid     = "SSID WIFI";
const char* password = "PWD";


void setup() {
  Serial.begin(115200);
  
/*________________________ CONNECTING TO WIFI  _____________________________*/
Serial.println("Starting connecting WiFi.");
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); 
   
   /*________________________ RELAYS OUTPUT AND INITIAL STATE SETUP  _____________________________*/
  
    pinMode(relay1, OUTPUT);
    digitalWrite(relay1, HIGH); // HIGH = relay off
    
/*________________________ BUTTON INPUT SETUP  _____________________________*/
  pinMode(input1, INPUT_PULLUP);

/////////////////////////////////////////////////////////////////////////////
client.setServer("io.adafruit.com", 1883);
  client.setCallback(callback);  
}

void callback(char* topic, byte* message, unsigned int length) {
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    //Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  //Serial.println();

 /*________________________ MQTT TOPIC RELAY CONTROL  _____________________________*/
//********************* Relay 1 ***********************
  if (String(topic) == "ADAFRUIT_USERNAME/feeds/control") {
    //Serial.print("Changing relay state to ");
    
    if(messageTemp == "relay1ON"){
      //Serial.println("1on");
      relay1_status = 1;
    }
    if(messageTemp == "relay1OFF"){
      //Serial.println("1off");
      relay1_status = 0;
    }
  }
 }

void loop() {
  // put your main code here, to run repeatedly:
  if(WiFi.status() != WL_CONNECTED) {
  Serial.println("WIFI NOT CONNECTED! RESTART");
  ESP.restart();
}

  if (!client.connected()) {
    reconnect();
  }  

  long now = millis();
  if (now - lastMsg > 150) {
    lastMsg = now;

//********************* INPUT1 ***********************
  if(digitalRead(input1) == 0){
    if(lockLow1){  
      //makes sure we wait for a transition to LOW before any further output is made:
      lockLow1 = false;            
      if(relay1_status == 0){
      client.publish("ADAFRUIT_USERNAME/feeds/control", "relay1ON");}
      else  {
        client.publish("ADAFRUIT_USERNAME/feeds/control", "relay1OFF");}
      
 
      Serial.println("Button 1 pressed");
      delay(50);
    }         
    takeLowTime1 = true;
  }

  if(digitalRead(input1) == 1){      
    if(takeLowTime1){
      lowIn = millis();
      takeLowTime1 = false;
    }
    if(!lockLow1 && millis() - lowIn > pausa){  
      lockLow1 = true;                        
      //client.publish("ADAFRUIT_USERNAME/input/input1", "OPEN"); // Maybe a window contact...
      delay(50);
    }
  }  
  }
            
/*________________________ RELAYS STATUS SETTING  _____________________________*/
  if(relay1_status == 1){
    digitalWrite(relay1, LOW);
  }
    else { digitalWrite(relay1, HIGH); }

client.loop();  
}

void reconnect() {
  if(WiFi.status() != WL_CONNECTED) {
  Serial.println("NOT CONNECTED!!");
  ESP.restart();}
  
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("espClient", "****ADAFRUIT USERNAME****", "****ADAFRUIT TOKEN****")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("ADAFRUIT_USERNAME/feeds/control");
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
