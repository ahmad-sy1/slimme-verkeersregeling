// LED VOLTAGE: measures per pin the voltage over the LED at a very small current
// (internal pull-up ~45k). Red ~1.6V < orange ~1.8V < green ~1.9-2.6V.
// No LED / loose wire -> ~3V. Only pins with an ADC can be measured.
#include <Arduino.h>
#include <driver/adc.h>
#include <driver/rtc_io.h>
#include <esp_adc_cal.h>
#include <VriConfig.h>

esp_adc_cal_characteristics_t cal1, cal2;

// -1 = no ADC on this pin
int adc1Channel(int p) { return p == 32 ? ADC1_CHANNEL_4 : p == 33 ? ADC1_CHANNEL_5 : -1; }
int adc2Channel(int p) {
  switch (p) {
    case 13: return ADC2_CHANNEL_4;
    case 14: return ADC2_CHANNEL_6;
    case 25: return ADC2_CHANNEL_8;
    case 26: return ADC2_CHANNEL_9;
    case 27: return ADC2_CHANNEL_7;
  }
  return -1;
}

int measureMillivolts(int p) {
  int k1 = adc1Channel(p), k2 = adc2Channel(p);
  if (k1 < 0 && k2 < 0) return -1;
  if (k1 >= 0) adc1_config_channel_atten((adc1_channel_t)k1, ADC_ATTEN_DB_12);
  else adc2_config_channel_atten((adc2_channel_t)k2, ADC_ATTEN_DB_12);
  rtc_gpio_pullup_en((gpio_num_t)p);
  delay(30);
  uint32_t sum = 0;
  for (int i = 0; i < 64; i++) {
    int raw = 0;
    if (k1 >= 0) raw = adc1_get_raw((adc1_channel_t)k1);
    else adc2_get_raw((adc2_channel_t)k2, ADC_WIDTH_BIT_12, &raw);
    sum += esp_adc_cal_raw_to_voltage(raw, k1 >= 0 ? &cal1 : &cal2);
  }
  rtc_gpio_pullup_dis((gpio_num_t)p);
  rtc_gpio_deinit((gpio_num_t)p);
  pinMode(p, OUTPUT);
  digitalWrite(p, LOW);
  return sum / 64;
}

void setup() {
  Serial.begin(115200);
  for (int p : SLAVE_LED_PINS) {
    pinMode(p, OUTPUT);
    digitalWrite(p, LOW);
  }
  adc1_config_width(ADC_WIDTH_BIT_12);
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 1100, &cal1);
  esp_adc_cal_characterize(ADC_UNIT_2, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 1100, &cal2);
}

void loop() {
  Serial.print("MEASUREMENT");
  for (int p : SLAVE_LED_PINS) Serial.printf(" %d=%d", p, measureMillivolts(p));
  Serial.println();
  delay(1000);
}
