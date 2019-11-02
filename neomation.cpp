#include "neomation.h"

Neomation::Neomation(void)
{
  _brightness = 255;
  _color = 00000000000;
  _rcolor = 00000000000;

  for (int i=0;i<NUM_ANIM;i++) {
    onoff[i] = 0;
    time_start[i] = 0;
    time_length[i] = 0; 
    loop_num[i] = 0; 
    num_loops[i] = 0; 
    update_time[i] = 0;
  }
}

Neomation::Neomation(uint8_t pin, uint16_t num_leds, uint8_t max_brightness)
{
  
  _brightness = 255;
  _color = 00000000000;
  _rcolor = 00000000000;

  for (int i=0;i<NUM_ANIM;i++) {
    onoff[i] = 0;
    time_start[i] = 0;
    time_length[i] = 0; 
    loop_num[i] = 0; 
    num_loops[i] = 0; 
    update_time[i] = 0;
  }

//  Adafruit_NeoPixel* strip = Adafruit_NeoPixel(num_leds, pin, NEO_GRBW + NEO_KHZ800)
//  _pin = pin;
//  _num_leds = num_leds;
  setPin(pin);
  setNum(num_leds);
  setMaxBrightness(max_brightness);
  
  strip.begin();
  setStrip(); // turn off all leds

}

void Neomation::setPin(uint8_t pin)
{
  Serial.println("\n Setting Neomation object to pin... ");
  Serial.println(pin);
//  Adafruit_NeoPixel* strip = Adafruit_NeoPixel(_num_leds, pin, NEO_GRBW + NEO_KHZ800);
  strip.setPin(pin);
  strip.updateType(NEO_GRBW + NEO_KHZ800);
  _pin = pin;
}
void Neomation::setNum(uint16_t num_leds)
{
  Serial.println("\n Setting Neomation object with number of pixels... ");
  Serial.println(num_leds);
  
//  Adafruit_NeoPixel* strip = Adafruit_NeoPixel(num_leds, _pin, NEO_GRBW + NEO_KHZ800);
  strip.updateLength(num_leds);
  _fps = 1000000 / ((num_leds * 30) + 50); 
  _num_leds = num_leds;

  Serial.print("\n _fps calculated as ");
  Serial.println(_fps);
}
void Neomation::setMaxBrightness(uint8_t max_brightness)
{
  strip.setBrightness(max_brightness);
}

void Neomation::update(void)
{
  // put functions that need to be called every loop here
  _fade();
  _flash();
  _colorFade();
  _rainbow();
}


void Neomation::offAllEffects(void)
{
  if (onoff[fade_id]) offFade();
  if (onoff[flash_id]) offFlash();
  if (onoff[colorfade_id]) offColorFade();
  if (onoff[rainbow_id]) offRainbow();
}


uint32_t Neomation::color(uint8_t r,uint8_t g,uint8_t b,uint8_t w,uint8_t brightness)
{
  r = map(r, 0, 255, 0, brightness);
  g = map(g, 0, 255, 0, brightness);
  b = map(b, 0, 255, 0, brightness);
  w = map(w, 0, 255, 0, brightness);
  return strip.Color(r,g,b,w);
}

void Neomation::setBrightness(uint8_t brightness)
{
  _brightness = brightness;
}

void Neomation::setStrip(uint32_t color, uint16_t led_array[], bool gamma, bool strip_show)
{
  // color is 32bit value that is created from color = strip.Color(r, g, b, w);
  // gamma is 1 or 0. 1 will gamma correct so that light intesisty increment is based on human perceived intensity. 0 gamma correct is off. Defaults to ON
  // ledmatrix is a list of LEDs to be set the given color. Defaults to all LEDs
  // ledMatrixSize is the size of the aforementioned list

  Serial.println("\nThis is setStrip()... ");
  uint8_t a[4];
  uint8_t b[4];
  
  if (gamma) {
    Serial.println("Gamma correction is on... ");
    
    Serial.println("Parsing colors for correction... ");
    a[3] = color >> 24;
    Serial.println(a[3]);
    a[0] = color >> 16;
    Serial.println(a[2]);
    a[1] = color >>  8;
    Serial.println(a[1]);
    a[2] = color;
    Serial.println(a[0]);

    Serial.println("Corrected colors are... ");
    b[0] = gammaconvert[a[0]];
    Serial.println(b[0]);
    b[1] = gammaconvert[a[1]];
    Serial.println(b[1]);
    b[2] = gammaconvert[a[2]];
    Serial.println(b[2]);
    b[3] = gammaconvert[a[3]];
    Serial.println(b[3]);

    color = ((uint32_t)b[3] << 24) | ((uint32_t)b[0] << 16) | ((uint32_t)b[1] << 8) | b[2];
  }

//  uint16_t arrsz = sizeof(led_array) / sizeof(uint16_t);
//  if (arrsz > 1) {
//    Serial.println("led_array was defined... ");
//    
//    for(int i=0; i<arrsz; i++) {
//      strip.setPixelColor(led_array[i], color);
//    }
//  } 
  else {
    Serial.println("led_array was NOT defined, setting color for entire strip... ");

    for(int i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, color);
    }
  }
  if (strip_show) {
      strip.show(); 
  }
  _color = color;
}


