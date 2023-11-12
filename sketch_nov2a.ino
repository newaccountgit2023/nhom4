#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
 
const char* ssid = "HKDI";
const char* password = "minh2001";
 
#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_USER "pdk310201"
#define MQTT_PASSWORD "Quenmatroi@1305"
 
#define MQTT_PUB_TOPIC "tb/mqtt-integration-torial/devices/test_light/state"
#define MQTT_SUB_TOPIC "tb/mqtt-integration-tutorial/devices/test_light/rx"
#define MQTT_SUB_TOPIC_SW4 "tb/mqtt-integration-tutorial/devices/sw_4/rx"
#define MQTT_PUB_TOPIC_SW4 "tb/mqtt-integration-torial/devices/sw_4/state"

//khai biến cho phần kết nối k có internet
#define SWITCH_PIN 2 //chân kết nối của switch với Arduino
#define SWITCH_PIN_2 32 

uint8_t trig = 19;
uint8_t echo = 21;
uint8_t pir = 18;
uint8_t relay = 33;
//uint8_t sound = A3;
uint8_t led_1 = 17;
uint8_t led_2 = 26;

uint8_t val_pir, switchVal_1, switchVal_2 ;
uint8_t flag_sw3, flag_sw4;  
long duration;
short distance;

unsigned long lastTime;
unsigned long elapsedTime = 0; 

unsigned long previousMillis = 0;
const long interval = 100;

const short MAX_DISTANCE = 50;
const int MAX_TIME = 3000;

 
WiFiClient wifiClient;
PubSubClient client(wifiClient);
 
 
void setup_wifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  //Serial.println("");
  Serial.println("WiFi connected");
  //Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
 
void connect_to_broker() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
      client.subscribe(MQTT_SUB_TOPIC);
      client.subscribe(MQTT_SUB_TOPIC_SW4);
      //client.subscribe(MQTT_PUB_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      //Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}
 
void callback(char* topic, byte *payload, unsigned int length) {
  char status[12];
  Serial.println("-------new message from broker-----");
  Serial.print("topic: ");
  Serial.println(topic);
  Serial.print("message: ");
  Serial.write(payload, length);
  Serial.println();
  for(int i = 0; i<length; i++)
  {
    status[i] = payload[i];
  }
  Serial.println(status);
  if(String(topic) == MQTT_SUB_TOPIC)
  {
    if(String(status) == "{\"value1\":0}")
    {
      Serial.println("LED1 OFF");
      //digitalWrite(relay, LOW);
      client.publish(MQTT_PUB_TOPIC, "{\"value1\":0}");
      flag_sw3 = 0;
    }
    else if(String(status) == "{\"value1\":1}")
    {
      Serial.println("LED1 ON");
      //digitalWrite(relay, HIGH);
      client.publish(MQTT_PUB_TOPIC, "{\"value1\":1}");
      flag_sw3 = 1;
    }
  }else if(String(topic) == MQTT_SUB_TOPIC_SW4)
  {
    if(String(status) == "{\"value1\":0}")
    {
      Serial.println("sw4 OFF");
      //digitalWrite(relay, LOW);
      client.publish(MQTT_PUB_TOPIC_SW4, "{\"value1\":0}");
      flag_sw4 = 0;
    }
    else if(String(status) == "{\"value1\":1}")
    {
      Serial.println("sw4 ON");
      //digitalWrite(relay, HIGH);
      client.publish(MQTT_PUB_TOPIC_SW4, "{\"value1\":1}");
      flag_sw4 = 1;
    }
  }
}

 
void setup() {
  Serial.begin(9600);
  Serial.setTimeout(500);
  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT );
  client.setCallback(callback);
  connect_to_broker();
  Serial.println("Start transfer");
  client.publish(MQTT_PUB_TOPIC_SW4, "{\"value1\":0}");
  //Phần k internet
  pinMode(SWITCH_PIN, INPUT_PULLUP); //kết nối switch dưới chế độ INPUT_PULLUP
  pinMode(SWITCH_PIN_2 , INPUT_PULLUP);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(pir, INPUT);
  //pinMode(sound, INPUT);
  pinMode(relay, OUTPUT);
  pinMode(led_1, OUTPUT);
  pinMode(led_2, OUTPUT);
}

  
void loop() {
  client.loop();
  if (!client.connected()) {
    connect_to_broker();
  }
  //Serial.println(flag_sw4);
  switchVal_1 = digitalRead(SWITCH_PIN);
  switchVal_2 = digitalRead(SWITCH_PIN_2);

  if (switchVal_1 == LOW ) {
    digitalWrite(led_1, HIGH);
    //digitalWrite(relay, HIGH);
    //flag_sw3 = 1;
    client.publish(MQTT_PUB_TOPIC, "{\"value1\":1}");
    if (switchVal_2 == HIGH) {
      digitalWrite(relay, HIGH);
      //flag_sw3 = true;
      digitalWrite(led_2, LOW);
    } else {
      digitalWrite(led_2, HIGH);
      client.publish(MQTT_PUB_TOPIC, "{\"value1\":1}");
      if(flag_sw3 == 1){
        if(flag_sw4 == 0){
          digitalWrite(relay, HIGH);
        }else{

          digitalWrite(trig, LOW);
          delayMicroseconds(2);
          digitalWrite(trig, HIGH);
          delayMicroseconds(10);
          duration = pulseIn(echo, HIGH);
          distance = duration * 0.034 / 2;

          val_pir = digitalRead(pir);
          Serial.println(distance);
          if (val_pir == HIGH  || distance <= MAX_DISTANCE) {
            digitalWrite(relay, HIGH);
            lastTime = millis();
            elapsedTime = 0;
          }else{
            elapsedTime = millis() - lastTime;
            if (elapsedTime > MAX_TIME) {
              digitalWrite(relay, LOW);
            }
          }
          if (elapsedTime > MAX_TIME && (val_pir == HIGH || distance <= MAX_DISTANCE )) {
            lastTime = millis();
            elapsedTime = 0;
          }
        }
        
        
      }else{
        digitalWrite(relay, LOW);
      }
    }
  } else {
    client.publish(MQTT_PUB_TOPIC, "{\"value1\":0}");
    digitalWrite(led_1, LOW);
    digitalWrite(led_2, LOW);
    digitalWrite(relay, LOW);
    //flag_sw3 = 0;
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
  } 

}


