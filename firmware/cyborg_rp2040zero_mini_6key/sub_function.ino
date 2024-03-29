void status_output() {
  Serial.print("lyr:");
  Serial.println(layers);
  Serial.print("kys:");
  Serial.println(key);
}

///簡易的に英字配列を日本語配列に変換
uint16_t en2jp(char ch, uint8_t *mod) {
  //小文字
  if (ch >= 0x61 && ch <= 0x7a)
    return ch - 0x5d;
  //数字
  if (ch >= 0x30 && ch <= 0x39)
    return ch - 0x12;
  //大文字
  if (ch >= 0x41 && ch <= 0x5a) {
    *mod = KEYBOARD_MODIFIER_LEFTSHIFT;
    return ch - 0x3d;
  }
  if (ch == '/')
    return HID_KEY_SLASH;  //0x38
  if (ch == ':')
    return HID_KEY_APOSTROPHE;  //0x34
  if (ch == ';')
    return HID_KEY_SEMICOLON;  //0x33
  if (ch == '.')
    return HID_KEY_PERIOD;  //0x07
  if (ch == ',')
    return HID_KEY_COMMA;  //0x36
  if (ch == '@')
    return HID_KEY_BRACKET_LEFT;  //0x2f
  if (ch == '[')
    return HID_KEY_BRACKET_RIGHT;  //0x30
  if (ch == ']')
    return HID_KEY_BACKSLASH;  //0x31
  if (ch == '^')
    return HID_KEY_EQUAL;  //0x2e
  if (ch == '-')
    return HID_KEY_MINUS;  //0x2d
  if (ch == ' ')
    return HID_KEY_SPACE;  //0x2c
  if (ch == '\\')
    return HID_KEY_KANJI3;  //0x89

  *mod = KEYBOARD_MODIFIER_LEFTSHIFT;
  if (ch == '!')
    return 0x1E;  //
  if (ch == '"')
    return 0x1F;  //
  if (ch == '#')
    return 0x20;  //
  if (ch == '$')
    return 0x21;  //
  if (ch == '%')
    return 0x22;  //
  if (ch == '&')
    return 0x23;  //
  if (ch == '\'')
    return 0x24;  //
  if (ch == '(')
    return 0x25;  //
  if (ch == ')')
    return 0x26;  //
  if (ch == '=')
    return 0x2D;  //
  if (ch == '~')
    return 0x2E;  //
  if (ch == '|')
    return 0x89;  //
  if (ch == '`')
    return 0x2F;  //
  if (ch == '{')
    return 0x30;  //
  if (ch == '+')
    return 0x33;  //
  if (ch == '*')
    return 0x34;  //
  if (ch == '}')
    return 0x32;  //
  if (ch == '<')
    return 0x36;  //
  if (ch == '>')
    return 0x37;  //
  if (ch == '?')
    return 0x38;  //
  if (ch == '_')
    return 0x87;  //

  *mod = 0;
  return 0x0000;
}

//シリアルとUSBの接続をチェックする
void check_mount() {

  if (TinyUSBDevice.suspended() == true && suspend_state == false) {
    suspend_state = true;
  }

  if (TinyUSBDevice.suspended() == false && suspend_state == true) {
    suspend_state = false;
    for (int i = 0; i < (int)strip.numPixels(); i++)
      strip.setPixelColor(i, Brightness * rgb_mask[0], Brightness * rgb_mask[1], Brightness * rgb_mask[2]);
    strip.show();
  }

  if (suspend_state) {
    delay(100);
    for (int i = 0; i < (int)strip.numPixels(); i++)
      strip.setPixelColor(i, 0, 0, 0);
    strip.show();
  }

  if (Serial.dtr() == 1 && serial_state == false) {
    serial_state = true;
    delay(100);
    status_output();
  }
  if (Serial.dtr() == 0 && serial_state == true) {
    serial_state = false;
  }

  if (!TinyUSBDevice.mounted()) {
    mount_state = false;
    delay(100);
  } else if (mount_state == false) {
    mount_state = true;
  }
}

void layerState_led(int s) {
  switch (s) {
    case 0:
      rgb_mask[0] = 1;
      rgb_mask[1] = 1;
      rgb_mask[2] = 1;
      break;
    case 1:
      rgb_mask[0] = 1;
      rgb_mask[1] = 0;
      rgb_mask[2] = 0;
      break;
    case 2:
      rgb_mask[0] = 1;
      rgb_mask[1] = 1;
      rgb_mask[2] = 0;
      break;
    case 3:
      rgb_mask[0] = 0;
      rgb_mask[1] = 1;
      rgb_mask[2] = 0;
      break;
    case 4:
      rgb_mask[0] = 0;
      rgb_mask[1] = 0;
      rgb_mask[2] = 1;
      break;
    case 5:
      rgb_mask[0] = 1;
      rgb_mask[1] = 0;
      rgb_mask[2] = 1;
      break;
  }
  for (int i = 0; i < (int)strip.numPixels(); i++)
    strip.setPixelColor(i, Brightness * rgb_mask[0], Brightness * rgb_mask[1], Brightness * rgb_mask[2]);
  strip.show();
}

void off_LEDs() {
  for (int i = 0; i < (int)strip.numPixels(); i++)
    strip.setPixelColor(i, 0, 0, 0);
  strip.show();
}

void pickOneKey(int num, int index) {
  keys[0] = layer_keys[num][index][0];
  keys[1] = layer_keys[num][index][1];
  keys[2] = layer_keys[num][index][2];
  keys[3] = layer_keys[num][index][3];
  keys[4] = layer_keys[num][index][4];
  keys[5] = layer_keys[num][index][5];
}

//6文字のHEX文字列をIntに
uint16_t hexToInt(String str) {
  const char hex[6] = { str[0], str[1], str[2], str[3], str[4], str[5] };
  return strtol(hex, NULL, 16);
}


int string_cut(String _str, char delimiter) {
  for (int i = 0; i < str.length(); i++)
    if (_str.charAt(i) == delimiter)
      return i;
  return 0;
}