void Neomation::setFade(uint32_t color, uint16_t led_array[], uint16_t length)
{
  _rcolor = _color;
  time_length[flash_id] = length;
//  _copyArray(led_array,fade_array);

  _calculateFadeParameters(_color,color,length);
  
  onoff[fade_id] = 1;
  time_start[fade_id] = millis();
  update_time[fade_id] = millis();
}

void Neomation::offFade(void)
{
  onoff[fade_id] = 0;
  
  Serial.println("\nStopping the fading sequence... ");

  uint8_t a[4];
  a[3] = _color >> 24;
  a[0] = _color >> 16;
  a[1] = _color >>  8;
  a[2] = _color;

  Serial.println("fade error(set - intended) due to truncation from integer math is... ");
  Serial.print("Red error is ");
  Serial.println(redVal - a[0]);
  Serial.print("Blue Error is ");
  Serial.println(grnVal - a[1]);
  Serial.print("Green Error is ");
  Serial.println(bluVal - a[2]);
  Serial.print("White Error is ");
  Serial.println(whtVal - a[3]);

  if (loop_num[fade_id] >= fade_steps) {
  Serial.print("Manually setting final intended values ");
  redVal = a[0];
  grnVal = a[1];
  bluVal = a[2];
  whtVal = a[3];
  
  // integer math errors may have caused fading to miss target, so set final color now
  setStrip(_rcolor,fade_array);
  }
  
}

void Neomation::_fade(void)
{
  if (onoff[fade_id]) {
    Serial.println("\nThis is fade()");
    if (millis() - update_time[fade_id] > fade_dly) {
      if (loop_num[fade_id] <= fade_steps) {
        Serial.println("calculating new RGB values and updating LEDs");
        update_time[fade_id] = millis();
  
        redVal = _calculateVal(stepR, redVal, loop_num[fade_id]);
        grnVal = _calculateVal(stepG, grnVal, loop_num[fade_id]);
        bluVal = _calculateVal(stepB, bluVal, loop_num[fade_id]);
        whtVal = _calculateVal(stepW, whtVal, loop_num[fade_id]);
  
        setStrip(strip.Color(redVal, grnVal, bluVal, whtVal),fade_array); // Write current values to LED pins and strip.show
  
        Serial.print("Loop count: ");
        Serial.println(loop_num[fade_id]);
        loop_num[fade_id]++;
      }
      else {
        offFade();
      }
    }
  }
}



void Neomation::setFlash(uint32_t color1, uint32_t color2, uint16_t led_array[], uint16_t speed, uint16_t num_flash, uint16_t length)
{
  _rcolor = _color;
  flash_color_1 = color1;
  flash_color_2 = color2;
  flash_speed = speed;
  num_loops[flash_id] = num_flash;
  time_length[flash_id] = length;
//  _copyArray(led_array,flash_array);
  
  Serial.println("Initializing the flashing sequence... ");
  onoff[flash_id] = 1;
  time_start[flash_id] = millis();
  update_time[flash_id] = millis();
  flash_state = 0;
  loop_num[flash_id] = 0;

}


void Neomation::offFlash(void)
{
  onoff[flash_id] = 0;
  setStrip(_rcolor,flash_array);
  Serial.println("\nStopping the flashing sequence... ");
}


