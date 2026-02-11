#pragma once

#include <SoftTimers.h>

class RgbLedBlinker {
  public:
    // Color structure for RGB LED
    struct Color24 {
      uint8_t r;
      uint8_t g;
      uint8_t b;

      constexpr bool operator==(const Color24& other) const noexcept { return (r == other.r) && (g == other.g) && (b == other.b); }
      constexpr bool operator!=(const Color24& other) const noexcept { return !(*this == other); }
    };

    // Define color constants
    static constexpr Color24 COLOR_BLACK    = {  0,   0,   0};
    static constexpr Color24 COLOR_WHITE    = {255, 255, 255};
    static constexpr Color24 COLOR_RED      = {255,   0,   0};
    static constexpr Color24 COLOR_GREEN    = {  0, 255,   0};
    static constexpr Color24 COLOR_BLUE     = {  0,   0, 255};
    static constexpr Color24 COLOR_YELLOW   = {255, 255,   0};
    static constexpr Color24 COLOR_CYAN     = {  0, 255, 255};
    static constexpr Color24 COLOR_MAGENTA  = {255,   0, 255};
    static constexpr Color24 COLOR_ORANGE   = {255, 128,   0}; // Red + half Green
    static constexpr Color24 COLOR_PURPLE   = {128,   0, 255}; // Blue + half Red
    static constexpr Color24 COLOR_TEAL     = {  0, 128, 255}; // Blue + half Green
    static constexpr Color24 COLOR_LIME     = {128, 255,   0}; // Green + half Red

    static constexpr const Color24* colors[] = {
      &COLOR_BLACK  ,
      &COLOR_WHITE  ,
      &COLOR_RED    ,
      &COLOR_GREEN  ,
      &COLOR_BLUE   ,
      &COLOR_YELLOW ,
      &COLOR_CYAN   ,
      &COLOR_MAGENTA,
      &COLOR_ORANGE ,
      &COLOR_PURPLE ,
      &COLOR_TEAL   ,
      &COLOR_LIME   ,
    };
    static constexpr size_t colors_count = sizeof(colors)/sizeof(colors[0]);
    
    // Mode structure to define how we blink the LED
    enum Mode {
      MODE_OFF,
      MODE_SOLID,
      MODE_BLINK_SLOW,
      MODE_BLINK_NORMAL,
      MODE_BLINK_FAST,
      MODE_PULSE_ONCE_PER_HALF_SECOND,
      MODE_PULSE_ONCE_PER_SECOND,
      MODE_PULSE_ONCE_PER_15_SECONDS,
      MODE_PULSE_ONCE_PER_30_SECONDS,
      MODE_PULSE_ONCE_PER_MINUTE,
    };
    static const int MODE_LAST_VALUE = MODE_PULSE_ONCE_PER_MINUTE;

  private:
    // Configurations based on Modes
    enum Test {
      TEST_EQUALS,
      TEST_GT, // Greater than
      TEST_LT, // Lower than
    };
    struct Config {
      unsigned long modulo;
      Test test;
      unsigned long value;
    };
    static constexpr Config configs[] = {
      {  1, TEST_EQUALS, 99},    // MODE_OFF
      {  1, TEST_EQUALS,  0},    // MODE_SOLID
      { 40, TEST_LT,     20},    // MODE_BLINK_SLOW   (2 seconds per 4 seconds)
      {  9, TEST_LT,      3},    // MODE_BLINK_NORMAL (300ms per 900ms)
      {  2, TEST_EQUALS,  0},    // MODE_BLINK_FAST   (100 ms per 200 ms)
      {  5, TEST_EQUALS,  0},    // MODE_PULSE_ONCE_PER_HALF_SECOND
      { 10, TEST_EQUALS,  0},    // MODE_PULSE_ONCE_PER_SECOND
      {150, TEST_EQUALS,  0},    // MODE_PULSE_ONCE_PER_15_SECONDS
      {300, TEST_EQUALS,  0},    // MODE_PULSE_ONCE_PER_30_SECONDS
      {600, TEST_EQUALS,  0},    // MODE_PULSE_ONCE_PER_MINUTE
    };

  private:
    uint8_t pin;
    SoftTimer timer; // millisecond timer
    Mode mode;
    Color24 color;

  public:
    RgbLedBlinker() :
      pin(pin),
      mode(MODE_OFF),
      color(COLOR_BLACK) {
    }

    void set(Mode mode, const Color24 &color) {
      bool has_changed = (this->mode != mode || this->color != color);

      this->mode = mode;
      this->color = color;

      // reset timer it something has changed to prevent weird blinking issues
      if (has_changed) {
        this->timer.reset();
      }
    }

    Mode getMode() {
      return mode;
    }

    Color24 getColor() {
      return color;
    }

    void setup(uint8_t pin) {
      this->pin = pin;
      pinMode(pin, OUTPUT);

      // init timer
      timer.setTimeOutTime(100);

      // start counting
      timer.reset();
    }

    void loop() {
      // Get config based on current mode
      const Config& config = configs[mode];

      // Compute new pin state
      unsigned long loopCount = timer.getLoopCount();
      bool pinHigh = false;
      switch (config.test) {
        case TEST_EQUALS:
          pinHigh = ((loopCount % config.modulo) == config.value);
          break;
        case TEST_GT:
          pinHigh = ((loopCount % config.modulo) > config.value);
          break;
        case TEST_LT:
          pinHigh = ((loopCount % config.modulo) < config.value);
          break;
        default:
          pinHigh = false;
      };

      // Update LED
      if (pinHigh) {
        rgbLedWrite(pin, color.r, color.g, color.b);
      } else {
        rgbLedWrite(pin, 0, 0, 0);
      }
    }

    static String toString(const RgbLedBlinker::Color24& color) {
      String s;
      s += '{';
      s += String(color.r);
      s += ',';
      s += String(color.g);
      s += ',';
      s += String(color.b);
      s += '}';
      return s;
    }

    static inline int getRandomColorIndex() {
      // Generate random value between 1 (inclusive) and colors_count (exclusive)
      // Exclude value 0 to exclude BLACK color
      int index = random(1 , colors_count); 
      return index;
    }

    static const RgbLedBlinker::Color24& getRandomColor() {
      int index = getRandomColorIndex();
      const RgbLedBlinker::Color24* color_ptr = RgbLedBlinker::colors[index];
      return *color_ptr;
    }

    const RgbLedBlinker::Color24& getNewRandomColor() {
      // Find actual selected color index.
      // Don't really care if the actual colors is not found.
      int actual_color_index = -1;
      for(int i=0; i<colors_count; i++) {
        if (this->color == *RgbLedBlinker::colors[i])
          actual_color_index = i;
      }

      int new_color_index = getRandomColorIndex();
      while (actual_color_index == new_color_index) {
        new_color_index = getRandomColorIndex();
      }
      const RgbLedBlinker::Color24* new_color_ptr = RgbLedBlinker::colors[new_color_index];
      return *new_color_ptr;
    }

};
