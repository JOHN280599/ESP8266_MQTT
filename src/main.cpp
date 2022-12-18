#include <Arduino.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

SoftwareSerial nodemcu(D1, D2); // khai báo chân truyền thông cho esp D6 là rx còn D5 là tx
WiFiClient client;
PubSubClient mqtt_client(client);

int fixwifi = 0;
int mqttconnect = 0;

const char *server_mqtt = "testdoan1.cloud.shiftr.io";
const int port_mqtt = 1883;
const char *mqtt_id = "esp8266";
const char *mqtt_pass = "7ERKBmTKLLrl5EYn"; // Pass đăng nhập                               "sHPBQ1hBZ1cHM6j2";
const char *mqtt_name = "testdoan1";        // tên User
const char *topic_subscrise = "TRoom1";     // Topic nhận dữ liệu
const char *topic_Temp1 = "Room1";          // Topic gửi dữ liệu
unsigned long timerr = 0;
unsigned long count = 0;
int wificnt = 0;
float temp[4];
float humi[4];
float timee[4];
float op[4];
int x;
String payload;

void callback(char *topic, byte *payload, unsigned int length)
{
  StaticJsonDocument<1000> doc;
  deserializeJson(doc, payload);
  float aaa = doc["GT"];
  Serial.print("message: ");
  Serial.println(aaa);
  serializeJson(doc, nodemcu);
}

void setup()
{
  pinMode(D0, OUTPUT);
  Serial.begin(9600);
  nodemcu.begin(9600);
  digitalWrite(D0, LOW);
}

void sendtoserver()
{
  // gửi tín hiệu cho mqtt
  if (mqtt_client.connected())
  {
    mqtt_client.publish(topic_Temp1, payload.c_str());
    if (mqttconnect == 1)
    {
      mqtt_client.subscribe(topic_subscrise);
      Serial.println("Connected");
      digitalWrite(D0, HIGH);
      mqttconnect = 0;
    }
  }
  else
  {
    Serial.println("Disconnected");
    digitalWrite(D0, LOW);
    mqtt_client.connect(mqtt_id, mqtt_name, mqtt_pass);
    mqttconnect = 1;
  }
}

void connectingwifi()
{
  delay(1000);
  mqtt_client.setServer(server_mqtt, port_mqtt);
  mqtt_client.setCallback(callback);
  while (!mqtt_client.connect(mqtt_id, mqtt_name, mqtt_pass))
  {
    delay(500);
  }
  Serial.println("Connected to mqtt");
  mqtt_client.subscribe(topic_subscrise);
  fixwifi = 0;
}

void loop()
{
  StaticJsonDocument<1000> doc2;
  DeserializationError error = deserializeJson(doc2, nodemcu);
  // if ((unsigned long)(millis() - timerr) >= 500)
  // {
  if (!error)
  {
    x = doc2["x"];
    if (x == 3)
    {
      wificnt = 1;
      const char *ssid = doc2["N"];
      const char *password = doc2["P"];
      Serial.println("Connecting to wifi...");
      WiFi.begin(ssid, password);
      WiFi.reconnect();
      while (WiFi.status() != WL_CONNECTED)
      {
        delay(500);
        Serial.print(".");
        yield();
      }
      if (x != 4)
      {
        Serial.print("Connected to WiFi");
        Serial.print(ssid);
        Serial.print("Ip Address: ");
        Serial.print(WiFi.localIP());
        Serial.println();
        fixwifi = 1;
      }
      else if (x == 4)
      {
        fixwifi = 0;
        Serial.print("Disconnect WiFi");
      }
    }
    else if (x == 1 || x == 2)
    {
      temp[0] = doc2["t1"];
      temp[1] = doc2["t2"];
      temp[2] = doc2["t3"];
      temp[3] = doc2["t4"];
      humi[0] = doc2["h1"];
      humi[1] = doc2["h2"];
      humi[2] = doc2["h3"];
      humi[3] = doc2["h4"];
      payload = "{\"x\":";
      payload += x;
      payload += ",\"temp1\":";
      payload += temp[0];
      payload += ",\"temp2\":";
      payload += temp[1];
      payload += ",\"temp3\":";
      payload += temp[2];
      payload += ",\"temp4\":";
      payload += temp[3];
      payload += ",\"humi1\":";
      payload += humi[0];
      payload += ",\"humi2\":";
      payload += humi[1];
      payload += ",\"humi3\":";
      payload += humi[2];
      payload += ",\"humi4\":";
      payload += humi[3];
      payload += "}";
      // Serial.println(payload.c_str());
    }
    else if (x == 4)
    {
      wificnt = 0;
      WiFi.disconnect();
      fixwifi = 0;
      Serial.print("Disconnect WiFi");
    }
    else if (x == 5)
    {
      op[0] = doc2["op1"];
      op[1] = doc2["op2"];
      op[2] = doc2["op3"];
      timee[0] = doc2["h"];
      timee[1] = doc2["p"];
      payload = "{\"x\":";
      payload += x;
      payload += ",\"op1\":";
      payload +=  op[0];
      payload += ",\"op2\":";
      payload +=  op[1];
      payload += ",\"op3\":";
      payload +=  op[2];
      payload += ",\"h\":";
      payload += timee[0];
      payload += ",\"p\":";
      payload += timee[1];
      payload += "}";
    }
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("WIFI lost");
    // WiFi.reconnect();
    // fixwifi = 1;
    // while (WiFi.status() != WL_CONNECTED)
    // {
    //   delay(500);
    //   Serial.print(".");
    //   StaticJsonDocument<1000> doc2;
    //   DeserializationError error = deserializeJson(doc2, nodemcu);
    //   if (!error)
    //   {
    //     x = doc2["x"];
    //     if (x == 3)
    //     {
    //       const char *ssid = doc2["N"];
    //       const char *password = doc2["P"];
    //       Serial.println("Connecting to wifi...");
    //       WiFi.begin(ssid, password);
    //       WiFi.reconnect();
    //       fixwifi = 1;
    //     }
    //     else if (x == 4)
    //     {
    //       WiFi.disconnect();
    //       break;
    //     }
    //   }
    // }
  }
  if (fixwifi == 1)
  {
    connectingwifi();
  }

  mqtt_client.loop();
  sendtoserver();
  yield();
}
