#include <IRremote.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const int ir_pin = 4; // NodeMCU D2
const char* ssid = "";
const char* password = "";

const uint16_t port = 5000;
const char* host = "192.168.1.133";
const char* _uri = "/api/bulb";
const bool https = false;


/*
 * Generic LCD Remote
 * Protocol - NEC
 * Keys
 *  Power - 0x12
 *  Video Select (Top) - 0x1B
 *  Mode Select (Bottom) - 0x1A
 *  Left Arrow - 0x4
 *  Right Arrow - 0x6
 *  Menu (Middle) - 0x5
 *
 */
const int BUTTON_POWER = 0x12;
const int BUTTON_VIDEO_SELECT = 0x1b;
const int BUTTON_MODE_SELECT = 0x1a;
const int BUTTON_LEFT_ARROW = 0x4;
const int BUTTON_RIGHT_ARROW = 0x6;
const int BUTTON_MENU = 0x5;


enum Action {
  ACTION_POWER_TOGGLE = 0,
  ACTION_BRIGHTNESS_MAX,
  ACTION_BRIGHTNESS_MIN,
  ACTION_MODE_WARM,
  ACTION_MODE_COOL,
  NONE = 100,
};

String requests[] = {
  "{\"action\":\"power\", \"value\": \"toggle\"}",
  "{\"action\":\"brightness\", \"value\": \"100\"}",
  "{\"action\":\"brightness\", \"value\": \"1\"}",
  "{\"action\":\"color_temp\", \"value\": \"1\"}",
  "{\"action\":\"color_temp\", \"value\": \"100\"}",
};

void signal_error() {
  for (int i=0;i<2;i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  IrReceiver.begin(ir_pin, DISABLE_LED_FEEDBACK);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("\tMAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("\tIP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("\tSSID: ");
  Serial.println(WiFi.SSID());
}

String action_to_string(Action action) {
  if (action == ACTION_POWER_TOGGLE) {
    return "Toggle Power";
  }
  if (action == ACTION_BRIGHTNESS_MAX) {
    return "Max Brightness";
  }
  if (action == ACTION_BRIGHTNESS_MIN) {
    return "Min Brightness";
  }
  if (action == ACTION_MODE_WARM) {
    return "Warm Color Temperature";
  }
  if (action == ACTION_MODE_COOL) {
    return "Cool Color Temperature";
  }
  return "";
}

int decode_button(IRData data) {
  if (
      (data.protocol == NEC) &&
      (!(data.flags & IRDATA_FLAGS_IS_REPEAT))
     )
  {
    IrReceiver.printIRResultShort(&Serial);
    return data.command;
  }
  return 0;
}

String send_http_request(Action action) {
  WiFiClient client;
  HTTPClient http;

  String result = "";
  
  if (http.begin(client, host, port, _uri, false)) {
    http.addHeader("Content-Type", "application/json");
    Serial.print("[HTTP] POST... request: ");
    Serial.println(requests[action]);
    int httpCode = http.POST(requests[action]);
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK) {
        const String& payload = http.getString();
        Serial.print("received payload: ");
        Serial.println(payload);
        result = payload;
      }
    } else {
      signal_error();
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.printf("[HTTP} Unable to connect\n");
  }
  return result;
}

void loop() {
  if (IrReceiver.decode()) {
    Action action = NONE;
    int button = decode_button(IrReceiver.decodedIRData);
    Serial.printf("Key pressed %#x\n", button);
    if (button) {
      if (button == BUTTON_POWER) {
        action = ACTION_POWER_TOGGLE;
      } else if (button == BUTTON_VIDEO_SELECT) {
        action = ACTION_BRIGHTNESS_MAX;
      } else if (button == BUTTON_MODE_SELECT) {
        action = ACTION_BRIGHTNESS_MIN;
      } else if (button == BUTTON_LEFT_ARROW) {
        action = ACTION_MODE_WARM;
      } else if (button == BUTTON_RIGHT_ARROW) {
        action = ACTION_MODE_COOL;
      } else {
        Serial.println("ERROR - no action found for key pressed");
      }
    }
    
    if (action != NONE) {
      Serial.printf("Executing action: ");
      Serial.println(action_to_string(action));
      send_http_request(action);
    }
    IrReceiver.resume();
  }
}
