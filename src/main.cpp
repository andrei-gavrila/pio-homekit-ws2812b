#include <Arduino.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include <WS2812FX.h>
#include <ESP8266WebServer.h>
#include <arduino_homekit_server.h>

struct
{
  char checksum[10] = {0};

  uint8_t mode = 0;
  uint16_t speed = 1000;
} eeprom_settings;

WS2812FX ws2812fx = WS2812FX(8, D1, NEO_GRB + NEO_KHZ800);

extern "C" void ws2812b_fx_set(bool, uint8_t, uint8_t, uint8_t, uint8_t);

void ws2812b_fx_set(bool on, uint8_t brightness, uint8_t led_r, uint8_t led_g, uint8_t led_b)
{
  Serial.printf("ws2812b_fx_set() - on: %d, led_r: %d, led_g: %d, led_b: %d\n", on, led_r, led_g, led_b);

  if (on)
  {
    ws2812fx.setMode(eeprom_settings.mode);

    ws2812fx.setBrightness(255 / 100 * brightness);

    ws2812fx.setColor(led_r, led_g, led_b);
  }
  else
  {
    ws2812fx.setMode(0);

    ws2812fx.setBrightness(0);

    ws2812fx.setColor(0, 0, 0);
  }
}

ESP8266WebServer server(8080);

void srv_handle_index_html()
{
  String tmp = "";

  tmp += "\
  <!doctype html>\
<html>\
  <head>\
    <title>HomeKit WS2812b</title>\
\
    <style>\
body {\
  font-family: Verdana, sans-serif;\
\
  color: #000000;\
}\
\
.speed {\
  width: 300px;\
}\
\
.speed_text {\
  width: 50px;\
}\
\
.mode {\
  width: 360px;\
}\
    </style>\
\
    <script type='text/javascript'>\
function submitVal(name, val) {\
  var xhttp = new XMLHttpRequest();\
\
  xhttp.open('GET', 'set?' + name + '=' + val, true);\
\
  xhttp.send();\
}\
	</script>\
  </head>\
\
  <body>\
<h1>HomeKit WS2812b</h1>\
\
Speed<br/>\
<input type=\"range\" min=\"2\" max=\"65535\" value=\"";
tmp += ws2812fx.getSpeed();
tmp += "\" class=\"speed\" onChange=\"submitVal('s', this.value); this.nextElementSibling.value = this.value;\"><input type=\"text\" value=\"";
tmp += ws2812fx.getSpeed();
tmp += "\" class=\"speed_text\" onChange=\"submitVal('s', this.value); this.previousElementSibling.value = this.value;\"><br />\
Mode<br />\
<select class=\"mode\" onChange=\"submitVal('m', this.options[this.selectedIndex].value);\">";
  for (uint8_t i = 0; i < ws2812fx.getModeCount(); i++)
  {
    tmp += "<option value=\"";
    tmp += i;
    tmp += "\"";
    if (i == ws2812fx.getMode())
    {
      tmp += " selected";
    }
    tmp += ">";
    tmp += ws2812fx.getModeName(i);
    tmp += "</option>";
  }
  tmp += "</select><br />\
  </body>\
</html>";

  server.send(200, "text/html", tmp);
}

void srv_handle_set()
{
  for (uint8_t i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "m")
    {
      uint8_t tmp = (uint8_t)strtol(server.arg(i).c_str(), NULL, 10);

      ws2812fx.setMode(tmp);

      eeprom_settings.mode = tmp;

      EEPROM.put(1408, eeprom_settings);

      EEPROM.commit();

      Serial.println("srv_handle_set() - (set mode) - EEPROM settings:");
      Serial.printf("eeprom_settings.mode: %d\n", eeprom_settings.mode);
      Serial.printf("eeprom_settings.speed: %d\n", eeprom_settings.speed);
    }

    if (server.argName(i) == "s")
    {
      uint16_t tmp = (uint16_t) strtol(server.arg(i).c_str(), NULL, 10);

      ws2812fx.setSpeed(tmp);

      eeprom_settings.speed = tmp;

      EEPROM.put(1408, eeprom_settings);

      EEPROM.commit();

      Serial.println("srv_handle_set() - (set speed) - EEPROM settings:");
      Serial.printf("eeprom_settings.mode: %d\n", eeprom_settings.mode);
      Serial.printf("eeprom_settings.speed: %d\n", eeprom_settings.speed);
    }
  }

  server.send(200, "text/plain", "OK");
}

extern "C" void my_acessory_init(void);

extern "C" homekit_server_config_t config;

void setup()
{
  Serial.begin(115200);

  WiFiManager wm;

  wm.setConfigPortalTimeout(600);

  char hostString[64] = {0};
  sprintf(hostString, "HomeKit-WS2812b-%06X", ESP.getChipId());

  if (!wm.autoConnect(hostString))
  {
    Serial.println("setup() - wm.autoConnect() - failed to connect, resetting...");

    ESP.restart();
  }
  else
  {
    Serial.println("setup() - wm.autoConnect() - connected...");
  }

  EEPROM.begin(4096);

  EEPROM.get(1408, eeprom_settings);

  if (strcmp(eeprom_settings.checksum, "Andrei01"))
  {
    Serial.println("setup() - EEPROM settings checksum do not match, writing default settings...");

    strcpy(eeprom_settings.checksum, "Andrei01");

    eeprom_settings.mode = 0;
    eeprom_settings.speed = 1000;

    EEPROM.put(1408, eeprom_settings);

    EEPROM.commit();

    EEPROM.get(1408, eeprom_settings);
  }

  Serial.println("setup() - EEPROM settings read:");
  Serial.printf("eeprom_settings.checksum: %s\n", eeprom_settings.checksum);
  Serial.printf("eeprom_settings.mode: %d\n", eeprom_settings.mode);
  Serial.printf("eeprom_settings.speed: %d\n", eeprom_settings.speed);

  ws2812fx.init();

  ws2812fx.setMode(eeprom_settings.mode);
  ws2812fx.setSpeed(eeprom_settings.speed);

  ws2812fx.setColor(0, 0, 0);
  ws2812fx.setBrightness(255);

  ws2812fx.start();

  server.on("/", srv_handle_index_html);
  server.on("/set", srv_handle_set);

  server.begin();

  // homekit_storage_reset();

  my_acessory_init();

  arduino_homekit_setup(&config);
}

void loop()
{
  ws2812fx.service();

  server.handleClient();

  arduino_homekit_loop();

  delay(10);
}
