#ifndef EXTERNAL_API_H
#define EXTERNAL_API_H

#include <Arduino.h>
#include <HTTPClient.h>
#include "utils.h"

#define DAY_COUNT 5

// Certificate of nominatim.openmaps.org until Mar 12 23:59:59 2027 GMT
// Check out using command: openssl s_client -showcerts -connect jigsaw.w3.org:443
const char *rootCACertificate = R"string_literal(
-----BEGIN CERTIFICATE-----
MIIFBjCCAu6gAwIBAgIRAIp9PhPWLzDvI4a9KQdrNPgwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjQwMzEzMDAwMDAw
WhcNMjcwMzEyMjM1OTU5WjAzMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg
RW5jcnlwdDEMMAoGA1UEAxMDUjExMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIB
CgKCAQEAuoe8XBsAOcvKCs3UZxD5ATylTqVhyybKUvsVAbe5KPUoHu0nsyQYOWcJ
DAjs4DqwO3cOvfPlOVRBDE6uQdaZdN5R2+97/1i9qLcT9t4x1fJyyXJqC4N0lZxG
AGQUmfOx2SLZzaiSqhwmej/+71gFewiVgdtxD4774zEJuwm+UE1fj5F2PVqdnoPy
6cRms+EGZkNIGIBloDcYmpuEMpexsr3E+BUAnSeI++JjF5ZsmydnS8TbKF5pwnnw
SVzgJFDhxLyhBax7QG0AtMJBP6dYuC/FXJuluwme8f7rsIU5/agK70XEeOtlKsLP
Xzze41xNG/cLJyuqC0J3U095ah2H2QIDAQABo4H4MIH1MA4GA1UdDwEB/wQEAwIB
hjAdBgNVHSUEFjAUBggrBgEFBQcDAgYIKwYBBQUHAwEwEgYDVR0TAQH/BAgwBgEB
/wIBADAdBgNVHQ4EFgQUxc9GpOr0w8B6bJXELbBeki8m47kwHwYDVR0jBBgwFoAU
ebRZ5nu25eQBc4AIiMgaWPbpm24wMgYIKwYBBQUHAQEEJjAkMCIGCCsGAQUFBzAC
hhZodHRwOi8veDEuaS5sZW5jci5vcmcvMBMGA1UdIAQMMAowCAYGZ4EMAQIBMCcG
A1UdHwQgMB4wHKAaoBiGFmh0dHA6Ly94MS5jLmxlbmNyLm9yZy8wDQYJKoZIhvcN
AQELBQADggIBAE7iiV0KAxyQOND1H/lxXPjDj7I3iHpvsCUf7b632IYGjukJhM1y
v4Hz/MrPU0jtvfZpQtSlET41yBOykh0FX+ou1Nj4ScOt9ZmWnO8m2OG0JAtIIE38
01S0qcYhyOE2G/93ZCkXufBL713qzXnQv5C/viOykNpKqUgxdKlEC+Hi9i2DcaR1
e9KUwQUZRhy5j/PEdEglKg3l9dtD4tuTm7kZtB8v32oOjzHTYw+7KdzdZiw/sBtn
UfhBPORNuay4pJxmY/WrhSMdzFO2q3Gu3MUBcdo27goYKjL9CTF8j/Zz55yctUoV
aneCWs/ajUX+HypkBTA+c8LGDLnWO2NKq0YD/pnARkAnYGPfUDoHR9gVSp/qRx+Z
WghiDLZsMwhN1zjtSC0uBWiugF3vTNzYIEFfaPG7Ws3jDrAMMYebQ95JQ+HIBD/R
PBuHRTBpqKlyDnkSHDHYPiNX3adPoPAcgdF3H2/W0rmoswMWgTlLn1Wu0mrks7/q
pdWfS6PJ1jty80r2VKsM/Dj3YIDfbjXKdaFU5C+8bhfJGqU3taKauuz0wHVGT3eo
6FlWkWYtbt4pgdamlwVeZEW+LM7qZEJEsMNPrfC03APKmZsJgpWCDWOKZvkZcvjV
uYkQ4omYCTX5ohy+knMjdOmdH9c7SpqEWBDC86fiNex+O0XOMEZSa8DA
-----END CERTIFICATE-----
)string_literal";

struct WeatherDescription {
  short code;
  const char* description;
};

WeatherDescription weatherDescriptions[] = {
  { 0, "Clear" },
  { 1, "Mainly Clear" },
  { 2, "Partly Cloudy" },
  { 3, "Overcast" },
  { 45, "Fog" },
  { 48, "Fog" },
  { 51, "Light Drizzle" },
  { 53, "Drizzle" },
  { 55, "Dense Drizzle" },
  { 56, "Freezing Drizzle" },
  { 57, "Freezing Drizzle" },
  { 61, "Freezing Rain" },
  { 63, "Freezing Rain" },
  { 65, "Freezing Rain" },
  { 66, "Rain" },
  { 67, "Heavy Rain" },
  { 71, "Slight Snow" },
  { 73, "Snow" },
  { 75, "Heavy Snow" },
  { 77, "Snow Grains" },
  { 80, "Slight Rain" },
  { 81, "Rain" },
  { 82, "Heavy Rain" },
  { 85, "Snow" },
  { 86, "Heavy Snow" },
  { 95, "Storm" },
  { 96, "Storm Hail" },
  { 99, "Storm Hail" }
};

struct WeatherData {
  float temperature;
  short weather_code;
  int precipitation_probility;
  int humidity;
  float uv_index;
  String sunrise_time;
  String sunset_time;
  char degree_unit;
  float temp_max_daily[DAY_COUNT];
  float temp_min_daily[DAY_COUNT];
  short weather_code_daily[DAY_COUNT];
};

