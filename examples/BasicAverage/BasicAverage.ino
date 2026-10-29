/*
 * BasicAverage
 * Demonstrates RollingAverage with a single integer type,
 * smoothing an analog photoresistor reading.
 */
#include <RollingAverage.h>

#define PHOTO_PIN A0

RollingAverage<uint16_t, uint64_t> oPhotoAverage(50);   // stores uint16_t, sums into uint64_t

void setup() {
  Serial.begin(115200);

  if (!oPhotoAverage.bIsValid()) {                       // check allocation succeeded
     Serial.println("*** MEMORY ERROR ***");
     while (true) delay(1000);
  }

  Serial.print("Free memory at start: ");
  Serial.println(ulFreeMemory());
}

void loop() {
  uint16_t uiPhoto = analogRead(PHOTO_PIN);
  uint16_t uiAvg   = oPhotoAverage.tGetRollingAverage(uiPhoto);

  Serial.print("Raw: ");
  Serial.print(uiPhoto);
  Serial.print("  Avg (");
  Serial.print(oPhotoAverage.uiFilledCount());
  Serial.print("/");
  Serial.print(oPhotoAverage.uiCapacity());
  Serial.print("): ");
  Serial.println(uiAvg);

  delay(100);
}
