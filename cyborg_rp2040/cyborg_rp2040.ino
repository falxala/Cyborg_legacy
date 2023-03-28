//デバイス識別番号
//シリアルで文字 "R" を受信したらこの数字を返す
#define ACK 0x00  //肯定応答
#define EEPROM_SIZE 1024
#define PRESET_ADDRESS 1000
#define BRIGHTNESS_ADDRESS 1004

#include "Adafruit_TinyUSB.h"
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

#define PIN 23
/*
#define PB1 27
#define SigA1 28
#define SigB1 29
#define SigA2 13
#define SigB2 14
#define KeySW1 2
#define KeySW2 3
#define KeySW3 4
#define KeySW4 5
#define KeySW5 6
#define KeySW6 7
*/
#define KeySW1 6
#define KeySW2 7
#define KeySW3 8
#define KeySW4 9
#define KeySW5 10
#define KeySW6 11
#define SigA1 20
#define SigB1 21
#define SigA2 13
#define SigB2 14
#define PB1 18
#define encGND 19

int rot1 = 0;
int rot2 = 0;
int counter = 0;
int preset = 0;
bool state;
uint8_t keys[6] = { 0 };

static const int iRotPtn[] = { 0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0, 0 };

//エンコーダー状態を記憶
volatile int8_t pos1;
volatile int enc_count1;
volatile uint8_t pos2;
volatile int enc_count2;

int Brightness = 128;

char buff[64] = { '\0' };
String str = "";

volatile uint8_t key = 0b00000000;
volatile uint8_t oldKey = 0b00000000;

//WS2812 本機では1つ
Adafruit_NeoPixel strip =
  Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);

// Report ID
enum {
  RID_KEYBOARD1 = 1,
  RID_KEYBOARD2,
  RID_KEYBOARD3,
  RID_KEYBOARD4,
  RID_KEYBOARD5,
  RID_KEYBOARD6,
  RID_CONSUMER_CONTROL,  // Media, volume etc ..
};

// HID report descriptor using TinyUSB's template
uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(RID_KEYBOARD1)),
  TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(RID_KEYBOARD2)),
  TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(RID_KEYBOARD3)),
  TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(RID_KEYBOARD4)),
  TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(RID_KEYBOARD5)),
  TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(RID_KEYBOARD6)),
  TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(RID_CONSUMER_CONTROL))
};

Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_KEYBOARD, 2, false);

//preset 5 ,keys 7 ,code 6 + 1 modifier
//6 keys 4 rotary　10 code
uint8_t preset_keys[5][10][7] = { 0 };

