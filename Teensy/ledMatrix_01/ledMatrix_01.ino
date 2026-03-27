
//#define USE_ADAFRUIT_GFX_LAYERS
#include <MatrixHardware_Teensy4_ShieldV5.h>  
#include <SmartMatrix.h>

#define COLOR_DEPTH 24                  // Choose the color depth used for storing pixels in the layers: 24 or 48 (24 is good for most sketches - If the sketch uses type rgb24 directly, COLOR_DEPTH must be 24)
const uint16_t kMatrixWidth = 64;       // Set to the width of your display, must be a multiple of 8
const uint16_t kMatrixHeight = 32;      // Set to the height of your display
const uint8_t kRefreshDepth = 36;       // Tradeoff of color quality vs refresh rate, max brightness, and RAM usage.  36 is typically good, drop down to 24 if you need to.  On Teensy, multiples of 3, up to 48: 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48.  On ESP32: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save RAM, more to keep from dropping frames and automatically lowering refresh rate.  (This isn't used on ESP32, leave as default)
const uint8_t kPanelType = SM_PANELTYPE_HUB75_32ROW_MOD16SCAN;   // Choose the configuration that matches your panels.  See more details in MatrixCommonHub75.h and the docs: https://github.com/pixelmatix/SmartMatrix/wiki
const uint32_t kMatrixOptions = (SM_HUB75_OPTIONS_NONE);        // see docs for options: https://github.com/pixelmatix/SmartMatrix/wiki

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);

const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);
SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer1, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);


void setup() {
  Serial.begin(115200);

  matrix.addLayer(&scrollingLayer1); 

  matrix.begin();

  scrollingLayer1.setMode(wrapForward);

  scrollingLayer1.setColor({0xff, 0xff, 0xff});

  scrollingLayer1.setSpeed(10);

  scrollingLayer1.setFont(gohufont11);

  scrollingLayer1.setOffsetFromTop((kMatrixHeight/2) - 7);

  // Start with default names
  scrollingLayer1.start("unsorted.fest", -1);

  //Serial.println("Enter '1:YourName' to set Artist 1. Enter '2:YourName' to set Artist 2. Enter '3:YourTime' to set Time.");
}

void loop() {
  // Check if there is input from Serial

  delay(100); // Small delay to avoid overloading
}
