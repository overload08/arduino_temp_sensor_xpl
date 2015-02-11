/*
This file is part of B{Domogik} project (U{http://www.domogik.org}).
 
 License
 =======
 
 B{Domogik} is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 B{Domogik} is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with Domogik. If not, see U{http://www.gnu.org/licenses}.
 
 Plugin purpose
 ==============
 
 This file is part of 'ar_rgb' hardware plugin
 
 Control RGB led strip with an arduino and xPL protocol
 
 This arduino program may be used without Domogik with any xPL project
 
 @author: Fritz <fritz.smh@gmail.com>, OverLoad <y.poilvert@geekinfo.fr>
 @copyright: (C) 2007-2014 Domogik project
 @license: GPL(v3)
 @organization: Domogik
 */

/***********************************************
 * setColorFromRGBCode
 * transform a #rrggbb to red, green and blue values and call setColorFromRGB
 * Input : color in #rrggbb format (char)
 * Output : n/a
 ***********************************************/
void setColorFromRGBCode(char color[8]) {
  int red;
  int green;
  int blue;
  Serial.print(F("Set color to RGB code : "));
  Serial.println(color);
  // we translate each value in decimal
  // remember that format is #rrggbb with rrggbb hexadecimal data
  red = getDecimal(color[1]) * 16 + getDecimal(color[2]);
  green = getDecimal(color[3]) * 16 + getDecimal(color[4]);
  blue = getDecimal(color[5]) * 16 + getDecimal(color[6]);
  setColorFromRGB(red, green, blue);
  //setPotarColor(red, green, blue);
}

/***********************************************
 * setColorOff
 * Set color to #000000
 * Input : n/a
 * Output : n/a
 ***********************************************/
void setColorOff() {
  Serial.println(F("Set color to OFF"));
  setColorFromRGB(0, 0, 0);
  //setPotarColor(standbyColor[0], standbyColor[1], standbyColor[2]);
  isOff = true;
}


/***********************************************
 * setColorOn
 * Set color to saved color
 * Input : n/a
 * Output : n/a
 ***********************************************/
void setColorOn() {
  Serial.println(F("Set color to ON"));
  if ((currentColor[0] == 0) && (currentColor[1] == 0) && (currentColor[2] == 0)) {
    Serial.println(F("Set default color for ON : #ffffff"));
    setColorFromRGB(255, 255, 255);
    //setPotarColor(255, 255, 255);
  }
  else {
    // Create color code from values
    char color[8];
    sprintf(color, "#%02x%02x%02x", currentColor[0], currentColor[1], currentColor[2]);
    Serial.print(F("Set default color for ON : "));
    Serial.println(color);
    setColorFromRGB(currentColor[0], currentColor[1], currentColor[2]);
    //setPotarColor(currentColor[0], currentColor[1], currentColor[2]);
  }
}


/***********************************************
 * setColorFromRGB
 * Set color to PWM outputs
 * Input : red (int), green (int), blue (int)
 * Output : n/a
 ***********************************************/
void setColorFromRGB(int redValue, int greenValue, int blueValue) {
  // Create color code from values
  char color[8];
  char colorOld[8];
  sprintf(color, "#%02x%02x%02x", redValue, greenValue, blueValue);
  sprintf(colorOld, "#%02x%02x%02x", currentColor[0], currentColor[1], currentColor[2]);
  sendXplTrigForSetColor(color);
  if((redValue != currentColor[0]) || (greenValue != currentColor[1]) || (blueValue != currentColor[2]) || isOff == true) {
    Serial.print(F("Set color to : "));
    Serial.print(color);
    Serial.print(F(" from : "));
    Serial.println(colorOld);

    // Set led colors on PWM output
    // red, green, blue : 0..255  
    crossFade(redValue, greenValue, blueValue);

    Serial.println(F("Color set!"));
    // Save color only if different from #000000 (if not an OFF command)
    if (!((redValue == 0) && (greenValue == 0 ) && (blueValue == 0))) {
      currentColor[0] = redValue;
      currentColor[1] = greenValue;
      currentColor[2] = blueValue;
    }

    isOff = false;
    // Wait a little in order not to spam xPL network (for manual control)
    delay(100);
    return;   
  }
}

void setColorFromRGBnofade(int redValue, int greenValue, int blueValue) {
  // Create color code from values
  char color[8];
  sprintf(color, "#%02x%02x%02x", redValue, greenValue, blueValue);
  Serial.print("Set color to : ");
  Serial.println(color);

  // Set led colors on PWM output
  // red, green, blue : 0..255
  analogWrite(RED_PIN, redValue);
  analogWrite(GREEN_PIN, greenValue);
  analogWrite(BLUE_PIN, blueValue);
  Serial.println("Color set!");
  // Save color only if different from #000000 (if not an OFF command)
  if (!((redValue == 0) && (greenValue == 0 ) && (blueValue == 0))) {
    currentColor[0] = redValue;
    currentColor[1] = greenValue;
    currentColor[2] = blueValue;
  }
  sendXplTrigForSetColor(color);
  isOff = false;
  // Wait a little in order not to spam xPL network (for manual control)
  //    delay(100);
  return;
}
/***********************************************
 * setColorFromHSL
 * set color from Hue, Saturation and Luminosity
 * Input : hue, saturation, luminosity (0...1)
 * Output : n/a
 ***********************************************/