void setup() {
  //default keys
  preset_keys[0][0][0] = HID_KEY_D;
  preset_keys[0][1][0] = HID_KEY_S;
  preset_keys[0][2][0] = HID_KEY_A;
  preset_keys[0][3][0] = HID_KEY_W;
  preset_keys[0][6][0] = HID_USAGE_CONSUMER_VOLUME_INCREMENT;
  preset_keys[0][7][0] = HID_USAGE_CONSUMER_VOLUME_DECREMENT;

  preset_keys[1][0][0] = HID_KEY_Z;
  preset_keys[1][0][6] = KEYBOARD_MODIFIER_LEFTGUI | KEYBOARD_MODIFIER_LEFTSHIFT;
  preset_keys[1][1][0] = HID_KEY_S;
  preset_keys[1][1][6] = KEYBOARD_MODIFIER_LEFTGUI;
  preset_keys[1][2][0] = HID_KEY_Z;
  preset_keys[1][2][6] = KEYBOARD_MODIFIER_LEFTGUI;
  preset_keys[1][3][0] = HID_KEY_V;
  preset_keys[1][3][6] = KEYBOARD_MODIFIER_LEFTGUI;
  preset_keys[1][6][0] = HID_KEY_ARROW_RIGHT;
  preset_keys[1][7][0] = HID_KEY_ARROW_LEFT;

  preset_keys[2][0][0] = HID_KEY_Z;
  preset_keys[2][0][6] = KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT;
  preset_keys[2][1][0] = HID_KEY_S;
  preset_keys[2][1][6] = KEYBOARD_MODIFIER_LEFTCTRL;
  preset_keys[2][2][0] = HID_KEY_Z;
  preset_keys[2][2][6] = KEYBOARD_MODIFIER_LEFTCTRL;
  preset_keys[2][3][0] = HID_KEY_V;
  preset_keys[2][3][6] = KEYBOARD_MODIFIER_LEFTCTRL;
  preset_keys[2][6][0] = HID_KEY_ARROW_RIGHT;
  preset_keys[2][7][0] = HID_KEY_ARROW_LEFT;

  preset_keys[3][0][0] = HID_KEY_Z;
  preset_keys[3][0][6] = KEYBOARD_MODIFIER_LEFTGUI | KEYBOARD_MODIFIER_LEFTSHIFT;
  preset_keys[3][1][0] = HID_KEY_E;
  preset_keys[3][1][6] = 0;
  preset_keys[3][2][0] = HID_KEY_Z;
  preset_keys[3][2][6] = KEYBOARD_MODIFIER_LEFTGUI;
  preset_keys[3][3][0] = HID_KEY_P;
  preset_keys[3][3][6] = 0;
  preset_keys[3][6][0] = HID_KEY_BACKSLASH;
  preset_keys[3][7][0] = HID_KEY_BRACKET_RIGHT;

  preset_keys[4][0][0] = HID_KEY_Z;
  preset_keys[4][0][6] = KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT;
  preset_keys[4][1][0] = HID_KEY_SPACE;
  preset_keys[4][1][6] = 0;
  preset_keys[4][2][0] = HID_KEY_Z;
  preset_keys[4][2][6] = KEYBOARD_MODIFIER_LEFTCTRL;
  preset_keys[4][3][0] = HID_KEY_K;
  preset_keys[4][3][6] = KEYBOARD_MODIFIER_LEFTCTRL;
  preset_keys[4][6][0] = HID_KEY_ARROW_RIGHT;
  preset_keys[4][7][0] = HID_KEY_ARROW_LEFT;

#if defined(ARDUINO_ARCH_MBED) && defined(ARDUINO_ARCH_RP2040)
  // Manual begin() is required on core without built-in support for TinyUSB such as mbed rp2040
  TinyUSB_Device_Init(0);
#endif

  //PIN設定より前に書かないとプルアップされない
  //割り込み
  attachInterrupt(SigA1, interrupt_enc, CHANGE);
  attachInterrupt(SigB1, interrupt_enc, CHANGE);

  //ピン設定
  pinMode(SigA1, INPUT_PULLUP);
  pinMode(SigB1, INPUT_PULLUP);
  pinMode(KeySW1, INPUT_PULLUP);
  pinMode(KeySW2, INPUT_PULLUP);
  pinMode(KeySW3, INPUT_PULLUP);
  pinMode(KeySW4, INPUT_PULLUP);
  pinMode(PB1, INPUT_PULLUP);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(encGND, OUTPUT);
  gpio_put(encGND, 0);

  //EEPROMエミュレート開始
  EEPROM.begin(EEPROM_SIZE);

  strip.begin();
  strip.show();  // Initialize all pixels to 'off'
  usb_hid.begin();

  //起動LED
  rainbowCycle(5);
  //シリアル通信開始
  Serial.begin(115200);
  // wait until device mounted
  while (!TinyUSBDevice.mounted()) delay(1);
  Serial.println("Adafruit TinyUSB HID");
  init();  //eepromで初期化
}

int count = 0;
unsigned long watch = 0;
void loop() {
  Switch_function(0);
  RotEncFunc();
  read_keys();

  count = count + 1;
  if (count > 0)
    gpio_put(LED_BUILTIN, 1);
  else
    gpio_put(LED_BUILTIN, 0);
  if (count > 10)
    count = -90;

  //30秒経過でpresetを永続化
  if (millis() - watch > 30000) {
    if (EEPROM.read(PRESET_ADDRESS) != preset) {
      EEPROM.write(BRIGHTNESS_ADDRESS, Brightness);
      EEPROM.write(PRESET_ADDRESS, preset);
      EEPROM.commit();
      Serial.print("preset=");
      Serial.println(preset);
      Serial.print("brightness=");
      Serial.println(Brightness);
    }
  }
}

void init() {
  uint8_t read_preset = EEPROM.read(PRESET_ADDRESS);
  if (read_preset < 10)
    preset = read_preset;
  else if (read_preset == 255) {
    preset = 0;
  } else {
    preset = 0;
    Serial.print("Memory Error");
  }

  //EEPROM読み取り
  short addres;
  uint8_t value;
  short len1 = sizeof(preset_keys[0][0]);
  short len2 = sizeof(preset_keys[0]);
  short len3 = sizeof(preset_keys);
  for (int i = 0; i < (len3 / len2); i++) {
    for (int j = 0; j < (len2 / len1); j++) {
      for (int k = 0; k < len1; k++) {
        addres = i * 100 + j * 10 + k;
        value = EEPROM.read(addres);
        if (value != 255) {
          preset_keys[i][j][k] = value;
        }
      }
    }
  }

  Brightness = EEPROM.read(BRIGHTNESS_ADDRESS);
  preset_led(preset);
}

