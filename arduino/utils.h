#ifndef UTILS_H
#define UTILS_H

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
    // Serial.println(outputArray[count]);
    startIndex = endIndex + 1;
    endIndex = trimmedString.indexOf(',', startIndex);
    count++;
  }
}

String urlEncode(const char* msg) {
  const char* hex = "0123456789ABCDEF";
  String encodedMsg = "";

  while (*msg != '\0') {
    if (
      ('a' <= *msg && *msg <= 'z') || ('A' <= *msg && *msg <= 'Z') || ('0' <= *msg && *msg <= '9') || *msg == '-' || *msg == '_' || *msg == '.' || *msg == '~') {
      encodedMsg += *msg;
    } else {
      encodedMsg += '%';
      encodedMsg += hex[(unsigned char)*msg >> 4];
      encodedMsg += hex[*msg & 0xf];
    }
    msg++;
  }
  return encodedMsg;
}

String urlEncode(String msg) {
  return urlEncode(msg.c_str());
}

#endif  // UTILS_H