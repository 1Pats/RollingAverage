/*
 * MultiTypeAverage
 * Demonstrates several independent RollingAverage instances,
 * each with its own value type, accumulator type and buffer size.
 */
#include <RollingAverage.h>

RollingAverage<uint16_t, uint64_t> oPhotoAverage(50);      // fast-changing light sensor, integer
RollingAverage<float, double>      oTempAverage(30);       // slow-changing temperature, floating point
RollingAverage<int64_t>            oDeltaAverage(10);      // signed values, small buffer, no drift correction needed

void setup() {
  Serial.begin(115200);

  bool bAllValid = oPhotoAverage.bIsValid() && oTempAverage.bIsValid() && oDeltaAverage.bIsValid();
  if (!bAllValid) {
     Serial.println("*** MEMORY ERROR ***");
     while (true) delay(1000);
  }
}

void loop() {
  uint16_t uiPhoto = analogRead(A0);
  float    fTemp   = 21.5 + (float)(random(-50, 50)) / 100.0;   // simulated sensor reading
  int64_t  lDelta  = random(-1000, 1000);

  Serial.print("Photo avg: ");
  Serial.print(oPhotoAverage.tGetRollingAverage(uiPhoto));
  Serial.print("  Temp avg: ");
  Serial.print(oTempAverage.tGetRollingAverage(fTemp), 3);
  Serial.print("  Delta avg: ");
  Serial.println((long)oDeltaAverage.tGetRollingAverage(lDelta));

  delay(200);
}
