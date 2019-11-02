#ifndef neomation_h
#define neomation_h

#include "Arduino.h"
#include <Adafruit_NeoPixel.h>

/*
 * NeoPixel Animation Library
 *
 * Supports:
 * RGBW LEDs (SK6812/WS2812/Neopixel) with Adafruit Library
 *
 * See https://github.com/garrettdowd/mqtt-digital-rgbw-lights for more information.
 */

 /* TO DO
  *  
  *  Clean up global variables
  *  
  *  Enable additive animations (animation will modify a global state variable, which will then be published)
  * 
  * make code compatible with analog RGBW, RGB, and single color strips
  * use #if to decide which code to compile
  * 
  * fading bugs
  * - fade to array then switch to other array with fade causes some LEDs to be left on (fading from one array to the next not supported)
  * - sometimes color mix is not correct. white will be too dominant and rgb values are not set correctly
  * - reduce fading error
  * 
  * is colorTemp working correctly?
  * 
  * Implement Color temperature with RGB
  * 
  * 
  * MUSIC EFFECT
  * Need to broadcast/multicast message to any clients listening (currently setup to only send to one client)
  * How do I take different strip lengths into account??? - maybe unicast is still best choice 
  * https://stackoverflow.com/questions/1735952/for-udp-broadcast-gurus-problems-achieving-high-bandwidth-audio-udp-broadcast-o
  * https://superuser.com/questions/695813/wifi-udp-unicast-vs-multicast-speed
  * 
  * Is websocketsserver and hash needed?
  */

#define NUM_ANIM     6

class Neomation
{
  public:

    Neomation();
    Neomation(uint8_t pin, uint16_t num_leds, uint8_t max_brightness = 255);
    void setPin(uint8_t pin);
    void setNum(uint16_t num_leds);
    void setMaxBrightness(uint8_t max_brightness);

    void update(void); // to update animations, call at beginning of loop

    uint32_t color(uint8_t r,uint8_t g,uint8_t b,uint8_t w = 0,uint8_t brightness = 255);
    void setBrightness(uint8_t brightness = 255);
    void setStrip(uint32_t color = 000000000000, uint16_t led_array[] = {0}, bool gamma = 0, bool strip_show = 1);
    
    void setFade(uint32_t color = 000000000000, uint16_t led_array[] = {0}, uint16_t length = 1000);
    void offFade(void);
    
    void setFlash(uint32_t color1 = 255000000000, uint32_t = 000000000000, uint16_t led_array[] = {0}, uint16_t speed = 500, uint16_t num = 0, uint16_t length = 0);
    void offFlash(void);
    
    void setColorFade(uint16_t led_array[] = {0}, uint16_t length = 0, uint16_t speed = 10000);
    void offColorFade(void);  
    
    void setRainbow(uint16_t led_array[] = {0}, uint16_t length = 0, uint16_t rainbow_speed = 10000, uint16_t white_length = 0, uint16_t white_speed = 5000);
    void offRainbow(void);
    void setArray(uint8_t led_array[] = {0});
 
    void offAllEffects(void);

//    void setUDP(uint16_t port);



    
  private:
    Adafruit_NeoPixel strip;
//    WiFiUDP port; // for receving music animation frames
    uint8_t _pin;
    uint16_t _local_port;
    uint16_t _num_leds;
    int _fps;
    
    uint32_t _color; // always the actual state of the LED
    uint32_t _rcolor; // the color that the leds should return to after animation
//    uint8_t _stripR[];
//    uint8_t _stripG[];
//    uint8_t _stripB[];
//    uint8_t _stripW[];

    uint8_t _brightness;

//    uint16_t _led_array[];
    
    // Animation Variables

    bool onoff[NUM_ANIM];  // keep track of whether an animation should be displayed or not
    uint32_t time_start[NUM_ANIM];
    uint32_t time_length[NUM_ANIM]; // to keep track of animations that should be turned off after a set amount of time
    uint16_t loop_num[NUM_ANIM]; // current loop number
    uint16_t num_loops[NUM_ANIM]; // to keep track of how many times an animation should loop
    uint32_t update_time[NUM_ANIM]; // to keep track of the time when animations were last updated



    void _fade(void); // #0
      const uint8_t fade_id = 0;
      uint16_t fade_array[];
      // these are bytes because LED brightness is reprented with 8 bits (0-255), may need to change if higher bit depths are used
      uint8_t redVal, grnVal, bluVal, whtVal; // temporary values of LEDs while fading, used to calculate step sizes, so ensure that these are updated to current values once any fading or animation is finished
      int fade_multiplier, fade_steps, fade_dly; // (0-255)
      int stepR, stepG, stepB, stepW; // needs to hold value (-255 to 255)

    
    void _flash(void); // #1
      const uint8_t flash_id = 1;
      uint16_t flash_array[];
      bool flash_state;
      uint16_t flash_speed;
      uint32_t flash_color_1;
      uint32_t flash_color_2;

    
    void _bootUp(void); // #2
      const uint8_t bootup_id = 2;


    
    void _colorFade(void); // #3
      const uint8_t colorfade_id = 3;
      uint16_t colorfade_array[];
      uint8_t current_color;
      const uint8_t num_colors = 7;
      // {red, grn, blu, wht}
      const byte colors[7][4] = {
        {255, 0, 0, 0},
        {0, 255, 0, 0},
        {0, 0, 255, 0},
        {255, 80, 0, 0},
        {163, 0, 255, 0},
        {0, 255, 255, 0},
        {255, 255, 0, 0}
      };
      uint16_t colorfade_speed;


    void _rainbow(void); // #4
      const uint8_t rainbow_id = 4;
      uint16_t rainbow_array[];
      int rainbow_head, rainbow_tail, rainbow_j;
      int rainbow_multiplier, white_multiplier;
      int rainbow_dly, white_dly;
      uint32_t white_time;

      
    void _stream(void); // #5
      const uint8_t stream_id = 5;

    void _calculateFadeParameters(uint32_t color1, uint32_t color2, uint32_t transition);
    int _calculateVal(int step_size, int val, int i);
//    void _copyArray(const int* input, int input_size, int* output);
//    int _colorTemp(int kelvin, int rgb, bool white = 0);
    uint32_t _wheel(byte WheelPos);

    const uint8_t PROGMEM gammaconvert[256] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,
        2,  2,  2,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
        5,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9, 10, 10, 10,
       11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17,
       18, 18, 19, 19, 20, 20, 21, 21, 22, 23, 23, 24, 24, 25, 26, 26,
       27, 28, 28, 29, 30, 30, 31, 32, 32, 33, 34, 35, 35, 36, 37, 38,
       38, 39, 40, 41, 42, 42, 43, 44, 45, 46, 47, 48, 49, 49, 50, 51,
       52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
       69, 70, 71, 72, 73, 74, 75, 76, 78, 79, 80, 81, 82, 84, 85, 86,
       87, 89, 90, 91, 92, 94, 95, 96, 98, 99,100,102,103,104,106,107,
      109,110,112,113,114,116,117,119,120,122,123,125,126,128,130,131,
      133,134,136,138,139,141,143,144,146,148,149,151,153,154,156,158,
      160,161,163,165,167,169,170,172,174,176,178,180,182,183,185,187,
      189,191,193,195,197,199,201,203,205,207,209,211,213,215,218,220,
      222,224,226,228,230,233,235,237,239,241,244,246,248,250,253,255 };

};

#endif