struct Geocode {
  float lat;
  float lng;
  String city;
  String country_code;
  String country;
};

struct TimeInfo {
  long unixtime;
  int day_of_week;
  int raw_offset;
};

struct IPInfo {
  String city;
  String region;
  String country;
  float lat;
  float lng;
  String timezone;
};

const char* weather_description(int code) {
  int count = sizeof(weatherDescriptions) / sizeof(weatherDescriptions[0]);
  for (int i = 0; i < count; i++) {
    if (weatherDescriptions[i].code == code) {
      return weatherDescriptions[i].description;
    }
  }

  return "Unknown";
}

void update_IP_info(IPInfo& info) {
  HTTPClient http;
  http.begin("http://ipinfo.io/json");
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    // Serial.println(payload);
    info.city = json_val(payload, "city");
    info.region = json_val(payload, "region");
    info.country = json_val(payload, "country");
    info.timezone = json_val(payload, "timezone");
    String loc = json_val(payload, "loc");
    int commaIndex = loc.indexOf(',');
    info.lat = loc.substring(0, commaIndex).toFloat();
    info.lng = loc.substring(commaIndex + 1).toFloat();
  } else {
    // Serial.printf("Error fetching ip info: %d\n", httpCode);
    info.city = "";
  }

  http.end();
}

void update_time_info(TimeInfo& info, const String& timezone) {
  HTTPClient http;
  http.begin("http://worldtimeapi.org/api/timezone/" + timezone);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    // Serial.println(payload);
    info.unixtime = json_val(payload, "unixtime").toInt();
    info.day_of_week = json_val(payload, "day_of_week").toInt();
    info.raw_offset = json_val(payload, "raw_offset").toInt();
  } else {
    // Serial.printf("Error fetching time: %d\n", httpCode);
    info.unixtime = 0;
  }

  http.end();
}

void update_geocode(Geocode& g, const String& address) {
  NetworkClientSecure* client = new NetworkClientSecure;
  if (!client)
    return;
  client->setCACert(rootCACertificate);
  String url = "https://nominatim.openstreetmap.org/search?q=" + urlEncode(address) + "&format=json&addressdetails=1";

  {
    // Add a scope for HTTPClient to make sure it is destroyed before NetworkClientSecure
    HTTPClient https;
    https.begin(*client, url);
    https.addHeader("User-Agent", "ESP32/1.0 (clin4185@usc.edu)");
    int httpCode = https.GET();
    if (httpCode == HTTP_CODE_OK) {
      String payload = https.getString();
      // Serial.println(payload);
      g.lat = json_val(payload, "lat").toFloat();
      g.lng = json_val(payload, "lon").toFloat();
      g.city = json_val(payload, "address.city");
      g.country_code = json_val(payload, "address.country_code");
      g.country_code.toUpperCase();
      g.country = json_val(payload, "address.country");
    } else {
      // Serial.printf("Error fetching geocode: %d\n", httpCode);
      g.city = "";
      g.country_code = "";
      g.country = "";
    }
  }

  delete client;
}

void update_weather(WeatherData& w, const float& lat, const float& lng, const String& timezone = "auto", const String& tempUnit = "celsius") {
  String url = "http://api.open-meteo.com/v1/forecast";
  url += "?latitude=" + String(lat);
  url += "&longitude=" + String(lng);
  url += "&current=temperature_2m,weather_code,relative_humidity_2m";
  url += "&daily=weather_code,temperature_2m_max,temperature_2m_min,sunrise,sunset,uv_index_max,precipitation_probability_max";
  url += "&timezone=" + timezone;
  url += "&temperature_unit=" + tempUnit;
  url += "&forecast_days=" + String(DAY_COUNT);

  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    // Serial.println(payload);
    String arr_buffer[DAY_COUNT];
    w.temperature = json_val(payload, "current.temperature_2m").toFloat();
    w.weather_code = json_val(payload, "current.weather_code").toInt();
    w.humidity = json_val(payload, "current.relative_humidity_2m").toInt();
    String unit_str = json_val(payload, "current_units.temperature_2m");
    w.degree_unit = unit_str[unit_str.length() - 1];
    json_array(json_val(payload, "daily.precipitation_probability_max"), arr_buffer, DAY_COUNT);
    w.precipitation_probility = arr_buffer[0].toInt();
    json_array(json_val(payload, "daily.uv_index_max"), arr_buffer, DAY_COUNT);
    w.uv_index = arr_buffer[0].toFloat();
    json_array(json_val(payload, "daily.sunrise"), arr_buffer, DAY_COUNT);
    w.sunrise_time = arr_buffer[0].substring(arr_buffer[0].length() - DAY_COUNT);
    json_array(json_val(payload, "daily.sunset"), arr_buffer, DAY_COUNT);
    w.sunset_time = arr_buffer[0].substring(arr_buffer[0].length() - DAY_COUNT);

    json_array(json_val(payload, "daily.temperature_2m_max"), arr_buffer, DAY_COUNT);
    for (int i = 0; i < DAY_COUNT; i++)
      w.temp_max_daily[i] = arr_buffer[i].toFloat();
    json_array(json_val(payload, "daily.temperature_2m_min"), arr_buffer, DAY_COUNT);
    for (int i = 0; i < DAY_COUNT; i++)
      w.temp_min_daily[i] = arr_buffer[i].toFloat();
    json_array(json_val(payload, "daily.weather_code"), arr_buffer, DAY_COUNT);
    for (int i = 0; i < DAY_COUNT; i++)
      w.weather_code_daily[i] = arr_buffer[i].toInt();
  } else {
    // Serial.printf("Error fetching weather: %d\n", httpCode);
    w.weather_code = -1;
  }

  http.end();
}

#endif  // EXTERNAL_API_H