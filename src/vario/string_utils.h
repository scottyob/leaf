#pragma once

#include <Arduino.h>

// Formats seconds into HH:MM:SS (or HH:MM if short)
// If rightAlignTo is set, we'll pad this many characters
String formatSeconds(unsigned long seconds,
                     const bool minified = false,
                     const int rightAlignTo = 0);