void Neomation::_flash(void)
{
  if (onoff[flash_id]) {
    Serial.println("\nThis is flash()");

    if ((millis() -  time_start[flash_id]) < time_length[flash_id]  |  loop_num[flash_id] < num_loops[flash_id] | (num_loops[flash_id]==0 & time_length[flash_id]==0)) {
      if ((millis() - update_time[flash_id]) > flash_speed) {
        flash_state = !flash_state;
        if(flash_state) {
          Serial.println("Turning LEDs ON... ");
          setStrip(flash_color_1,flash_array);
        }
        else {
          Serial.println("Turning LEDs OFF... ");
          setStrip(flash_color_2,flash_array);
          loop_num[flash_id] += loop_num[flash_id];
        }
        update_time[flash_id] = millis();
      }
    }
    else {
      offFlash();
    }
  }
}



void Neomation::setColorFade(uint16_t led_array[], uint16_t length, uint16_t speed)
{
  onoff[colorfade_id] = 1;
//  _copyArray(led_array,colorfade_array);
  time_length[colorfade_id] = length;
  colorfade_speed = speed;

  time_start[colorfade_id] = millis();
  current_color = 0;
}


void Neomation::offColorFade(void)
{
  onoff[colorfade_id] = 0;
  offFade();
}
  
void Neomation::_colorFade(void) // use fade() and cycle through some colors
{
  if (onoff[colorfade_id]) {
    if (!onoff[fade_id]) { // check to see if fade is done with last color fade yet
      if (millis() - time_start[colorfade_id] < time_length[colorfade_id] | time_length[colorfade_id] == 0) {
        Serial.println("Changing the color that is being faded to...");
        uint8_t a[4];
        a[0] = map(colors[current_color][0], 0, 255, 0, _brightness);
        a[1] = map(colors[current_color][1], 0, 255, 0, _brightness);
        a[2] = map(colors[current_color][2], 0, 255, 0, _brightness);
        a[3] = map(colors[current_color][3], 0, 255, 0, _brightness);
        current_color = (current_color + 1) % num_colors;
        uint32_t color = ((uint32_t)a[3] << 24) | ((uint32_t)a[0] << 16) | ((uint32_t)a[1] << 8) | a[2];
        setFade(color, colorfade_array, colorfade_speed);
      }
      else offColorFade();
    }
  }
}



void Neomation::setRainbow(uint16_t led_array[], uint16_t length, uint16_t rainbow_speed, uint16_t white_length, uint16_t white_speed)
{
  _rcolor = _color;
  Serial.println("Initializing the rainbow animation... ");
  
  rainbow_head = white_length - 1;
  rainbow_tail = 0;
  rainbow_j = 0;
  white_time = 0;
  rainbow_dly = 0;
  white_dly = 0;
  rainbow_multiplier = 1;
  white_multiplier = 1;
  
  while (rainbow_dly <= 0) {
    rainbow_dly = (rainbow_speed*rainbow_multiplier)/_num_leds - 1000/_fps; // strip.numPixels() is the number of animation states that ideally would be displayed 
    rainbow_multiplier++;
  }
  rainbow_multiplier -= 1;

  while (white_dly <= 0) {
    white_dly = (white_speed*white_multiplier)/_num_leds - 1000/_fps; // strip.numPixels() is the number of animation states that ideally would be displayed 
    white_multiplier++;
  }
  white_multiplier -= 1;

//  _copyArray(led_array,rainbow_array);
  time_length[rainbow_id] = length;
  onoff[rainbow_id] = 1;
  update_time[rainbow_id] = 0;
  time_start[rainbow_id] = millis();
  update_time[rainbow_id] = millis();
  loop_num[rainbow_id] = 0;

}


void Neomation::offRainbow(void)
{
  onoff[rainbow_id] = 0;
  setStrip(_rcolor,flash_array);
  Serial.println("\nStopping the rainbow sequence... ");
}




