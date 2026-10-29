#ifndef ROLLING_AVERAGE_H
#define ROLLING_AVERAGE_H

#include <Arduino.h>
#include <type_traits>

/*
 * RollingAverage - accumulates readings in a ring buffer and returns
 * the running average of the last N values.
 * 1Pats September 2026
 * Code developed according to KISS principle
 *
 * Buffer size (N) and the stored value type are configurable per instance.
 *
 * Template parameters:
 *   T     - type of the individual stored readings (e.g. uint16_t, uint32_t, int64_t, float, double)
 *   TAcc  - type used for the running sum (accumulator). Defaults to T.
 *           Choose a wider type than T if T is narrow and/or the buffer is large,
 *           to avoid the sum overflowing (e.g. T = uint16_t, TAcc = uint64_t).
 *
 * Integer averages are truncated (rounded down), e.g. 2.9 is returned as 2.
 * Objects cannot be copied; pass them by reference.
 *
 * For floating-point TAcc, the running sum is periodically recalculated from
 * scratch (once per full buffer lap) to correct accumulated rounding drift that
 * repeated add/subtract operations introduce over long-term operation.
 * For integer TAcc this recalculation is skipped at compile time - no extra
 * code is generated, since integer add/subtract is always exact.
 * Using a large buffer size produces a smoother, well-rounded average and
 * indirectly implements hysteresis. At the same time, it may slow down
 * the system's reaction time.
 *
 * Examples:
 * RollingAverage<uint16_t, uint64_t> oPhotoAverage(50);       // stores uint16_t, sums into uint64_t - safe from overflow
 * RollingAverage<uint32_t, uint64_t> oPressureAverage(20);    // stores uint32_t readings
 * RollingAverage<int64_t>            oTempDeltaAverage(10);   // signed values, no explicit TAcc -> defaults to int64_t itself
 * RollingAverage<uint64_t>           oCounterAverage(100);    // same as before, one type for both
 */
template <typename T, typename TAcc = T>
class RollingAverage {
  public:
    // uiSize - how many recent readings to average over
    explicit RollingAverage(uint16_t uiSize) : ptValues(NULL), uiSize(uiSize), uiIndex(0), uiCount(0), tTotal(0) {
        if (uiSize == 0) return;                                        // size 0 is invalid - bIsValid() reports false
        ptValues = (T*)malloc((size_t)uiSize * sizeof(T));               // allocate buffer for uiSize elements
        if (ptValues != NULL) memset(ptValues, 0, (size_t)uiSize * sizeof(T));   // zero it out
    }

    ~RollingAverage(){
        if (ptValues != NULL) free(ptValues);                           // release buffer
    }

    // Copying is disabled - copies would share one buffer and free it twice
    RollingAverage(const RollingAverage&) = delete;
    RollingAverage& operator=(const RollingAverage&) = delete;

    // Feed a new reading, get back the current rolling average
    T tGetRollingAverage(T tCurrentValue){
        if (ptValues == NULL) return tCurrentValue;                     // allocation failed - fall back to raw value

        tTotal -= (TAcc)ptValues[uiIndex];                              // remove value being replaced
        ptValues[uiIndex] = tCurrentValue;                              // store new value
        tTotal += (TAcc)tCurrentValue;                                  // add new value

        if (++uiIndex >= uiSize) {                                      // wrap index if needed
            uiIndex = 0;
            vRecalculateTotal();                                        // full buffer lap done - resync sum for float/double, no-op for integers
        }
        if (uiCount < uiSize) uiCount++;                                // increase valid count

        return (T)(tTotal / (TAcc)uiCount);                             // calculate average
    }

    // Replace the most recent reading with a new value, get back the current rolling average
    T tReplaceLastValue(T tCurrentValue){
        if (ptValues == NULL) return tCurrentValue;                     // allocation failed - fall back to raw value
        if (uiCount == 0) return tGetRollingAverage(tCurrentValue);     // nothing to replace yet - store as a normal reading

        uint16_t uiLast = (uiIndex == 0) ? uiSize - 1 : uiIndex - 1;    // position of the most recent reading
        tTotal -= (TAcc)ptValues[uiLast];                               // remove the reading being replaced
        ptValues[uiLast] = tCurrentValue;                               // store new value
        tTotal += (TAcc)tCurrentValue;                                  // add new value

        return (T)(tTotal / (TAcc)uiCount);                             // calculate average
    }

    bool bIsValid() const { return ptValues != NULL; }                  // check if allocation succeeded
    uint16_t uiCapacity() const { return uiSize; }                      // configured buffer size
    uint16_t uiFilledCount() const { return uiCount; }                  // how many real readings so far

  private:
    // Dispatches to the floating-point or integer version at COMPILE TIME.
    // The unused overload isn't just skipped - it's never generated for this
    // instantiation, so integer types pay zero cost for this feature.
    void vRecalculateTotal(){
        vRecalculateTotalImpl(std::integral_constant<bool, std::is_floating_point<TAcc>::value>());
    }

    // TAcc is float/double - resum the whole buffer to correct rounding drift
    void vRecalculateTotalImpl(std::true_type){
        TAcc tSum = 0;
        for (uint16_t ui = 0; ui < uiSize; ui++) tSum += (TAcc)ptValues[ui];   // sum every slot - buffer is fully populated by this point
        tTotal = tSum;
    }

    // TAcc is an integer type - add/subtract is always exact, nothing to correct
    // (inlining and dead-code elimination will fully eliminate this function at compile time)
    void vRecalculateTotalImpl(std::false_type){
        // intentionally empty
    }

    T        *ptValues;      // ring buffer of readings
    uint16_t  uiSize;        // configured number of elements
    uint16_t  uiIndex;       // next write position
    uint16_t  uiCount;       // number of valid readings collected so far (<= uiSize)
    TAcc      tTotal;        // running sum of buffer contents (wider type to avoid overflow)
};

/*
 * Helper function
 * Returns amount of available heap/SRAM, platform-independent
 * Marked inline since this header may be included in more than one
 * translation unit within the same project.
 */
inline uint32_t ulFreeMemory() {
#if defined(ESP32) || defined(ESP8266)
  return ESP.getFreeHeap();
#elif defined(__AVR__)
  extern int __heap_start, *__brkval;
  int v;
  return (uint32_t)((int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval));
#else
  #warning "ulFreeMemory() not implemented for this platform"
  return 0;
#endif
}

#endif // ROLLING_AVERAGE_H
