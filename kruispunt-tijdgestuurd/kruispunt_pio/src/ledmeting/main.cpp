// LEDMETING: meet per pin de spanning over het lampje bij een heel klein stroompje
// (interne pull-up ~45k). Rood ~1.6V < geel ~1.8V < groen ~1.9-2.6V.
// Geen lampje / los draadje -> ~3V. Alleen pinnen met ADC kunnen gemeten worden.
#include <Arduino.h>
#include <driver/adc.h>
#include <driver/rtc_io.h>
#include <esp_adc_cal.h>

const int leds[] = {13, 14, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33};
const int aantal = sizeof(leds) / sizeof(leds[0]);

esp_adc_cal_characteristics_t cal1, cal2;

// -1 = geen ADC op deze pin
int adc1Kanaal(int p) { return p == 32 ? ADC1_CHANNEL_4 : p == 33 ? ADC1_CHANNEL_5 : -1; }
int adc2Kanaal(int p) {
  switch (p) {
    case 13: return ADC2_CHANNEL_4;
    case 14: return ADC2_CHANNEL_6;
    case 25: return ADC2_CHANNEL_8;
    case 26: return ADC2_CHANNEL_9;
    case 27: return ADC2_CHANNEL_7;
  }
  return -1;
}

int meet(int p) {
  int k1 = adc1Kanaal(p), k2 = adc2Kanaal(p);
  if (k1 < 0 && k2 < 0) return -1;
  if (k1 >= 0) adc1_config_channel_atten((adc1_channel_t)k1, ADC_ATTEN_DB_11);
  else adc2_config_channel_atten((adc2_channel_t)k2, ADC_ATTEN_DB_11);
  rtc_gpio_pullup_en((gpio_num_t)p);
  delay(30);
  uint32_t som = 0;
  for (int i = 0; i < 64; i++) {
    int raw = 0;
    if (k1 >= 0) raw = adc1_get_raw((adc1_channel_t)k1);
    else adc2_get_raw((adc2_channel_t)k2, ADC_WIDTH_BIT_12, &raw);
    som += esp_adc_cal_raw_to_voltage(raw, k1 >= 0 ? &cal1 : &cal2);
  }
  rtc_gpio_pullup_dis((gpio_num_t)p);
  rtc_gpio_deinit((gpio_num_t)p);
  pinMode(p, OUTPUT);
  digitalWrite(p, LOW);
  return som / 64;
}

void setup() {
  Serial.begin(115200);
  for (int p : leds) {
    pinMode(p, OUTPUT);
    digitalWrite(p, LOW);
  }
  adc1_config_width(ADC_WIDTH_BIT_12);
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &cal1);
  esp_adc_cal_characterize(ADC_UNIT_2, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &cal2);
}

void loop() {
  Serial.print("METING");
  for (int p : leds) Serial.printf(" %d=%d", p, meet(p));
  Serial.println();
  delay(1000);
}
