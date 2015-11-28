#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_DATA 6
#define ONE_WIRE_BUS 2

int colorMode; //0=solid, 1=temp, 2=rainbow
int pulse = 0; //0=no, 1=yes
int twinkle = 0; //0=no, 1=yes
int strobe = 0; //0=no, 1=yes

//Global params
int animationSpeed;
int maxBrightness;

//Color parmas
int r = 0;
int g = 0;
int b = 0;
int tempMin;
int tempMax;

//Control IDs
int colorModeID;
int solidButton;
int tempButton;
int rainbowButton;
int rotaryR;
int rotaryG;
int rotaryB;
int rotaryTempMin;
int rotaryTempMax;

//Temp Probe
int tempValue;
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

//Internal
bool pulseUp = true;
int brightness = 0;
int colorStep = 0;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino 0 number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(100 , NEOPIXEL_DATA, NEO_GRB + NEO_KHZ800);

uint32_t magenta = strip.Color(255, 0, 255);
uint32_t cyan = strip.Color(0, 255, 255);
uint32_t yellow = strip.Color(255, 255, 0);
uint32_t orange = strip.Color(255, 165, 0);
uint32_t green = strip.Color(0,255,0);
uint32_t blue = strip.Color(0,0,255);
uint32_t purple = strip.Color(165,0,255);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  //Initialize Temp Probe
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement

  //Initialize GUI
  gBegin(11613); 
}

void loop() {
  UpdateLights();

  UpdateTemp();

  //Update GUI
  guino_update();

  delay(animationSpeed);
}

void gInit()
{
  
  gAddLabel("CHROMA CONTROL",1);
  gAddSpacer(1);
  
  gAddSlider(0,255,"BRIGHTNESS",&maxBrightness);
  gAddSlider(0,100,"ANIMATION SPEED", &animationSpeed);

  gAddLabel("COLOR",1);
  gAddSpacer(1);
  char* curr;
  if(colorMode==0){
    curr = "CURRENT: SOLID";
  }else if(colorMode==1){
    curr = "CURRENT: TEMPERATURE";
  }else if(colorMode==2){
    curr = "CURRENT: RAINBOW";
  }
  colorModeID = gAddLabel(curr,1);
  solidButton = gAddButton("SOLID");
  tempButton = gAddButton("TEMPERATURE"); 
  rainbowButton = gAddButton("RAINBOW"); 

  gAddLabel("EFFECTS",1);
  gAddSpacer(1);
  gAddToggle("PULSE",&pulse);
  gAddToggle("TWINKLE (COMING SOON)",&twinkle);
  gAddToggle("STROBE (COMING SOON)",&strobe);

  gAddColumn();

  gAddLabel("TEMPERATURE (C)",1);
  gAddSpacer(1);
  gAddMovingGraph("CASE",20,60, &tempValue, 10);

  gAddLabel("PARAMETERS",1);
  gAddSpacer(1);
  gAddLabel("SOLID",1);
  rotaryR = gAddRotarySlider(0,255,"R",&r);
  rotaryG = gAddRotarySlider(0,255,"G",&g);
  rotaryB = gAddRotarySlider(0,255,"B",&b);
  gAddLabel("TEMPERATURE",1);
  rotaryTempMin = gAddRotarySlider(20,80,"Min",&tempMin);
  rotaryTempMax = gAddRotarySlider(20,80,"Max",&tempMax);
  
    gSetColor(255,165,0);
}

void gButtonPressed(int id)
{
  if(id == solidButton){
    colorMode = 0;
    for(uint8_t i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(r,g,b));
    } 
    gUpdateLabel(colorModeID, "CURRENT: SOLID");
  }
  if(id == tempButton){
    colorMode = 1;
    gUpdateLabel(colorModeID, "CURRENT: TEMPERATURE");
  }
  if(id == rainbowButton){
    colorMode = 2;
    gUpdateLabel(colorModeID, "CURRENT: RAINBOW");

  }
}

void gItemUpdated(int id)
{
  if(id == rotaryR || id == rotaryG || id == rotaryB){
    for(uint8_t i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(r,g,b));
    }
  }
}

void UpdateTemp(){
  sensors.requestTemperatures(); 
  tempValue = sensors.getTempCByIndex(0);
}


void UpdateLights(){
  if(pulse){
    pulseAll(); 
  }
  if(colorMode == 1){ //Temp mode
    tempCycle();
  } 
  if(colorMode == 2){ //Rainbow mode
    rainbowCycle();
  } 

  strip.setBrightness(brightness);
  strip.show();

}



void pulseAll(){
  if(pulseUp){
    brightness++;
    if(brightness >= maxBrightness) pulseUp = false; 
  }
  else{
    brightness--;
    if(brightness <= 0) pulseUp = true;
  }


}

//Set color between yellow to orange to red based on case temp
void tempCycle(){
int tempCode = 255 -  255 * ((tempValue - tempMin)/(tempMax - tempMin));
  
  for(uint8_t i=0; i< strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(255,tempCode,0));
  }
}


// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle() {

  for(uint8_t i=0; i< strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + colorStep) & 255));
  }
  colorStep = (colorStep+1)%256;
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } 
  else if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } 
  else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}


void resetAll(){
  for(uint8_t i=0;i<strip.numPixels();i++){
    strip.setPixelColor(i,0);
  }
}