void setColorFromHSL(float H, float S, float L) {
  // HSL : 0..1
  // RGB : 0..255
  float R;
  float G;
  float B;
  float var1;
  float var2;
  Serial.print("Set color from HSL : H=");
  Serial.print(H);
  Serial.print(", S=");
  Serial.print(S);
  Serial.print(", L=");
  Serial.println(L);

  if (S == 0.0) {
    R = L*255.0;
    G = L*255.0;
    B = L*255.0;
  }
  else { 
    if (L < 0.5) {
      var2 = L*(1.0+S);
    }
    else {
      var2 = (L+S)-(S*L);
    }
    var1 = 2.0*L-var2;

    R = 255*hue2RGB(var1, var2, H+(1.0/3.0));
    G = 255*hue2RGB(var1, var2, H);
    B = 255*hue2RGB(var1, var2, H-(1.0/3.0));
  }
  setColorFromRGBnofade((int) R, (int) G, (int) B);
  return;
}

/***********************************************
 * hue2RGB
 * Help for setColorFromHSL function
 * Input : var1, var2, hue
 * Output : red, green or blue value
 ***********************************************/
float hue2RGB(float v1, float v2, float vH) {
  if (vH < 0.0) vH += 1.0;
  if (vH > 1.0) vH -= 1.0;
  if ((6.0*vH ) < 1.0) return (v1+(v2-v1)*6.0*vH);
  if ((2.0*vH ) < 1.0) return (v2);
  if ((3.0*vH ) < 2.0) return (v1+(v2-v1)*((2.0/3.0)-vH)*6.0);
  return v1;
}


/***********************************************
 * getDecimal
 * Get decimal value from a hexadecimal char
 * Input : hex value (char) 
 * Output : decimal (int)
 ***********************************************/
int getDecimal(char value) {
  int decimal = 0;
  if ((value >= 'a') && (value <= 'f')) {
    decimal = value - 'a' + 10;
  }
  if ((value >= 'A') && (value <= 'F')) {
    decimal = value - 'A' + 10;
  }
  if ((value >= '0') && (value <= '9')) {
    decimal = value - '0';
  }
  return decimal;
}

int calculateStep(int prevValue, int endValue) {
  int step = endValue - prevValue; // What's the overall gap?
  if (step) {                      // If its non-zero, 
    step = 1020/step;              //   divide by 1020
  } 
  return step;
}

int calculateVal(int step, int val, int i) {

  if ((step) && i % step == 0) { // If step is non-zero and its time to change a value,
    if (step > 0) {              //   increment the value if step is positive...
      val += 1;           
    } 
    else if (step < 0) {         //   ...or decrement it if step is negative
      val -= 1;
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

void crossFade(int R, int G, int B) {

  int stepR = calculateStep(currentColor[0], R);
  int stepG = calculateStep(currentColor[1], G); 
  int stepB = calculateStep(currentColor[2], B);

  for (int i = 0; i <= 1020; i++) {
    currentColor[0] = calculateVal(stepR, currentColor[0], i);
    currentColor[1] = calculateVal(stepG, currentColor[1], i);
    currentColor[2] = calculateVal(stepB, currentColor[2], i);

    analogWrite(RED_PIN, currentColor[0]);   // Write current values to LED pins
    analogWrite(GREEN_PIN, currentColor[1]);      
    analogWrite(BLUE_PIN, currentColor[2]); 

    delay(1); // Pause for 'wait' milliseconds before resuming the loop
    //if(Udp.parsePacket()){
    //  break;
    //}
  }
  // Update current values for next loop
  //currentColor[0] = redVal; 
  //currentColor[1] = grnVal; 
  //currentColor[2] = bluVal;
  //delay(10); // Pause for optional 'wait' milliseconds before resuming the loop
}

void HSBToRGB(
unsigned int inHue, unsigned int inSaturation, unsigned int inBrightness,
unsigned int *oR, unsigned int *oG, unsigned int *oB )
{
  if (inSaturation == 0)
  {
    // achromatic (grey)
    *oR = *oG = *oB = inBrightness;
  }
  else
  {
    unsigned int scaledHue = (inHue * 6);
    unsigned int sector = scaledHue >> 8; // sector 0 to 5 around the color wheel
    unsigned int offsetInSector = scaledHue - (sector << 8);	// position within the sector         
    unsigned int p = (inBrightness * ( 255 - inSaturation )) >> 8;
    unsigned int q = (inBrightness * ( 255 - ((inSaturation * offsetInSector) >> 8) )) >> 8;
    unsigned int t = (inBrightness * ( 255 - ((inSaturation * ( 255 - offsetInSector )) >> 8) )) >> 8;

    switch( sector ) {
    case 0:
      *oR = inBrightness;
      *oG = t;
      *oB = p;
      break;
    case 1:
      *oR = q;
      *oG = inBrightness;
      *oB = p;
      break;
    case 2:
      *oR = p;
      *oG = inBrightness;
      *oB = t;
      break;
    case 3:
      *oR = p;
      *oG = q;
      *oB = inBrightness;
      break;
    case 4:
      *oR = t;
      *oG = p;
      *oB = inBrightness;
      break;
    default:    // case 5:
      *oR = inBrightness;
      *oG = p;
      *oB = q;
      break;
    }
  }
}

