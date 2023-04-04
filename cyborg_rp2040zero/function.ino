void read_keys() {
  old_layer = layers;
  layerChangeFlag = false;
  sendURI = false;
  while (gpio_get(PB1) == 0) {
    count = count + 10000;

    gpio_put(LED_BUILTIN, 1);
    delay(10);
    rot1 = 0;
    rot1 = ENC_COUNT1(rot1, enc_count1);
    if (rot1 > 0) {
      layers = layers + 1;
      layerChangeFlag = true;
      count = 0;
    }
    if (rot1 < 0) {
      layers = layers - 1;
      layerChangeFlag = true;
      count = 0;
    }

    if (gpio_get(KeySW1) == 0) {
      Brightness += 16;
      if (Brightness > 255)
        Brightness = 255;
      count = 0;
    }
    if (gpio_get(KeySW2) == 0) {
      Brightness -= 16;
      if (Brightness < 0)
        Brightness = 0;
      count = 0;
    }

    if (layerChangeFlag == false)
      layers = old_layer + 1;

    if (layers > 5)
      layers = 0;
    if (layers < 0)
      layers = 5;

    if (count > 500000 && sendURI == false) {
      sendURI = true;
      outputsKeys(setup_url);
    }

    if (count > 1000000) {
      strip.setPixelColor(0, 0, 0, 0);
      strip.show();
      Serial.println("-----EEPROM RESET-----");
      for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(i, 255);
      }
      EEPROM.commit();
      delay(1000);
      layers = 0;
    }

    layerState_led(layers);
    watch = millis();
    gpio_put(LED_BUILTIN, 0);
    delay(90);
  }

  key = 0b00000000;

  if (!digitalReadFast(KeySW1))
    key |= 0b00000001;
  if (!digitalReadFast(KeySW2))
    key |= 0b00000010;
  if (!digitalReadFast(KeySW3))
    key |= 0b00000100;
  if (!digitalReadFast(KeySW4))
    key |= 0b00001000;
  if (!digitalReadFast(KeySW5))
    key |= 0b00010000;
  if (!digitalReadFast(KeySW6))
    key |= 0b00100000;

  if (key != oldKey) {

    if ((key & 0b00000001) == 0b00000001) {
      keys[0] = pickOneKey(layers, 0);
      sendKeys(RID_KEYBOARD1, keys, layer_keys[layers][0][6]);
    } else if ((oldKey & 0b00000001) != 0b00000000) {
      usb_hid.keyboardRelease(RID_KEYBOARD1);
      delay(2);  //delayを入れないと解放されない場合がある
    }

    if ((key & 0b00000010) == 0b00000010) {
      keys[0] = pickOneKey(layers, 1);
      sendKeys(RID_KEYBOARD2, keys, layer_keys[layers][1][6]);
    } else if ((oldKey & 0b00000010) != 0b00000000) {
      usb_hid.keyboardRelease(RID_KEYBOARD2);
      delay(2);
    }

    if ((key & 0b00000100) == 0b00000100) {
      keys[0] = pickOneKey(layers, 2);
      sendKeys(RID_KEYBOARD3, keys, layer_keys[layers][2][6]);
    } else if ((oldKey & 0b00000100) != 0b00000000) {
      usb_hid.keyboardRelease(RID_KEYBOARD3);
      delay(2);
    }

    if ((key & 0b00001000) == 0b00001000) {
      keys[0] = pickOneKey(layers, 3);
      sendKeys(RID_KEYBOARD4, keys, layer_keys[layers][3][6]);
    } else if ((oldKey & 0b00001000) != 0b00000000) {
      usb_hid.keyboardRelease(RID_KEYBOARD4);
      delay(2);
    }

    if ((key & 0b00010000) == 0b00010000) {
      keys[0] = pickOneKey(layers, 4);
      sendKeys(RID_KEYBOARD5, keys, layer_keys[layers][4][6]);
    } else if ((oldKey & 0b00010000) != 0b00000000) {
      usb_hid.keyboardRelease(RID_KEYBOARD5);
      delay(2);
    }

    if ((key & 0b00100000) == 0b00100000) {
      keys[0] = pickOneKey(layers, 5);
      sendKeys(RID_KEYBOARD6, keys, layer_keys[layers][5][6]);
    } else if ((oldKey & 0b00100000) != 0b00000000) {
      usb_hid.keyboardRelease(RID_KEYBOARD6);
      delay(2);
    }
    Serial.print("lyr:");
    Serial.println(layers);
    Serial.print("kys:");
    Serial.println(key);
    oldKey = key;
  }
}

void outputsKeys(String str) {
  for (int i = 0; i < str.length(); i++) {
    uint8_t mod = 0x00;
    ///簡易的にJISに変換
    //小文字
    if (str[i] >= 0x61 && str[i] <= 0x7a)
      keys[0] = str[i] - 0x5d;
    //数字
    if (str[i] >= 0x30 && str[i] <= 0x39)
      keys[0] = str[i] - 0x12;
    //大文字
    if (str[i] >= 0x41 && str[i] <= 0x5a) {
      keys[0] = str[i] - 0x3d;
      mod = KEYBOARD_MODIFIER_LEFTSHIFT;
    }
    if (str[i] == '/')
      keys[0] = HID_KEY_SLASH;
    if (str[i] == ':')
      keys[0] = HID_KEY_APOSTROPHE;
    if (str[i] == '.')
      keys[0] = HID_KEY_PERIOD;
    if (str[i] == '@')
      keys[0] = HID_KEY_BRACKET_LEFT;

    usb_hid.keyboardReport(RID_KEYBOARD1, mod, keys);
    delay(2);
    usb_hid.keyboardRelease(RID_KEYBOARD1);
    delay(2);
  }

  keys[0] = HID_KEY_ENTER;
  usb_hid.keyboardReport(RID_KEYBOARD1, 0x00, keys);
  delay(2);
  usb_hid.keyboardRelease(RID_KEYBOARD1);
  delay(2);
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
  strip.setPixelColor(0, Brightness * rgb_mask[0], Brightness * rgb_mask[1], Brightness * rgb_mask[2]);
  strip.show();
}