int old_preset = 0;
bool flag;
void read_keys() {
  old_preset = preset;
  flag = false;
  while (gpio_get(PB1) == 0) {
    count = count + 10000;

    gpio_put(LED_BUILTIN, 1);
    delay(10);
    rot1 = 0;
    rot1 = ENC_COUNT1(rot1, enc_count1);
    if (rot1 > 0) {
      preset = preset + 1;
      flag = true;
      count = 0;
    }
    if (rot1 < 0) {
      preset = preset - 1;
      flag = true;
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

    if (flag == false)
      preset = old_preset + 1;

    if (preset > 4)
      preset = 0;
    if (preset < 0)
      preset = 4;

    if (count > 1000000) {
      strip.setPixelColor(0, 0, 0, 0);
      strip.show();
      Serial.println("-----EEPROM RESET -----");
      for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(i, 255);
      }
      EEPROM.commit();
      delay(1000);
      preset = 0;
    }
    preset_led(preset);
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
      keys[0] = pickoneKey(preset, 0);
      sendKey(RID_KEYBOARD1, keys, preset_keys[preset][0][6]);
    } else if ((oldKey & 0b00000001) != 0b00000000) {
      usb_hid.keyboardRelease(RID_KEYBOARD1);
      delay(2);  //delayを入れないと解放されない場合がある
    }

    if ((key & 0b00000010) == 0b00000010) {
      keys[0] = pickoneKey(preset, 1);
      sendKey(RID_KEYBOARD2, keys, preset_keys[preset][1][6]);
    } else if ((oldKey & 0b00000010) != 0b00000000) {
      usb_hid.keyboardRelease(RID_KEYBOARD2);
      delay(2);
    }

    if ((key & 0b00000100) == 0b00000100) {
      keys[0] = pickoneKey(preset, 2);
      sendKey(RID_KEYBOARD3, keys, preset_keys[preset][2][6]);
    } else if ((oldKey & 0b00000100) != 0b00000000) {
      usb_hid.keyboardRelease(RID_KEYBOARD3);
      delay(2);
    }

    if ((key & 0b00001000) == 0b00001000) {
      keys[0] = pickoneKey(preset, 3);
      sendKey(RID_KEYBOARD4, keys, preset_keys[preset][3][6]);
    } else if ((oldKey & 0b00001000) != 0b00000000) {
      usb_hid.keyboardRelease(RID_KEYBOARD4);
      delay(2);
    }

    if ((key & 0b00010000) == 0b00010000) {
      keys[0] = pickoneKey(preset, 4);
      sendKey(RID_KEYBOARD5, keys, preset_keys[preset][4][6]);
    } else if ((oldKey & 0b00010000) != 0b00000000) {
      usb_hid.keyboardRelease(RID_KEYBOARD5);
      delay(2);
    }

    if ((key & 0b00100000) == 0b00100000) {
      keys[0] = pickoneKey(preset, 5);
      sendKey(RID_KEYBOARD6, keys, preset_keys[preset][5][6]);
    } else if ((oldKey & 0b00100000) != 0b00000000) {
      usb_hid.keyboardRelease(RID_KEYBOARD6);
      delay(2);
    }
    oldKey = key;
  }
}

void preset_led(int s) {
  switch (s) {
    case 0:
      strip.setPixelColor(0, Brightness, Brightness, Brightness);
      strip.show();
      break;
    case 1:
      strip.setPixelColor(0, Brightness, 0, 0);
      strip.show();
      break;
    case 2:
      strip.setPixelColor(0, 0, Brightness, 0);
      strip.show();
      break;
    case 3:
      strip.setPixelColor(0, 0, 0, Brightness);
      strip.show();
      break;
    case 4:
      strip.setPixelColor(0, Brightness, 0, Brightness);
      strip.show();
      break;
  }
}

uint8_t pickoneKey(int num, int index) {
  return preset_keys[num][index][0];
}

//6文字のHEX文字列をIntに
uint16_t hexToInt(String str) {
  const char hex[6] = { str[0], str[1], str[2], str[3], str[4], str[5] };
  return strtol(hex, NULL, 16);
}