void Neomation::_rainbow(void)
{
  if (onoff[rainbow_id]) {
    if (millis() - time_start[rainbow_id] < time_length[rainbow_id] | time_length[rainbow_id] == 0) {
      if (millis() - update_time[rainbow_id] > rainbow_dly) {
        
        for(uint16_t i=0; i<strip.numPixels(); i++) {
          if((i >= rainbow_tail && i <= rainbow_head) || (rainbow_tail > rainbow_head && i >= rainbow_tail) || (rainbow_tail > rainbow_head && i <= rainbow_head) ){
            strip.setPixelColor(i, strip.Color(0,0,0, map(255, 0, 255, 0, _brightness) ) );
          }
          else{
            strip.setPixelColor(i, _wheel(((i * 256 / strip.numPixels()) + rainbow_j) & 255));
            strip.setPixelColor(i, _wheel(((i * 256 / strip.numPixels()) + rainbow_j) & 255));
          }
        }
    
        if(millis() - white_time > white_dly) {
          rainbow_head += white_multiplier;
          rainbow_tail += white_multiplier;
          white_time = millis();
        }
    
        rainbow_j += rainbow_multiplier;
        rainbow_j%=256;
        rainbow_head%=strip.numPixels();
        rainbow_tail%=strip.numPixels();
    
    //    // if array is active then only show those LEDs
    //    if (array_state) {
    //      // loop through all the LEDs in the strip
    //      for(uint16_t i=0; i<strip.numPixels(); i++) {
    //        // for each loop check if that LED is part of the array
    //        for(uint16_t j=0; i<led_array_size; j++) {
    //          // if it is not then turn it off
    //          if (i != atoi(led_array[j])) {
    //            strip.setPixelColor(i, strip.Color(0,0,0,0) );
    //          }
    //        }
    //      }
    //    }
        strip.show();
    
        update_time[rainbow_id] = millis();
      }
    }
    else offRainbow();
  }
}








// From https://www.arduino.cc/en/Tutorial/ColorCrossfader
/* BELOW THIS LINE IS THE MATH -- YOU SHOULDN'T NEED TO CHANGE THIS FOR THE BASICS
*
* The program works like this:
* Imagine a crossfade that moves the red LED from 0-10,
*   the green from 0-5, and the blue from 10 to 7, in
*   ten steps.
*   We'd want to count the 10 steps and increase or
*   decrease color values in evenly stepped increments.
*   Imagine a + indicates raising a value by 1, and a -
*   equals lowering it. Our 10 step fade would look like:
*
*   1 2 3 4 5 6 7 8 9 10
* R + + + + + + + + + +
* G   +   +   +   +   +
* B     -     -     -
*
* The red rises from 0 to 10 in ten steps, the green from
* 0-5 in 5 steps, and the blue falls from 10 to 7 in three steps.
*
* In the real program, the color percentages are converted to
* 0-255 values, and there are 1020 steps (255*4).
*
*
*/
  // Find the led_multiplier needed to achieve the desired fade time based on the FPS (# of LEDs) - brute force method
  // FPS is a global variable that is defined by the number of LEDs in the strip (for data wire only strips like the SK6812)
  // This is only an approximation of the dly_time needed because integer math has large truncation error
  // dly_time is the delay between animation states
  // led multiplier determines how many animations states are skipped (2 = every second state is displayed, 3 = every third state is displayed, etc)
  // <animation_millis> - how long you animation should last in ms
  // <animation_states> - the number of individual animation states that you want to animate. (For fading this could be the difference between the current LED brightness (0-255) and the new LED brightness)
  // <tune> - increase <tune> to further speed up animation. this is helpful if you have other code slowing down your animations
  // I forget how I derived this equation, but it is based on theoretical time to updated SK6812 strip based on number of LEDs
  // transition is in milliseconds
