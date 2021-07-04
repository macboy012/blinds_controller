#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266HTTPClient.h>

const char* ssid     = "No";
const char* password = "No";

const int DATA_PIN = 13;
const int LED_PIN = 0;

const int HIGH_LAG = 20;
const int SHORT_CYCLE_TIME = 320;
const int LONG_CYCLE_TIME = 640;

void write_bit(int val, int len) {
  if (val) {
      digitalWrite(DATA_PIN, HIGH);
      delayMicroseconds(len+HIGH_LAG);
      digitalWrite(DATA_PIN, LOW);
      delayMicroseconds(len-HIGH_LAG);

  }
  else {
      digitalWrite(DATA_PIN, LOW);
      delayMicroseconds(len-HIGH_LAG);
      digitalWrite(DATA_PIN, HIGH);
      delayMicroseconds(len+HIGH_LAG);
  }
}

void write_data_separator() {
  for (int i = 0; i < 11; i++) {
    write_bit(1, LONG_CYCLE_TIME);
  }
  digitalWrite(DATA_PIN, HIGH);
  delayMicroseconds(LONG_CYCLE_TIME*4);
  digitalWrite(DATA_PIN, LOW);
  delayMicroseconds(LONG_CYCLE_TIME*4);
}

void write_wakeup_preamble(int count) {
  for (int i = 0; i < count; i++) {
    write_bit(1, SHORT_CYCLE_TIME);
  }
}

const int UP_INDEX = 0;
const int DOWN_INDEX = 1;
const int UPPER_INDEX = 2;
const int LOWER_INDEX = 3;
const int STOP_INDEX = 4;


void write_command(int command_index, int repeats) {
  const char * COMMANDS[] = {
  "11111101" "10111001" "01100001" "01000000" "01000111" "00",
  "11111101" "10111001" "01100001" "01100000" "01100111" "00",
  "11111101" "10111001" "01100001" "00010010" "00010100" "10", // right tap (upper)
  "11111101" "10111001" "01100001" "01100010" "01100100" "10", // left tap (lower)
  "11111101" "10111001" "01100001" "00100000" "00100111" "00", // stop
  };
  digitalWrite(LED_PIN, LOW);
  Serial.println("Radio signaling begin");

  write_wakeup_preamble(498);
  for (int i = 0; i < repeats; i++) {
    Serial.println(i, DEC);

    write_data_separator();
    Serial.println("separator done");

    int chars = strlen(COMMANDS[command_index]);
    for (int j = 0; j < chars; j++) {
      write_bit('1' == COMMANDS[command_index][j] ? 1 : 0, LONG_CYCLE_TIME);
    }
    digitalWrite(DATA_PIN, LOW);
    delayMicroseconds(LONG_CYCLE_TIME*4);
    Serial.println("loop done");

  }
  digitalWrite(DATA_PIN, LOW);
  digitalWrite(LED_PIN, HIGH);
  Serial.println("Radio signaling done");
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);

  pinMode(DATA_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(DATA_PIN, LOW);
  digitalWrite(LED_PIN, HIGH);

  // We start by connecting to a WiFi network

  Serial.println();
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
  Serial.print("Netmask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
}


String get_blinds_command() {

  
  // Use WiFiClient class to create TCP connections
  WiFiClientSecure client;
  client.setInsecure();
  
  HTTPClient http;
  http.setTimeout(20000);
  bool working = http.begin(client, "No");
  if (working) {
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();
        Serial.println(payload);
        return payload;
      }
      else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
    }
  }
  else {
    Serial.println("not not not working");
  }
  return String("None");
}

void loop() {
  String command = get_blinds_command();
  if (command == "up") {
    write_command(UP_INDEX, 2);
  }
  else if (command == "down") {
    write_command(DOWN_INDEX, 2);
  }
  else if (command == "upper") {
    write_command(UPPER_INDEX, 26);
  }
  else if (command == "lower") {
    write_command(LOWER_INDEX, 26);
  }
  else if (command == "stop") {
    write_command(LOWER_INDEX, 2);
  }
  
}
