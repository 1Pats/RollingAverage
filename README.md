# RollingAverage

A generic, template-based rolling (moving) average class for ESP32 (Arduino core), using a
ring buffer. Buffer size and the stored/accumulator value types are
configurable per instance, so a single header works for `uint16_t`,
`uint32_t`, `int64_t`, `float`, `double`, or any other numeric type.

## Features

- Configurable buffer size at construction time (`RollingAverage<T> obj(size);`)
- Independent value type (`T`) and accumulator type (`TAcc`), to avoid sum
  overflow when averaging many narrow-type readings
  (e.g. `RollingAverage<uint16_t, uint64_t>`)
- Automatic periodic drift correction for floating-point accumulators
  (resums the buffer once per full lap)
- Zero runtime cost for integer accumulators — the drift-correction code path
  is eliminated entirely at compile time
- Includes `ulFreeMemory()`, a free-heap helper

## Installation

1. Download/clone this folder as `RollingAverage`.
2. Place it under your Arduino `libraries` folder
   (usually `Documents/Arduino/libraries/RollingAverage`).
3. Restart the Arduino IDE. The examples appear under
   **File > Examples > RollingAverage**.

## Usage

```cpp
#include <RollingAverage.h>

RollingAverage<uint16_t, uint64_t> oPhotoAverage(50);   // stores uint16_t, sums into uint64_t

void setup() {
  Serial.begin(115200);
  if (!oPhotoAverage.bIsValid()) {
     Serial.println("*** MEMORY ERROR ***");
     while (true) delay(1000);
  }
}

void loop() {
  uint16_t uiPhoto = analogRead(A0);
  uint16_t uiAvg   = oPhotoAverage.tGetRollingAverage(uiPhoto);
  Serial.println(uiAvg);
}
```

See `examples/BasicAverage` and `examples/MultiTypeAverage` for more.

## API

| Method | Returns | Description |
|---|---|---|
| `RollingAverage<T, TAcc = T>(uint16_t uiSize)` | - | Constructor. Allocates a ring buffer of `uiSize` elements (`uiSize` must be at least 1). |
| `T tGetRollingAverage(T tCurrentValue)` | `T` | Feeds a new reading, returns the current average. |
| `T tReplaceLastValue(T tCurrentValue)` | `T` | Replaces the most recent reading (e.g. a corrected or discarded value), returns the current average. With no readings yet, it stores the value as a normal reading. |
| `bool bIsValid()` | `bool` | `true` if the internal buffer was allocated successfully. Check this after construction. |
| `uint16_t uiCapacity()` | `uint16_t` | Configured buffer size. |
| `uint16_t uiFilledCount()` | `uint16_t` | Number of real readings collected so far (≤ capacity). |
| `uint32_t ulFreeMemory()` | `uint32_t` | Free heap in bytes. |

## Notes

- Using a large buffer size produces a smoother, well-rounded average and
  indirectly implements hysteresis, at the cost of slower reaction to real
  changes in the input.
- Memory is allocated with `malloc()` at construction time; check
  `bIsValid()` before use. A size of 0 is treated as a failed allocation.
- Integer averages are truncated (rounded down), e.g. 2.9 is returned as 2.
- Objects cannot be copied (the compiler rejects it); pass them by reference,
  e.g. `void vShow(RollingAverage<float> &oAvg)`.