void Neomation::_calculateFadeParameters(uint32_t color_start, uint32_t color_end, uint32_t transition)
{
  
  int diffR = (uint8_t)(color_end >> 16) - (uint8_t)(color_start >> 16);
  int diffG = (uint8_t)(color_end >> 8) - (uint8_t)(color_start >> 8);
  int diffB = (uint8_t)(color_end) - (uint8_t)(color_start);
  int diffW = (uint8_t)(color_end >> 24) - (uint8_t)(color_start >> 24);
  Serial.println("\nThis is calculateStepSize() ");
  Serial.print("diffR is ");
  Serial.println(diffR);
  Serial.print("diffG is ");
  Serial.println(diffG);
  Serial.print("diffB is ");
  Serial.println(diffB);
  Serial.print("diffW is ");
  Serial.println(diffW);
  Serial.print("transition is ");
  Serial.println(transition);
  
  // find number of animation states for fading
  fade_steps = _max(abs(diffR),abs(diffG));
  fade_steps = _max(fade_steps,abs(diffB));
  fade_steps = _max(fade_steps,abs(diffW));
  Serial.print("fade_steps calculated as ");
  Serial.println(fade_steps);
  
  if (fade_steps == 0) {
    stepR = 0;
    stepG = 0;
    stepB = 0;
    stepW = 0;
    Serial.println("fade_steps was zero so returning step size of zero ");
    return;
  }
  else if (_fps == 0) Serial.println("_fps = 0, something wwrong here ");
  
  
  fade_dly = 0;
  fade_multiplier = 0;
  
  while (fade_dly <= 0 && fade_multiplier < 255) {
    fade_multiplier += 1;
    // dly_time = (<animation_millis>*led_multiplier)/<animation_states> - 1000/FPS - <tune>; 
    fade_dly = (transition*fade_multiplier)/fade_steps - 1000/_fps;

    if (fade_dly > 10 && fade_steps < 1) { 
      // we have more than enough delay time, so lets increase the number of steps
      // will make fading smoother
      fade_steps += 1;
      // restart from beginning
      fade_dly = 0;
      fade_multiplier -= 1;
    }
  }
  Serial.print("fade_dly is ");
  Serial.println(fade_dly);
  Serial.print("fade_multiplier is ");
  Serial.println(fade_multiplier);
  Serial.print("fade_steps after fade_dly/fade_multiplier calculation is ");
  Serial.println(fade_steps);

/*
* To figure out how big a step there should be between one up- or
* down-tick of one of the LED values,we
* calculate the absolute gap between the start and end values divided by the multiplier,
* and then divides that gap into fade_steps to determine the size of the step
* between adjustments in the value.
*/

  stepR = 0;
  stepG = 0;
  stepB = 0;
  stepW = 0;
  if (diffR) {
    stepR = fade_steps / diffR;
  }
  if (diffG) {
    stepG = fade_steps / diffG;
  }
  if (diffB) {
    stepB = fade_steps / diffB;
  }
  if (diffW) {
    stepW = fade_steps / diffW;
  }

  Serial.print("stepR is ");
  Serial.println(stepR);
  Serial.print("stepG is ");
  Serial.println(stepG);
  Serial.print("stepB is ");
  Serial.println(stepB);
  Serial.print("stepW is ");
  Serial.println(stepW);
  

    // fade_steps will now be calculated to represent the number of loops needed to complete the animation
  fade_steps = fade_steps/fade_multiplier; // if fade_multiplier = 2 then half the number of steps to be taken, etc.
  Serial.print("fade_steps after fade_multiplier correction is ");
  Serial.println(fade_steps);
}




/* The next function is calculateVal. When the loop value, i,
*  reaches the step size appropriate for one of the
*  colors, it increases or decreases the value of that color by 1.
*  (R, G, and B are each calculated separately.)
*/
int Neomation::_calculateVal(int step_size, int val, int i)
{
  if ((step_size) && i % step_size == 0) { // If step is non-zero and its time to change a value,
      if (step_size > 0) {              //   increment the value if step is positive...
          val += fade_multiplier;
      }
      else if (step_size < 0) {         //   ...or decrement it if step is negative
          val -= fade_multiplier;
      }
  }

  // Defensive driving: make sure val stays in the range 0-255
  if (val > 255) {
      val = 255;
  }
  else if (val < 0) {
      val = 0;
  }

  return val;
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Neomation::_wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(map(255 - WheelPos * 3, 0, 255, 0, _brightness), 0, map(WheelPos * 3, 0, 255, 0, _brightness),0);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, map(WheelPos * 3, 0, 255, 0, _brightness), map(255 - WheelPos * 3, 0, 255, 0, _brightness),0);
  }
  WheelPos -= 170;
  return strip.Color(map(WheelPos * 3, 0, 255, 0, _brightness), map(255 - WheelPos * 3, 0, 255, 0, _brightness), 0,0);
}



//void Neomation::_copyArray(uint16_t input[], uint16_t* output[]) {
//  for (int i=0; i<sizeof(input) / sizeof(uint16_t); i++) {
//    output[i] = input[i];
//  }
//}
