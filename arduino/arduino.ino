#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <time.h>
#include "external_api.h"
#include "nextion.h"

WiFiMulti wifiMulti;
IPInfo ip_info;
uint8_t current_page = PAGE_MAIN;
String current_time;
String unit_of_temp = "fahrenheit";

void setup() {
  Serial.begin(115200);                     // UART0
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // UART2
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("SpectrumSetup-71", "chillysnake872");
  while (wifiMulti.run() != WL_CONNECTED) { delay(500); /* Wait for WiFi */ }

  update_IP_info(ip_info);
  TimeInfo time_info;
  update_time_info(time_info, ip_info.timezone);
  struct timeval tv;
  tv.tv_sec = time_info.unixtime + time_info.raw_offset;
  tv.tv_usec = 0;
  settimeofday(&tv, nullptr);
  nextion_send("page pageMain\xFF\xFF\xFF");
  nextion_send("sendme\xFF\xFF\xFF");
}

void loop() {
  Command cmds[5];
  int cmdCount = 0;
  nextion_recv(cmds, 5, cmdCount);
  for (int i = 0; i < cmdCount; i++) {
    processCommand(cmds[i]);
  }

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char time_buffer[30];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    String new_time = String(time_buffer);
    if (new_time != current_time) {
      current_time = new_time;
      nextion_send("tDatetime.txt=\"" + current_time + "\"\xFF\xFF\xFF", false);
    }
  }

  delay(30);
}

void processCommand(const Command& cmd) {
  if (cmd.event == CURRENT_PAGE_NUMBER) {
    current_page = cmd.page;
    if (cmd.page == PAGE_MAIN) {
      show_main();
    }

    return;
  }

  if (cmd.event == CURRENT_PAGE_NUMBER) {
    switch (cmd.page) {
      case PAGE_MAIN:
        break;
      case PAGE_MENU:
        break;
    }
  }
}

void show_main() {
  if (ip_info.city != "") {
    nextion_send("tAddress.txt=\"" + ip_info.city + ", " + ip_info.region + ", " + ip_info.country + "\"\xFF\xFF\xFF");
    render_weather(ip_info.lat, ip_info.lng, ip_info.timezone);
  }
}

void render_weather(const int lat, const int lng, const String& timezone) {
  WeatherData w;
  update_weather(w, lat, lng, timezone, unit_of_temp);
  if (w.weather_code == -1) {
    nextion_send("tAddress.txt=\"Error fetching weather\"\xFF\xFF\xFF");
    return;
  }

  // Current weather
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "pWeather.pic=%d\xFF\xFF\xFF", nextion_weather_pic(w.weather_code));
  nextion_send(buffer);
  snprintf(buffer, sizeof(buffer), "tWeather.txt=\"%s\"\xFF\xFF\xFF", weather_description(w.weather_code));
  nextion_send(buffer);
  snprintf(buffer, sizeof(buffer), "tTemperature.txt=\"%.1f\xB0%c\"\xFF\xFF\xFF", w.temperature, w.degree_unit);
  nextion_send(buffer);
  snprintf(buffer, sizeof(buffer), "tPrecipitation.txt=\"%d%%\"\xFF\xFF\xFF", w.precipitation_probility);
  nextion_send(buffer);
  snprintf(buffer, sizeof(buffer), "tHumidity.txt=\"%d%%\"\xFF\xFF\xFF", w.humidity);
  nextion_send(buffer);
  snprintf(buffer, sizeof(buffer), "tUVIndex.txt=\"%.1f\"\xFF\xFF\xFF", w.uv_index);
  nextion_send(buffer);
  snprintf(buffer, sizeof(buffer), "tSunrise.txt=\"%s\"\xFF\xFF\xFF", w.sunrise_time.c_str());
  nextion_send(buffer);
  snprintf(buffer, sizeof(buffer), "tSunset.txt=\"%s\"\xFF\xFF\xFF", w.sunset_time.c_str());
  nextion_send(buffer);

  // 5-day bar chart
  const int chart_left = 50, chart_top = 440;
  const int chart_width = 380, chart_height = 180;
  const int chart_bottom = chart_top + chart_height + 40;
  const int bar_width = 20;
  const int bar_space = chart_width / DAY_COUNT;
  const int bar_margin = (bar_space - bar_width) / 2;

  float max_temp = w.temp_max_daily[0], min_temp = w.temp_min_daily[0];
  for (int i = 1; i < DAY_COUNT; i++) {
    if (w.temp_max_daily[i] > max_temp)
      max_temp = w.temp_max_daily[i];
    if (w.temp_min_daily[i] < min_temp)
      min_temp = w.temp_min_daily[i];
  }

  float temp_range = max_temp - min_temp;
  float scale = (temp_range > 0) ? (float(chart_height) / temp_range) : 1;
  for (int i = 0; i < DAY_COUNT; i++) {
    int left = chart_left + i * bar_space;
    int top = chart_top + chart_height - int((w.temp_max_daily[i] - min_temp) * scale);
    int height = int((w.temp_max_daily[i] - w.temp_min_daily[i]) * scale);
    snprintf(buffer, sizeof(buffer), "fill %d,%d,%d,%d,%d\xFF\xFF\xFF", left + bar_margin, top, bar_width, height, 65120);
    nextion_send(buffer);
    snprintf(buffer, sizeof(buffer), "xstr %d,%d,%d,30,3,WHITE,0,1,1,0,\" %d\xB0\"\xFF\xFF\xFF", left, top - 30, bar_space, int(w.temp_max_daily[i]));
    nextion_send(buffer);
    snprintf(buffer, sizeof(buffer), "xstr %d,%d,%d,30,3,WHITE,0,1,1,0,\" %d\xB0\"\xFF\xFF\xFF", left, top + height, bar_space, int(w.temp_min_daily[i]));
    nextion_send(buffer);
  }
  snprintf(buffer, sizeof(buffer), "line %d,%d,%d,%d,%d\xFF\xFF\xFF", chart_left, chart_bottom, chart_left + chart_width, chart_bottom, 31727);
  nextion_send(buffer);

  // 5-day weather picture
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  const int pic_width = 50, pic_left = chart_left, pic_top = chart_bottom + 20;
  int pic_margin = (bar_space - pic_width) / 2;
  for (int i = 0; i < DAY_COUNT; i++) {
    int left = pic_left + i * bar_space;
    int pic_id_small = nextion_weather_pic(w.weather_code_daily[i], true);
    snprintf(buffer, sizeof(buffer), "pic %d,%d,%d\xFF\xFF\xFF", left + pic_margin, pic_top, pic_id_small);
    nextion_send(buffer);

    char date_buffer[8];
    strftime(date_buffer, sizeof(date_buffer), "%a %d", &timeinfo);
    snprintf(buffer, sizeof(buffer), "xstr %d,%d,%d,30,3,WHITE,0,1,1,0,\"%s\"\xFF\xFF\xFF", left, pic_top + pic_width + 10, bar_space, date_buffer);
    nextion_send(buffer);
    timeinfo.tm_mday += 1;
    mktime(&timeinfo);
  }
}