void Switch_function(int input) {

  if (Serial.available()) {
    str = Serial.readStringUntil('\n');

  } else if (str != "") {
    if (str.substring(0, string_cut(str, '_')) == "M") {

      str = str.substring(1 + string_cut(str, '_'));
      layer_num = str.substring(0, string_cut(str, '_')).toInt();

      str = str.substring(1 + string_cut(str, '_'));
      layer_key_num = str.substring(0, string_cut(str, '_')).toInt();

      str = str.substring(1 + string_cut(str, '_'));

      Serial.println(str.length());
      if (str.length() == 37) {
        String value = "";
        for (int i = 0; i < 7; i++) {
          if (str[0] == '[')
            str = str.substring(1);
          value = str.substring(0, string_cut(str, ','));
          if (value == "")
            value = str.substring(0, string_cut(str, ']'));
          temp_keys[i] = hexToInt(value);
          str = str.substring(1 + string_cut(str, ','));
        }

        Serial.print(layer_num);
        Serial.print("_");
        Serial.print(layer_key_num);
        Serial.print("|");

        int addres = 0;
        for (int i = 0; i < 7; i++) {
          addres = layer_num * 100 + layer_key_num * 10 + i;
          Serial.print(addres);
          Serial.print("|");
          Serial.println(temp_keys[i]);
          EEPROM.write(addres, temp_keys[i]);
        }

        EEPROM.commit();  //永続化

        //セット
        for (int i = 0; i < 7; i++) {
          addres = layer_num * 100 + layer_key_num * 10 + i;
          layer_keys[layer_num][layer_key_num][i] = EEPROM.read(addres);
        }

        Serial.println("import success");
      } else {
        Serial.println("invalid format");
      }
    }

    if (str.substring(0, string_cut(str, '_')) == "B") {
      str = str.substring(1 + string_cut(str, '_'));
      uint8_t brightness = str.toInt();
      EEPROM.write(BRIGHTNESS_ADDRESS, brightness);
      EEPROM.commit();
      Brightness = brightness;
      layerState_led(layers);
    }

    //メモリダンプ
    if (str.substring(0, string_cut(str, '_')) == "D") {
      for (int i = 0; i < 100; i++) {
        Serial.print(i);
        Serial.print(" | ");
        for (int j = 0; j < 10; j++) {
          Serial.print(EEPROM.read(i * 10 + j));
          Serial.print(" ");
        }
        Serial.println("");
      }
    }
    str = "";
  }
}

void sendKeys(uint8_t report_id, uint8_t keycode[6], uint8_t modifier) {

  TinyUSBDevice.remoteWakeup();
  if (!usb_hid.ready()) return;

  if (keycode[0] > 0xcd) {
    usb_hid.sendReport16(RID_CONSUMER_CONTROL, keycode[0]);
    delay(2);
    usb_hid.sendReport16(RID_CONSUMER_CONTROL, 0);
  } else {
    usb_hid.keyboardReport(report_id, modifier, keycode);
    delay(2);
  }
}

uint8_t pickOneKey(int num, int index) {
  return layer_keys[num][index][0];
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

//エンコーダに変化があればキー入力を実行
void RotEncFunc() {
  rot1 = 0;
  rot2 = 0;
  rot1 = ENC_COUNT1(rot1, enc_count1);
  rot2 = ENC_COUNT2(rot2, enc_count2);

  encf(rot1, layers);
  encf(rot2, layers + 1);
}

void encf(int rot, int i) {
  //右回り
  if (rot > 0) {
    keys[0] = pickOneKey(layers, 6);
    sendKeys(RID_KEYBOARD1, keys, layer_keys[layers][6][6]);
    delay(2);
    usb_hid.keyboardRelease(RID_KEYBOARD1);
    Serial.print("enc:");
    Serial.println("+");
  }
  //左回り
  if (rot < 0) {
    keys[0] = pickOneKey(layers, 7);
    sendKeys(RID_KEYBOARD1, keys, layer_keys[layers][7][6]);
    delay(2);
    usb_hid.keyboardRelease(RID_KEYBOARD1);
    Serial.print("enc:");
    Serial.println("-");
  }
}

int ENC_COUNT1(int incoming1, int enc_count1) {
  static int enc_old1 = enc_count1;
  int val_change1 = enc_count1 - enc_old1;

  if (val_change1 != 0) {
    incoming1 += val_change1;
    enc_old1 = enc_count1;
  }
  return incoming1;
}

int ENC_COUNT2(int incoming2, int enc_count2) {
  static int enc_old2 = enc_count2;
  int val_change2 = enc_count2 - enc_old2;

  if (val_change2 != 0) {
    incoming2 += val_change2;
    enc_old2 = enc_count2;
  }
  return incoming2;
}