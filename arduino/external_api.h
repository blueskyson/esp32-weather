#ifndef EXTERNAL_API_H
#define EXTERNAL_API_H

#include <Arduino.h>
#include <HTTPClient.h>

#define DAY_COUNT 5

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
  String display_name;
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

  return "Unknown Weather Code";
}

String json_val(const String& payload, const String& query) {
  String remainingQuery = query;
  String currentPayload = payload;

  while (remainingQuery.length() > 0) {
    int dotIndex = remainingQuery.indexOf('.');
    String key = (dotIndex == -1) ? remainingQuery : remainingQuery.substring(0, dotIndex);
    remainingQuery = (dotIndex == -1) ? "" : remainingQuery.substring(dotIndex + 1);

    int keyIndex = currentPayload.indexOf("\"" + key + "\":");
    if (keyIndex == -1) return "";  // Key not found

    int valueStart = currentPayload.indexOf(':', keyIndex) + 1;
    while (currentPayload[valueStart] == ' ') {
      valueStart++;
    }
    int valueEnd;

    // Check if the value is a string
    if (currentPayload[valueStart] == '"') {
      valueStart++;
      valueEnd = currentPayload.indexOf('"', valueStart);
    } else {
      // Handle numbers, arrays, or objects
      if (currentPayload[valueStart] == '[' || currentPayload[valueStart] == '{') {
        char closingChar = (currentPayload[valueStart] == '[') ? ']' : '}';
        valueEnd = currentPayload.indexOf(closingChar, valueStart) + 1;
      } else {
        valueEnd = currentPayload.indexOf(',', valueStart);
        if (valueEnd == -1) valueEnd = currentPayload.indexOf('}', valueStart);  // End of JSON
      }
    }

    String extractedValue = currentPayload.substring(valueStart, valueEnd);
    extractedValue.trim();

    // If there are more nested keys, update the current payload to extracted object/array
    if (remainingQuery.length() > 0 && (extractedValue.startsWith("{") || extractedValue.startsWith("["))) {
      currentPayload = extractedValue;
    } else {
      // Serial.println(extractedValue);
      return extractedValue;  // Return final value
    }
  }

  return "";
}

void json_array(const String& jsonArrayString, String* outputArray, size_t maxLength) {
  // Remove brackets from the JSON array string
  String trimmedString = jsonArrayString;
  trimmedString.replace("[", "");
  trimmedString.replace("]", ",");

  // Split the string by commas
  int startIndex = 0;
  int endIndex = trimmedString.indexOf(',');
  int offset = trimmedString[startIndex] == '"' ? 1 : 0;
  int count = 0;

  while (endIndex != -1 && count < maxLength) {
    outputArray[count] = trimmedString.substring(startIndex + offset, endIndex - offset);
    Serial.println(outputArray[count]);
    startIndex = endIndex + 1;
    endIndex = trimmedString.indexOf(',', startIndex);
    count++;
  }
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
    // Serial.printf("Error fetching weather: %d\n", httpCode);
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
    // Serial.printf("Error fetching weather: %d\n", httpCode);
    info.unixtime = 0;
  }

  http.end();
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