#ifndef NEXTION_H
#define NEXTION_H

String nextion_buffer = "";

// Picture ID
static const uint8_t PICTURE_BACKGROUND = 0;

// Event Codes
static const uint8_t EVENT_TOUCH = 0x65;
static const uint8_t CURRENT_PAGE_NUMBER = 0x66;
static const uint8_t STRING_DATA = 0x70;

// Main Page Components
static const uint8_t PAGE_MAIN = 0x00;
static const uint8_t B_MENU = 0x02;
static const uint8_t B_REFRESH = 0x04;

// Menu Page Components
static const uint8_t PAGE_MENU = 0x01;
static const uint8_t T_ID1 = 0x03;
static const uint8_t T_ID2 = 0x05;
static const uint8_t T_ID3 = 0x07;
static const uint8_t T_ID4 = 0x09;
static const uint8_t T_ID5 = 0x0b;
static const uint8_t T_SSID1 = 0x02;
static const uint8_t T_SSID2 = 0x04;
static const uint8_t T_SSID3 = 0x06;
static const uint8_t T_SSID4 = 0x08;
static const uint8_t T_SSID5 = 0x0a;
static const uint8_t B_LEFT = 0x0c;
static const uint8_t B_RIGHT = 0x0d;
static const uint8_t B_CONNECT = 0x0f;
static const uint8_t B_UPDATE_LOCATION = 0x13;
static const uint8_t B_UNIT_TEMP = 0x11;
static const uint8_t B_BACK = 0x01;

struct Command {
  int event = -1;
  int page = -1;
  int component = -1;
  String string_data = "";

  String toString() {
    return "Command(event=\\x" + String(event, HEX) + ", page=\\x" + String(page, HEX) + ", component=\\x" + String(component, HEX) + ")";
  }
};

struct WeatherPic {
  short code;
  short pic_id;
  short pic_id_small;
};

// Map Open Meteo weather code to Nextion picture ID
WeatherPic weatherPictures[] = {
  { 0, 1, 10 },
  { 1, 1, 10 },
  { 2, 2, 11 },
  { 3, 3, 12 },
  { 45, 9, 18 },
  { 48, 9, 18 },
  { 51, 6, 15 },
  { 53, 6, 15 },
  { 55, 6, 15 },
  { 56, 8, 17 },
  { 57, 8, 17 },
  { 61, 5, 14 },
  { 63, 5, 14 },
  { 65, 5, 14 },
  { 66, 5, 14 },
  { 67, 5, 14 },
  { 71, 8, 17 },
  { 73, 8, 17 },
  { 75, 8, 17 },
  { 77, 8, 17 },
  { 80, 5, 14 },
  { 81, 5, 14 },
  { 82, 5, 14 },
  { 85, 8, 17 },
  { 86, 8, 17 },
  { 95, 7, 16 },
  { 96, 7, 16 },
  { 99, 7, 16 }
};

void parse(Command& cmd, const String& buffer) {
  int len = buffer.length() - 3;  // Exclude "\xFF\xFF\xFF"
  cmd.event = (int)buffer[0];
  switch (cmd.event) {
    case EVENT_TOUCH:
      cmd.page = (int)buffer[1];
      cmd.component = (int)buffer[2];
      Serial.println(cmd.toString());
      break;
    case CURRENT_PAGE_NUMBER:
      cmd.page = (int)buffer[1];
      Serial.println(cmd.toString());
      break;
    case STRING_DATA:
      cmd.string_data = buffer.substring(1, len);
      Serial.println(cmd.string_data);
      break;
  }
}

short nextion_weather_pic(short code, bool useSmall = false) {
  int count = sizeof(weatherPictures) / sizeof(weatherPictures[0]);
  for (int i = 0; i < count; i++) {
    if (weatherPictures[i].code == code) {
      return useSmall ? weatherPictures[i].pic_id_small : weatherPictures[i].pic_id;
    }
  }

  return useSmall ? weatherPictures[0].pic_id_small : weatherPictures[0].pic_id;
}

void nextion_recv(Command* cmds, int maxLength, int& cmdCount) {
  cmdCount = 0;
  int ffCount = 0;
  while (Serial2.available()) {
    byte incomingByte = Serial2.read();
    nextion_buffer += char(incomingByte);
    if (incomingByte == 0xFF && ++ffCount >= 3 && nextion_buffer.endsWith("\xFF\xFF\xFF")) {
      parse(cmds[cmdCount], nextion_buffer);
      nextion_buffer = "";
      ffCount = 0;
      if (++cmdCount >= maxLength) {
        break;
      }
    }
  }
}

// Send a command to the Nextion display
void nextion_send(const String& instruction, bool shouldLog = true) {
  Serial2.print(instruction);
  Serial2.flush();
  if (shouldLog) {
    Serial.println(instruction);
  }
}

#endif  // NEXTION_H