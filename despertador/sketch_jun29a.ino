#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ArduinoJson.h>
#include <vector>
#include <algorithm>

void updateDisplay(bool forceUpdate = false);
void displayStatus(const char* message);
void connectWiFi();
void callback(char* topic, byte* payload, unsigned int length);
void addAlarm(int hour, int minute, String medicine);
void reconnect();
void triggerAlarm();
void checkAlarms();

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BUZZER_PIN 4
#define LED_PIN 2
#define BUTTON_PIN 5

const char* ssid = "SEU-WIFI";
const char* pass = "SUA-SENHA";

const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_client_id = "ESP32_Medicine_Reminder";

const char* topic_add = "medicine_reminder/add";
const char* topic_clear = "medicine_reminder/clear";
const char* topic_status = "medicine_reminder/status";

struct Alarm {
  int hour;
  int minute;
  String medicine;
};

std::vector<Alarm> alarms;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -3 * 3600);
WiFiClient espClient;
PubSubClient client(espClient);

bool alarmActive = false;
unsigned long lastDisplayUpdate = 0;
const unsigned long displayUpdateInterval = 1000;

void setup() {
  Serial.begin(115200);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while(1);
  }
  
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  connectWiFi();
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  timeClient.begin();
  timeClient.update();
  
  displayStatus("Sistema Pronto");
}

void connectWiFi() {
  WiFi.begin(ssid, pass);
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Conectando WiFi:");
  display.println(ssid);
  display.display();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    display.print(".");
    display.display();
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String message = (char*)payload;

  if (strcmp(topic, topic_add) == 0) {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload, length);
    addAlarm(doc["hour"], doc["minute"], doc["medicine"].as<String>());
  }
  else if (strcmp(topic, topic_clear) == 0) {
    alarms.clear();
    client.publish(topic_status, "Alarmes removidos");
    updateDisplay(true);
  }
}

void addAlarm(int hour, int minute, String medicine) {
  for (const auto& alarm : alarms) {
    if (alarm.hour == hour && alarm.minute == minute) {
      return;
    }
  }

  alarms.push_back({hour, minute, medicine});
  
  std::sort(alarms.begin(), alarms.end(), [](const Alarm &a, const Alarm &b) {
    return (a.hour < b.hour) || (a.hour == b.hour && a.minute < b.minute);
  });

  updateDisplay(true);
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect(mqtt_client_id)) {
      client.subscribe(topic_add);
      client.subscribe(topic_clear);
      client.publish(topic_status, "ESP32 Reconectada");
    } else {
      delay(5000);
    }
  }
}

void triggerAlarm() {
  unsigned long startTime = millis();
  bool interrupted = false;
  
  while ((millis() - startTime < 30000) && !interrupted) {
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    unsigned long toneStart = millis();
    while (millis() - toneStart < 1000 && !interrupted) {
      if (digitalRead(BUTTON_PIN) == LOW) {
        interrupted = true;
      }
      delay(10);
    }
    
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    unsigned long pauseStart = millis();
    while (millis() - pauseStart < 1000 && !interrupted) {
      if (digitalRead(BUTTON_PIN) == LOW) {
        interrupted = true;
      }
      delay(10);
    }
  }
  
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  alarmActive = false;
}

void updateDisplay(bool forceUpdate) {
  unsigned long currentMillis = millis();
  
  if (forceUpdate || (currentMillis - lastDisplayUpdate >= displayUpdateInterval)) {
    lastDisplayUpdate = currentMillis;
    
    timeClient.update();
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    
    display.setCursor(0,0);
    display.print("Hora: ");
    int hours = timeClient.getHours();
    int mins = timeClient.getMinutes();
    int secs = timeClient.getSeconds();
    
    display.print(hours);
    display.print(":");
    if (mins < 10) display.print("0");
    display.print(mins);
    display.print(":");
    if (secs < 10) display.print("0");
    display.println(secs);
    
    display.println("Proximos Alarmes:");
    
    int yPos = 24;
    for (size_t i = 0; i < alarms.size() && yPos < SCREEN_HEIGHT; i++) {
      display.setCursor(0, yPos);
      display.print(alarms[i].hour);
      display.print(":");
      if (alarms[i].minute < 10) display.print("0");
      display.print(alarms[i].minute);
      display.print(" - ");
      display.println(alarms[i].medicine.substring(0, 15));
      yPos += 10;
    }
    
    if (alarms.empty()) {
      display.println("Nenhum alarme");
    }
    
    display.display();
  }
}

void checkAlarms() {
  timeClient.update();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();

  for (auto it = alarms.begin(); it != alarms.end(); ) {
    if (it->hour == currentHour && it->minute == currentMinute) {
      alarmActive = true;
      triggerAlarm();
      it = alarms.erase(it);
      updateDisplay(true);
    } else {
      ++it;
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  updateDisplay();
  checkAlarms();
}

void displayStatus(const char* message) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println(message);
  display.display();
}