int preset_num = 0;
int preset_key_num = 0;
uint8_t temp_keys[7] = { 0 };
void Switch_function(int input) {

  if (Serial.available()) {
    str = Serial.readStringUntil('\n');

  } else if (str != "") {
    if (str.substring(0, string_cut(str, '_')) == "M") {

      str = str.substring(1 + string_cut(str, '_'));
      preset_num = str.substring(0, string_cut(str, '_')).toInt();

      str = str.substring(1 + string_cut(str, '_'));
      preset_key_num = str.substring(0, string_cut(str, '_')).toInt();

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

        Serial.print(preset_num);
        Serial.print("_");
        Serial.print(preset_key_num);
        Serial.print("|");

        int addres = 0;
        for (int i = 0; i < 7; i++) {
          addres = preset_num * 100 + preset_key_num * 10 + i;
          Serial.print(addres);
          Serial.print("|");
          Serial.println(temp_keys[i]);
          EEPROM.write(addres, temp_keys[i]);
        }

        EEPROM.commit();  //永続化

        //セット
        for (int i = 0; i < 7; i++) {
          addres = preset_num * 100 + preset_key_num * 10 + i;
          preset_keys[preset_num][preset_key_num][i] = EEPROM.read(addres);
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
      preset_led(preset);
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

  encf(rot1, preset);
  encf(rot2, preset + 1);
}

void encf(int rot, int i) {
  //右回り
  if (rot > 0) {
    keys[0] = pickoneKey(preset, 6);
    sendKey(RID_CONSUMER_CONTROL, keys, 0x00);
  }
  //左回り
  if (rot < 0) {
    keys[0] = pickoneKey(preset, 7);
    sendKey(RID_CONSUMER_CONTROL, keys, 0x00);
  }
}

void sendKey(uint8_t report_id, uint8_t keycode[6], uint8_t modifier) {

  TinyUSBDevice.remoteWakeup();
  if (!usb_hid.ready()) return;

  switch (report_id) {
    case RID_KEYBOARD1:
      usb_hid.keyboardReport(RID_KEYBOARD1, modifier, keycode);
      delay(2);
      break;

    case RID_KEYBOARD2:
      usb_hid.keyboardReport(RID_KEYBOARD2, modifier, keycode);
      delay(2);
      break;

    case RID_KEYBOARD3:
      usb_hid.keyboardReport(RID_KEYBOARD3, modifier, keycode);
      delay(2);
      break;

    case RID_KEYBOARD4:
      usb_hid.keyboardReport(RID_KEYBOARD4, modifier, keycode);
      delay(2);
      break;

    case RID_KEYBOARD5:
      usb_hid.keyboardReport(RID_KEYBOARD5, modifier, keycode);
      delay(2);
      break;

    case RID_KEYBOARD6:
      usb_hid.keyboardReport(RID_KEYBOARD6, modifier, keycode);
      delay(2);
      break;


    case RID_CONSUMER_CONTROL:
      if (keycode[0] > 0xcd) {
        usb_hid.sendReport16(RID_CONSUMER_CONTROL, keycode[0]);
        delay(3);
        usb_hid.sendReport16(RID_CONSUMER_CONTROL, 0);
      } else {
        usb_hid.keyboardReport(RID_KEYBOARD1, modifier, keycode);
        delay(3);
        usb_hid.keyboardRelease(RID_KEYBOARD1);
      }
      break;
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

//割り込みで変化を読み取る
void interrupt_enc(void) {
  byte cur1 = (!digitalRead(SigB1) << 1) + !digitalRead(SigA1);
  byte old1 = pos1 & 0b00000011;
  byte dir1 = (pos1 & 0b00110000) >> 4;

  if (cur1 == 3) cur1 = 2;
  else if (cur1 == 2) cur1 = 3;

  if (cur1 != old1) {
    if (dir1 == 0) {
      if (cur1 == 1 || cur1 == 3) dir1 = cur1;
    } else {
      if (cur1 == 0) {
        if (dir1 == 1 && old1 == 3) enc_count1++;
        else if (dir1 == 3 && old1 == 1) enc_count1--;
        dir1 = 0;
      }
    }

    bool rote1 = 0;
    if (cur1 == 3 && old1 == 0) rote1 = 0;
    else if (cur1 == 0 && old1 == 3) rote1 = 1;
    else if (cur1 > old1) rote1 = 1;

    pos1 = (dir1 << 4) + (old1 << 2) + cur1;
  }
}

void interrupt_enc2(void) {
  byte cur2 = (!digitalRead(SigB2) << 1) + !digitalRead(SigA2);
  byte old2 = pos2 & 0b00000011;
  byte dir2 = (pos2 & 0b00110000) >> 4;

  if (cur2 == 3) cur2 = 2;
  else if (cur2 == 2) cur2 = 3;

  if (cur2 != old2) {
    if (dir2 == 0) {
      if (cur2 == 1 || cur2 == 3) dir2 = cur2;
    } else {
      if (cur2 == 0) {
        if (dir2 == 1 && old2 == 3) enc_count2++;
        else if (dir2 == 3 && old2 == 1) enc_count2--;
        dir2 = 0;
      }
    }

    bool rote2 = 0;
    if (cur2 == 3 && old2 == 0) rote2 = 0;
    else if (cur2 == 0 && old2 == 3) rote2 = 1;
    else if (cur2 > old2) rote2 = 1;

    pos2 = (dir2 << 4) + (old2 << 2) + cur2;
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 1; j++) {  // 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
