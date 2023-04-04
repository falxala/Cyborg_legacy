#include "Adafruit_TinyUSB.h"
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

//デバイス識別番号
//シリアルで文字 "R" を受信したらこの数字を返す
#define ACK 0x00  //肯定応答
#define EEPROM_SIZE 1024
#define PRESET_ADDRESS 1000
#define BRIGHTNESS_ADDRESS 1004
#define KeySW1 1
#define KeySW2 2
#define KeySW3 3
#define KeySW4 4
#define KeySW5 5
#define KeySW6 6
#define SigA1 10
#define SigB1 9
#define SigA2 11
#define SigB2 12
#define PB1 7
#define PIN 16

String setup_url = "https://falxala.github.io/Cyborg/";
String str = "";
int rot1 = 0;
int rot2 = 0;
int counter = 0;
int layers = 0;
bool state;
uint8_t keys[6] = { 0 };

//エンコーダー状態を記憶
volatile int8_t pos1;
volatile int enc_count1;
volatile uint8_t pos2;
volatile int enc_count2;

int Brightness = 128;
uint8_t rgb_mask[3] = { 1, 1, 1 };

char buff[64] = { '\0' };

volatile uint8_t key = 0b00000000;
volatile uint8_t oldKey = 0b00000000;

int old_layer = 0;
bool layerChangeFlag;
bool sendURI;

int layer_num = 0;
int layer_key_num = 0;
uint8_t temp_keys[7] = { 0 };

//preset 5 ,keys 7 ,code 6 + 1 modifier
//6 keys 4 rotary　10 code
uint8_t layer_keys[6][10][7] = { 0 };

int count = 0;
unsigned long watch = 0;

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

void setup() {
  //default keys
  layer_keys[0][0][0] = HID_KEY_Q;
  layer_keys[0][0][6] = 0x00;
  layer_keys[0][1][0] = HID_KEY_W;
  layer_keys[0][1][6] = 0x00;
  layer_keys[0][2][0] = HID_KEY_E;
  layer_keys[0][2][6] = 0x00;
  layer_keys[0][3][0] = HID_KEY_A;
  layer_keys[0][3][6] = 0x00;
  layer_keys[0][4][0] = HID_KEY_S;
  layer_keys[0][4][6] = 0x00;
  layer_keys[0][5][0] = HID_KEY_D;
  layer_keys[0][5][6] = 0x00;
  layer_keys[0][6][0] = HID_USAGE_CONSUMER_VOLUME_INCREMENT;
  layer_keys[0][6][6] = 0x00;
  layer_keys[0][7][0] = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
  layer_keys[0][7][6] = 0x00;
  layer_keys[0][8][0] = 0x00;
  layer_keys[0][8][6] = 0x00;
  layer_keys[0][9][0] = 0x00;
  layer_keys[0][9][6] = 0x00;


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
  pinMode(KeySW5, INPUT_PULLUP);
  pinMode(KeySW6, INPUT_PULLUP);
  pinMode(PB1, INPUT_PULLUP);

  pinMode(LED_BUILTIN, OUTPUT);
  //pinMode(encGND, OUTPUT);
  //gpio_put(encGND, 0);

  //EEPROMエミュレート開始
  EEPROM.begin(EEPROM_SIZE);

  //rgbLED初期化
  strip.begin();
  strip.show();  // Initialize all pixels to 'off'

  //hid開始
  usb_hid.begin();

  //シリアル通信開始
  Serial.begin(115200);
  // wait until device mounted
  while (!TinyUSBDevice.mounted()) delay(1);
  Serial.println("Begin TinyUSB HID");
  init();  //eepromで初期化

  //起動LED
  for (int i = 0; i < Brightness; i++) {
    strip.setPixelColor(0, i * rgb_mask[0], i * rgb_mask[1], i * rgb_mask[2]);
    strip.show();
    delay(20);
  }
}

void init() {
  uint8_t read_preset = EEPROM.read(PRESET_ADDRESS);
  if (read_preset < 10)
    layers = read_preset;
  else if (read_preset == 255) {
    layers = 0;
  } else {
    layers = 0;
    Serial.print("Memory Error");
  }

  //EEPROM読み取り
  short addres;
  uint8_t value;
  short len1 = sizeof(layer_keys[0][0]);
  short len2 = sizeof(layer_keys[0]);
  short len3 = sizeof(layer_keys);
  for (int i = 0; i < (len3 / len2); i++) {
    for (int j = 0; j < (len2 / len1); j++) {
      for (int k = 0; k < len1; k++) {
        addres = i * 100 + j * 10 + k;
        value = EEPROM.read(addres);
        if (value != 255) {
          layer_keys[i][j][k] = value;
        }
      }
    }
  }

  Brightness = EEPROM.read(BRIGHTNESS_ADDRESS);
  layerState_led(layers);
}

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

  //30秒経過でlayerを永続化
  if (millis() - watch > 30000) {
    if (EEPROM.read(PRESET_ADDRESS) != layers) {
      EEPROM.write(BRIGHTNESS_ADDRESS, Brightness);
      EEPROM.write(PRESET_ADDRESS, layers);
      EEPROM.commit();
      Serial.println("-----AUTO SAVE-----");
      Serial.print("layer=");
      Serial.println(layers);
      Serial.print("brightness=");
      Serial.println(Brightness);
      Serial.println("-------------------");
    }
  }
}