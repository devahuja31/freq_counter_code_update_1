#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

volatile uint64_t PeriodCount;    
const char* ssid = "sd";
const char* password = "asdfsdaf";

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "78.156.102.68";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
float Freg=0;  
const int ledPin = 4;


void setup() 
{
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(ledPin, OUTPUT);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(ledPin, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(ledPin, LOW);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
  error_loop_again:
  Freg =1000000.00/PeriodCount;
  Freg=get_freqq(20);
  if(Freg>51){goto error_loop_again;}
  unsigned int volt_read=get_volt();
 
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
   
    char freqString[8];
    dtostrf(Freg, 1, 3, freqString);
    Serial.print("Freq: ");
    
    client.publish("mb/Freq",freqString);
  }
}

unsigned get_volt()
{
  long t101=micros();
  float vr=0;
  unsigned int volt_sample_c=0;
  while(micros()-t101<20000)
  {
    vr+=analogRead(36);
    volt_sample_c++;
    delayMicroseconds(100);
  }
vr=(float)vr/volt_sample_c;
if(vr<50)
{vr=0;
  
  }else{
vr=0.2141*vr +49.71;
vr=1.05*vr+5.082;
}
vr=vr/2.03;
//Serial.println(vr,3);
return(int(vr));
}


float get_freqq(unsigned int number_sample)
{
  unsigned int ty=0;
  float freq_a=0;
  for(ty=0;ty<number_sample;ty++)
  {
  long highp=pulseIn(34, HIGH);
  long highl=pulseIn(34, LOW);
  long ttt=highp+highl;
  float ff=(float)ttt/1000000;
  ff=(float)1/ff;  
  freq_a=freq_a+ff;
  }
  return(freq_a/number_sample);
}
  
  
