
/**************************************************************************
  This is a library for several Adafruit displays based on ST77* drivers.

  This example works with the 1.14" TFT breakout
    ----> https://www.adafruit.com/product/4383
  The 1.3" TFT breakout
    ----> https://www.adafruit.com/product/4313
  The 1.47" TFT breakout
    ----> https://www.adafruit.com/product/5393
  The 1.54" TFT breakout
    ----> https://www.adafruit.com/product/3787
  The 1.69" TFT breakout
    ----> https://www.adafruit.com/product/5206
  The 1.9" TFT breakout
    ----> https://www.adafruit.com/product/5394
  The 2.0" TFT breakout
    ----> https://www.adafruit.com/product/4311


  Check out the links above for our tutorials and wiring diagrams.
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional).

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 **************************************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <SdFat_Adafruit_Fork.h>  // SD card & FAT filesystem library
#include <Adafruit_ImageReader.h> // Image-reading functions

#define TFT_CS   33
#define TFT_RST  48 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC   37

#define TFT_MOSI 35  // Data out
#define TFT_SCLK 36  // Clock out

#define SD_CS 26
#define TFT_MISO 34

#define USE_SD_CARD

//SPIClass hspi = SPIClass(HSPI);
//#define SD_CONFIG SdioConfig(TFT_SCLK, TFT_MOSI, TFT_MISO, HSPI)
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

SdFat                SD;         // SD card filesystem
Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys
Adafruit_Image       img;        // An image loaded into RAM
int32_t              width  = 0, // BMP image dimensions
                     height = 0;
//ImageReturnCode      stat;
//SPIClass SDSPI = SPIClass();
float p = 3.1415926;
int i = 0;

void setup(void) {
  Serial.begin(9600);
  Serial.print(F("Hello! ST77xx TFT Test"));

  pinMode(SD_CS, OUTPUT);
  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI);
  //hspi.begin(TFT_SCLK, TFT_MISO, TFT_MOSI);
  // OR use this initializer (uncomment) if using a 2.0" 320x240 TFT:
  tft.init(240, 320);           // Init ST7789 320x240
  tft.setRotation(3);
  // SPI speed defaults to SPI_DEFAULT_FREQ defined in the library, you can override it here
  // Note that speed allowable depends on chip and quality of wiring, if you go too fast, you
  // may end up with a black screen some times, or all the time.
  //tft.setSPISpeed(40000000);
  Serial.println(F("Initialized"));

  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);


  time = millis() - time;
  Serial.println(time, DEC);
  // //uint8_t BYUILogo [33600];
  // HSPI.setSCK(TFT_SCLK);
  // HSPI.setTX(TFT_MOSI);
  // HSPI.setRX(TFT_MISO);

// SD card is pretty straightforward, a single call...
  if(!SD.begin(SD_CS)) { // Breakouts require 10 MHz limit due to longer wires
    Serial.println(F("SD begin() failed"));
    for(;;); // Fatal error, do not continue
  }
  Serial.print(F("Loading BYUILogo.bmp to screen..."));
  reader.drawBMP("/BYUILogo.bmp", tft, 0, 0);

  Serial.println("done");
  delay(1000);
}

void loop() {
  tft.invertDisplay(true);
  delay(500);
  tft.invertDisplay(false);
  delay(500);
  
}