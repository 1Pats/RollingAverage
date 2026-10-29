# Changelog

## 1.0.0 - 2026-09

- Initial release.
- Template `RollingAverage<T, TAcc = T>` with ring buffer sized at construction.
- Periodic drift correction for floating-point accumulators (compiled out for integer types).
- `tReplaceLastValue()` replaces the most recent reading.
- `ulFreeMemory()` free-heap helper.
- Copying is disabled; a buffer size of 0 is reported as invalid by `bIsValid()`.
