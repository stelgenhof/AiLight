/**
 * Ai-Thinker RGBW Light Firmware - OTA Module
 *
 * The OTA (Over The Air) module holds all the code managing over the air
 * firmware updates.
 *
 * Created by Sacha Telgenhof <stelgenhof at gmail dot com>
 * Copyright (c) 2016 - 2017 Sacha Telgenhof
 */
#define SerialPrint(format, ...)                                               \
  StreamPrint_progmem(Serial, PSTR(format), ##__VA_ARGS__)
#define StreamPrint(stream, format, ...)                                       \
  StreamPrint_progmem(stream, PSTR(format), ##__VA_ARGS__)

#ifdef DEBUG
#define DEBUGLOG(...) SerialPrint(__VA_ARGS__)
#else
#define DEBUGLOG(...)
#endif

/**
 * @brief A program memory version of printf
 *
 * Copy of format string and result share a buffer so as to avoid too much
 * memory use.
 *
 * Credits:  David Pankhurst
 * Source: http://www.utopiamechanicus.com/article/low-memory-serial-print/
 *
 * @param  out the output object (e.g. Serial)
 * @param  format the format string (as used in the printf function and alike)
 * @return void
 */
void StreamPrint_progmem(Print &out, PGM_P format, ...) {
  char formatString[128], *ptr;

  // Copy in from program mem
  strncpy_P(formatString, format, sizeof(formatString));

  // null terminate - leave last char since we might need it in worst case for
  // results \0
  formatString[sizeof(formatString) - 2] = '\0';
  ptr = &formatString[strlen(formatString) + 1]; // our result buffer...

  va_list args;
  va_start(args, format);
  vsnprintf(ptr, sizeof(formatString) - 1 - strlen(formatString), formatString,
            args);
  va_end(args);
  formatString[sizeof(formatString) - 1] = '\0';

  out.print(ptr);
}
