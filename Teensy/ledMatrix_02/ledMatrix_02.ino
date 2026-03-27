#define USE_ADAFRUIT_GFX_LAYERS
#include <MatrixHardware_Teensy4_ShieldV5.h>  
#include <SmartMatrix.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMono12pt7b.h>

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

static const uint32_t UART_BAUD = 115200;
static String incomingLine = "";

void applySpeedFromCommand(const String& message) {
  const String prefix = "CFG:SPEED:";
  if (!message.startsWith(prefix)) {
    return;
  }

  String speedPart = message.substring(prefix.length());
  speedPart.trim();
  int speed = speedPart.toInt();
  speed = constrain(speed, 0, 30);
  scrollingLayer1.setSpeed(speed);
  Serial.print("Set speed: ");
  Serial.println(speed);
}

int hexToNibble(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (c >= 'A' && c <= 'F') {
    return 10 + (c - 'A');
  }
  if (c >= 'a' && c <= 'f') {
    return 10 + (c - 'a');
  }
  return -1;
}

bool parseHexByte(const String& value, int startIndex, uint8_t& output) {
  if (startIndex < 0 || startIndex + 1 >= value.length()) {
    return false;
  }
  char high = value.charAt(startIndex);
  char low = value.charAt(startIndex + 1);
  int highVal = hexToNibble(high);
  int lowVal = hexToNibble(low);
  if (highVal < 0 || lowVal < 0) {
    return false;
  }
  output = static_cast<uint8_t>((highVal << 4) | lowVal);
  return true;
}

void applyColorFromCommand(const String& message) {
  const String prefix = "CFG:COLOR:";
  if (!message.startsWith(prefix)) {
    return;
  }

  String colorPart = message.substring(prefix.length());
  colorPart.trim();
  if (colorPart.length() != 7 || colorPart.charAt(0) != '#') {
    Serial.println("Invalid color format");
    return;
  }

  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;
  if (!parseHexByte(colorPart, 1, red) || !parseHexByte(colorPart, 3, green) || !parseHexByte(colorPart, 5, blue)) {
    Serial.println("Invalid hex color value");
    return;
  }

  scrollingLayer1.setColor({red, green, blue});
  Serial.print("Set color: ");
  Serial.println(colorPart);
}

bool applyConfigCommand(const String& message) {
  if (message.startsWith("CFG:SPEED:")) {
    applySpeedFromCommand(message);
    return true;
  }
  if (message.startsWith("CFG:COLOR:")) {
    applyColorFromCommand(message);
    return true;
  }
  return false;
}

String formatForDisplay(const String& rawMessage) {
  String msg = rawMessage;
  msg.replace("|", " x ");
  msg += " ";
  return msg;
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(UART_BAUD);

  matrix.addLayer(&scrollingLayer1); 
  matrix.begin();

  scrollingLayer1.setMode(wrapForward);
  scrollingLayer1.setColor({0xff, 0x00, 0x00});
  scrollingLayer1.setSpeed(10);
  // Smaller font prevents per-glyph descender clipping on 32px panel.
  scrollingLayer1.setFont(&FreeMono9pt7b);
  // FreeMono9pt7b yAdvance is 18px, centered in a 32px matrix.
  scrollingLayer1.setOffsetFromTop((kMatrixHeight - 18) / 2);

  // Start with default names
  scrollingLayer1.start("unsorted", -1);
  Serial.println("Teensy LED started");

}

void loop() {
  // Read newline-delimited messages from XIAO and push them to the scrolling layer.
  while (Serial1.available() > 0) {
    char c = static_cast<char>(Serial1.read());
    if (c == '\r') {
      continue;
    }
    if (c == '\n') {
      incomingLine.trim();
      if (incomingLine.length() > 0) {
        if (!applyConfigCommand(incomingLine)) {
          String formatted = formatForDisplay(incomingLine);
          Serial.print("Display message: ");
          Serial.println(formatted);
          scrollingLayer1.start(formatted.c_str(), - 1);
        }
      }
      incomingLine = "";
      continue;
    }
    if (incomingLine.length() < 120) {
      incomingLine += c;
    }
  }
}
