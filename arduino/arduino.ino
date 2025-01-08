#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <time.h>
#include "external_api.h"
#include "nextion.h"

#define SCAN_THRESHOLD_SEC 3
#define SSID_LIST_LENGTH 5

WiFiMulti wifiMulti;
IPInfo ip_info;
Geocode geocode;
String current_display_time;

// menu states
String unit_of_temp = "fahrenheit";
uint8_t current_page = PAGE_MAIN;
uint8_t current_ssid_page = 0;
int8_t selected_ssid_row = -1;
uint8_t last_scan_count = 0;
time_t last_scan_time = 0;

// Flag for fetching string data after user push buttons
bool is_password = false;
bool is_location = false;

void setup() {
  Serial.begin(115200);                     // UART0 for debug
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // UART2 for Nextion display
  while (!Serial || !Serial2) { delay(200); }
  WiFi.mode(WIFI_STA);

  // Optionally set initial WiFi.
  // wifiMulti.addAP("SSID", "PASSWORD");
  // while (wifiMulti.run() != WL_CONNECTED) { delay(500); }
  nextion_send("sendme\xFF\xFF\xFF");
}

void loop() {
  Command cmds[5];
  int cmdCount = 0;
  nextion_recv(cmds, 5, cmdCount);
  for (int i = 0; i < cmdCount; i++) {
    processCommand(cmds[i]);
  }

  if (current_page == PAGE_MAIN) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char time_buffer[30];
      strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
      String new_display_time = String(time_buffer);
      if (new_display_time != current_display_time) {
        current_display_time = new_display_time;
        nextion_send("tDatetime.txt=\"" + current_display_time + "\"\xFF\xFF\xFF", false);
      }
    } else if (WiFi.status() == WL_CONNECTED) {
      update_IP_info(ip_info);
      TimeInfo time_info;
      update_time_info(time_info, ip_info.timezone);
      struct timeval tv;
      tv.tv_sec = time_info.unixtime + time_info.raw_offset;
      tv.tv_usec = 0;
      settimeofday(&tv, nullptr);
    }
  }

  delay(30);
}

void processCommand(const Command& cmd) {
  if (cmd.event == CURRENT_PAGE_NUMBER) {
    current_page = cmd.page;
    if (cmd.page == PAGE_MAIN) {
      show_main();
    } else if (cmd.page == PAGE_MENU) {
      show_menu();
    }

    return;
  }

  if (cmd.page == PAGE_MAIN) {
    switch (cmd.component) {
      case B_REFRESH:
        nextion_send("page pageMain\xFF\xFF\xFF");  // Erase line chart and reset main page
        show_main();
        break;
      case B_MENU:
        show_menu();
        break;
    }
  } else if (cmd.page == PAGE_MENU) {
    switch (cmd.component) {
      case T_ID1:
      case T_SSID1:
        select_row(0);
        break;
      case T_ID2:
      case T_SSID2:
        select_row(1);
        break;
      case T_ID3:
      case T_SSID3:
        select_row(2);
        break;
      case T_ID4:
      case T_SSID4:
        select_row(3);
        break;
      case T_ID5:
      case T_SSID5:
        select_row(4);
        break;
      case B_LEFT:
        show_prev_ssid_page();
        break;
      case B_RIGHT:
        show_next_ssid_page();
        break;
      case B_CONNECT:
        nextion_send("get tPassword.txt\xFF\xFF\xFF");
        is_password = true;
        break;
      case B_UPDATE_LOCATION:
        nextion_send("get tLocation.txt\xFF\xFF\xFF");
        is_location = true;
        break;
      case B_UNIT_TEMP:
        if (unit_of_temp == "fahrenheit") {
          unit_of_temp = "celsius";
        } else {
          unit_of_temp = "fahrenheit";
        }
        nextion_send("bUnitTemp.txt=\"Unit of Temperature: " + unit_of_temp + "\"\xFF\xFF\xFF");
        break;
      case B_BACK:
        show_main();
        break;
    }
  } else if (cmd.page == 0xff) {
    if (!cmd.string_data.isEmpty())
      handle_string_data(cmd.string_data);
  }
}

