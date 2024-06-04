#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
#include <U8x8lib.h>
#include "WiFi.h"
#include <HardwareSerial.h>
#include "DHT.h"

#define DHTPIN 15     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
#define HPMA_RX_PIN 16
#define HPMA_TX_PIN 17

//OLED面板
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);//(rotation, clock, data [, reset])
 
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

//宣告變數 
float h = 0;
float t = 0;
long lastMsg = 0;
char msg[50];
bool HPMAstatus = false;
const int ledPin = BUILTIN_LED;
int PM25;
int PM10;
 
DHT dht(DHTPIN, DHTTYPE);
 
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
 
void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");
}
 
void publishMessage()
{
  StaticJsonDocument<200> doc;
  doc["humidity"] = h;
  doc["temperature"] = t;
  doc["PM2.5"] = PM25;
  doc["PM10 "] = PM10;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}
 
void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);
 
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
}
 
void setup()
{
  Serial.begin(9600, SERIAL_8N1);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, 0);
  connectAWS();
  Serial.println("Starting HPMA115S0 initialization...");
  Serial2.begin(9600, SERIAL_8N1, HPMA_RX_PIN, HPMA_TX_PIN);
    while(!Serial2);
    Serial.println("retrying");
    start_autosend();
  Serial.println("HPMA started");
  delay(100);
  dht.begin();
  u8g2.begin();

}
 
void loop()
{
  delay(2000);
  long now = millis();
  if (now - lastMsg > 5000){
    lastMsg = now;
    HPMAstatus = receive_measurement();
    if (!receive_measurement()){
      digitalWrite(ledPin, 0);
      Serial.println("Cannot receive data from HPMA115S0!");
      return;
    }
    snprintf (msg, 16, "%D", PM25);
    snprintf (msg, 16, "%D", PM10);
    digitalWrite(ledPin, 1);
    Serial.println("PM 2.5:\t" + String(PM25) + " ug/m3");
    Serial.println("PM 10:\t" + String(PM10) + " ug/m3");
    h = dht.readHumidity();
    t = dht.readTemperature();
    float f = dht.readTemperature(true);
      if (isnan(h) || isnan(t) || isnan(f)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
      }
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.println(F("°C "));
    u8g2.firstPage();
    do {
      draw();
    } while( u8g2.nextPage() ); 

    publishMessage();
    client.loop();
    delay(1000);
  }  
}


bool receive_measurement (void)
{ 

  byte HEAD0 = Serial2.read();
  byte HEAD1 = Serial2.read();
  while (HEAD0 != 0x42) {
    if (HEAD1 == 0x42) {
      HEAD0 = HEAD1;
      HEAD1 = Serial2.read();
    } else {
      HEAD0 = Serial2.read();
      HEAD1 = Serial2.read();
    }
  }

  if (HEAD0 == 0x42 && HEAD1 == 0x4D) {
    byte LENH = Serial2.read();
    byte LENL = Serial2.read();
    byte Data0H = Serial2.read();
    byte Data0L = Serial2.read();
    byte Data1H = Serial2.read();
    byte Data1L = Serial2.read();
    byte Data2H = Serial2.read();
    byte Data2L = Serial2.read();
    byte Data3H = Serial2.read();
    byte Data3L = Serial2.read();
    byte Data4H = Serial2.read();
    byte Data4L = Serial2.read();
    byte Data5H = Serial2.read();
    byte Data5L = Serial2.read();
    byte Data6H = Serial2.read();
    byte Data6L = Serial2.read();
    byte Data7H = Serial2.read();
    byte Data7L = Serial2.read();
    byte Data8H = Serial2.read();
    byte Data8L = Serial2.read();
    byte Data9H = Serial2.read();
    byte Data9L = Serial2.read();
    byte Data10H = Serial2.read();
    byte Data10L = Serial2.read();
    byte Data11H = Serial2.read();
    byte Data11L = Serial2.read();
    byte Data12H = Serial2.read();
    byte Data12L = Serial2.read();
    byte CheckSumH = Serial2.read();
    byte CheckSumL = Serial2.read();
    if (((HEAD0 + HEAD1 + LENH + LENL + Data0H + Data0L + Data1H + Data1L + Data2H + Data2L + Data3H + Data3L + Data4H + Data4L + Data5H + Data5L + Data6H + Data6L + Data7H + Data7L + Data8H + Data8L + Data9H + Data9L + Data10H + Data10L + Data11H + Data11L + Data12H + Data12L) % 256) != CheckSumL){
      Serial.println("Checksum fail");
      return 0;
    }
    PM25 = (Data1H * 256) + Data1L;
    PM10 = (Data2H * 256) + Data2L;
    return 1;
  }
}

bool start_autosend(void)
{
  // Start auto send
  Serial.println("start_autosend");
  byte start_autosend[] = {0x68, 0x01, 0x40, 0x57 };
  Serial2.write(start_autosend, sizeof(start_autosend));
  Serial2.flush();
  delay(500);
  //Then we wait for the response
  while(Serial2.available() < 2);
  byte read1 = Serial2.read();
  byte read2 = Serial2.read();
  
  // Test the response
  if ((read1 == 0xA5) && (read2 == 0xA5)){
    // ACK
    return 1;
  }
  else if ((read1 == 0x96) && (read2 == 0x96))
  {
    // NACK
    return 0;
  }
  else{
    return 0;  
  } 
}

void draw(){
  u8g2.setFont(u8g2_font_unifont_t_greek);//設定字型
  u8g2.setCursor(0,12);//設定位置(0,16)
  u8g2.print("Humidity: ");//印出文字
  u8g2.print(h,0);//印出濕度
  u8g2.print("%");//印出單位
  
  u8g2.setCursor(0,24);//設定位置
  u8g2.print("Temperature: ");//印出文字
  u8g2.print(t,0);//印出溫度
  u8g2.print("°C");//印出單位

  u8g2.setCursor(0,36);//設定位置(0,16)
  u8g2.print("PM 2.5: ");//印出文字
  u8g2.print(PM25,0);//印出PM2.5
  u8g2.print(" ug/m3");//印出單位
  
  u8g2.setCursor(0,48);//設定位置
  u8g2.print("PM 10 : ");//印出文字
  u8g2.print(PM10,0);//印出PM10
  u8g2.print(" ug/m3");//印出單位 
}