void show_menu() {
  time_t now = time(nullptr);
  if (now == -1 || now - last_scan_time >= SCAN_THRESHOLD_SEC) {
    last_scan_time = now;
    WiFi.scanDelete();
    last_scan_count = WiFi.scanNetworks();
  }

  render_ssids(current_ssid_page * SSID_LIST_LENGTH);
}

void show_prev_ssid_page() {
  if (current_ssid_page == 0)
    return;
  current_ssid_page -= 1;
  selected_ssid_row = -1;
  render_ssids(current_ssid_page * SSID_LIST_LENGTH);
}

void show_next_ssid_page() {
  if ((current_ssid_page + 1) * SSID_LIST_LENGTH >= last_scan_count)
    return;
  current_ssid_page += 1;
  selected_ssid_row = -1;
  render_ssids(current_ssid_page * SSID_LIST_LENGTH);
}

void select_row(int row) {
  char buffer[24];
  if (selected_ssid_row != -1) {
    snprintf(buffer, sizeof(buffer), "tSSID%d.bco=65535\xFF\xFF\xFF", selected_ssid_row + 1);
    nextion_send(buffer);
  }

  snprintf(buffer, sizeof(buffer), "tSSID%d.bco=65504\xFF\xFF\xFF", row + 1);
  nextion_send(buffer);
  selected_ssid_row = row;
}

void render_ssids(int first_index) {
  char buffer[96];
  String active_ssid = WiFi.SSID();
  for (int i = 0; i < SSID_LIST_LENGTH; i++) {
    snprintf(buffer, sizeof(buffer), "tId%d.txt=\"%02d\"\xFF\xFF\xFF", i + 1, first_index + i + 1);
    nextion_send(buffer);
    if (first_index + i < last_scan_count) {
      snprintf(buffer, sizeof(buffer), "tSSID%d.txt=\"%s\"\xFF\xFF\xFF", i + 1, WiFi.SSID(first_index + i).c_str());
      nextion_send(buffer);
      int back_color = !active_ssid.isEmpty() && active_ssid == WiFi.SSID(first_index + i) ? 2032 : 65535;
      snprintf(buffer, sizeof(buffer), "tSSID%d.bco=%d\xFF\xFF\xFF", i + 1, back_color);
      nextion_send(buffer);
    } else {
      snprintf(buffer, sizeof(buffer), "tSSID%d.txt=\"--\"\xFF\xFF\xFF", i + 1);
      nextion_send(buffer);
    }
  }
}

void show_main() {
  current_ssid_page = 0;
  selected_ssid_row = -1;

  if (!geocode.city.isEmpty()) {
    nextion_send(String("tAddress.txt=\"") + geocode.city + ", " + geocode.country_code + "\"\xFF\xFF\xFF");
    render_weather(geocode.lat, geocode.lng, ip_info.timezone);
    return;
  }

  if (!ip_info.city.isEmpty()) {
    nextion_send(String("tAddress.txt=\"") + ip_info.city + ", " + ip_info.region + ", " + ip_info.country + "\"\xFF\xFF\xFF");
    render_weather(ip_info.lat, ip_info.lng, ip_info.timezone);
    return;
  }

  nextion_send("tAddress.txt=\"NO WI-FI\"\xFF\xFF\xFF");
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

void handle_string_data(String data) {
  if (is_password) {
    is_password = false;
    if (selected_ssid_row == -1) {
      // Serial.println("Failed to connect. No SSID is selected.");
      return;
    }
    connect_to_WiFi(WiFi.SSID(current_ssid_page * 5 + selected_ssid_row).c_str(), data.c_str());
    show_menu();
    update_IP_info(ip_info);
  } else if (is_location) {
    is_location = false;
    update_geocode(geocode, data);
  }
}

void connect_to_WiFi(const char* ssid, const char* password) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.disconnect();
    while (WiFi.status() == WL_CONNECTED) { delay(500); }
  }

  WiFi.begin(ssid, password);
  // Serial.printf("Connecting to WiFi: %s, Password: %s\n", ssid, password);
  for (int i = 0; i < 10; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      wifiMulti.addAP(ssid, password);
      return;
    }
    delay(500);
  }

  // WiFi.scanNetworks doesn't work after failing to connect a WiFi.
  // So turn off WiFi here as a workaround.
  // See https://github.com/espressif/arduino-esp32/issues/3294
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  delay(500);
}
