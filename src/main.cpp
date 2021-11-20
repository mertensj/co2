#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include <InfluxDbClient.h>

//
// ProLiant : Home Assistant Node
//
#define INFLUXDB_URL "http://192.168.0.199:8086"
// InfluxDB v1 database name 
#define INFLUXDB_DB_NAME "hass"
// InfluxDB client instance for InfluxDB 1
InfluxDBClient influx(INFLUXDB_URL, INFLUXDB_DB_NAME);

// Data point
Point sensor("co2");


// WIFI HOME
//
const char ssid[] = "xxxxxxxxxxxxxxxxxxxx";
const char password[] = "yyyyyyyyyyyyyyyy"; 

ESP8266WebServer server(80);



String page = "";
int LEDPin = 14;



#include <MHZ19.h>                                        
#include <SoftwareSerial.h>                                // Remove if using HardwareSerial

#define RX_PIN 13              // 13 Rx pin which the MHZ19 Tx pin is attached to
#define TX_PIN 15              // 15 Tx pin which the MHZ19 Rx pin is attached to
#define BAUDRATE 9600                                      // Device to MH-Z19 Serial baudrate (should not be changed)

MHZ19 myMHZ19;                                             // Constructor for library
SoftwareSerial co2Serial(RX_PIN, TX_PIN);                   // (Uno example) create device to MH-Z19 serial

unsigned long getDataTimer = 0;

// Variables to be exposed to the API
int Temp;
int CO2;


#include <Adafruit_NeoPixel.h>

#define pixelpin 5
#define nr_of_pixels 60
#define DELAYVAL 50   // Time (in milliseconds) to pause between pixels
// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

//Adafruit_NeoPixel pixel(nr_of_pixels, pixelpin, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(nr_of_pixels, pixelpin, NEO_GRB + NEO_KHZ800);


void colorFill(uint32_t value) {
// co2 value :
// 000-450 : GREEN   : 0  ,255,0
// 450-500 : yellow  : 255,255,0
// 500-600 : orange  : 255,145,0
// 600-800 : red     : 255,0  ,0

  //strip.clear();

  int delta = value-300;  // offfset the below 300 ppm
  int numPix= 0;
  if (delta > 0) {
    numPix = delta / 8;
    for(int i=0; i<=numPix; i++) {
     strip.setPixelColor(i, strip.Color(0,   255,   0)); 
     strip.show();
    }

    for(int i=numPix; i<strip.numPixels(); i++) {
     strip.setPixelColor(i, strip.Color(10,  10,   10)); 
     strip.show();
    }



    if (numPix > 20) {
      for(int i=20; i<=numPix; i++) {
        strip.setPixelColor(i, strip.Color(255,   255,   0)); 
       strip.show();
     }
    }
    if (numPix > 26) {
      for(int i=26; i<=numPix; i++) {
        strip.setPixelColor(i, strip.Color(255,   145,   0)); 
        strip.show();
      }
    }
    if (numPix > 38) {
      for(int i=38; i<=numPix; i++) {
       strip.setPixelColor(i, strip.Color(255,   0,   0)); 
       strip.show();
      }
    }
  }
}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}


void setup(void){
  //the HTML of the web page
  page = "<h1>Simple NodeMCU Web Server</h1><p><a href=\"LEDOn\"><button>ON</button></a>&nbsp;<a href=\"LEDOff\"><button>OFF</button></a></p>";
  //make the LED pin output and initially turned off
  pinMode(LEDPin, OUTPUT);
  digitalWrite(LEDPin, LOW);
   
  delay(1000);
  Serial.begin(115200);
  WiFi.begin(ssid, password); //begin WiFi connection
  Serial.println("");
 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
   
  server.on("/", [](){
    server.send(200, "text/html", page);
  });
  server.on("/LEDOn", [](){
    server.send(200, "text/html", page);
    digitalWrite(LEDPin, HIGH);
    delay(1000);
  });
  server.on("/LEDOff", [](){
    server.send(200, "text/html", page);
    digitalWrite(LEDPin, LOW);
    delay(1000); 
  });
  
  server.begin();
  Serial.println("Web server started!");



  Serial.println(F("...mhz19 calibration..."));

  co2Serial.begin(BAUDRATE);                               // (Uno example) device to MH-Z19 serial start   
  myMHZ19.begin(co2Serial);                                // *Serial(Stream) refence must be passed to library begin(). 

  myMHZ19.autoCalibration();                              // Turn auto calibration ON (OFF autoCalibration(false))



  // Add constant tags - only once
  sensor.addTag("Device", "ESP8266");
  sensor.addTag("Sensor", "MHZ19");

  // Check server connection
  if (influx.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(influx.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(influx.getLastErrorMessage());
  }

  Serial.println(F("...NeoPixel..."));
  // NeoPixel
  //strip.begin();
  //strip.clear();
  //strip.setBrightness(225);

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(25); // Set BRIGHTNESS to about 1/5 (max = 255)

  //colorWipe(strip.Color(255,   0,   0), 50); // Red
  //colorWipe(strip.Color(  0, 255,   0), 50); // Green
  //colorWipe(strip.Color(  0,   0, 255), 50); // Blue

  // Do a theater marquee effect in various colors...
  //theaterChase(strip.Color(127, 127, 127), 50); // White, half brightness
  //theaterChase(strip.Color(127,   0,   0), 50); // Red, half brightness
  //theaterChase(strip.Color(  0,   0, 127), 50); // Blue, half brightness

  //rainbow(10);             // Flowing rainbow cycle along the whole strip
  theaterChaseRainbow(50); // Rainbow-enhanced theaterChase variant
  strip.clear();
  strip.show();
  // The first NeoPixel in a strand is #0, second is 1, all the way up
  // to the count of pixels minus one.
  //for(int i=0; i<nr_of_pixels; i++) { // For each pixel...

    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    // Here we're using a moderately bright green color:
    //strip.setPixelColor(i, strip.Color(0, 150, 0));

    //strip.show();   // Send the updated pixel colors to the hardware.

    //delay(DELAYVAL); // Pause before next pass through loop
  //}

  Serial.println(F("Setup Complete..........."));
}
 
void loop(void){
  server.handleClient();


  if (millis() - getDataTimer >= 4000)
  {
    /* note: getCO2() default is command "CO2 Unlimited". This returns the correct CO2 reading even 
    if below background CO2 levels or above range (useful to validate sensor). You can use the 
    usual documented command with getCO2(false) */

    CO2 = myMHZ19.getCO2();                             // Request CO2 (as ppm)
        
    Serial.print(F("  - CO2 (ppm): "));                      
    Serial.print(CO2);                                

    Temp = myMHZ19.getTemperature()-2;                     // Request Temperature (as Celsius)
    Serial.print(F("  - Temperature (C): "));                  
    Serial.println(Temp);                               

    getDataTimer = millis();
  }

  // NeoPixel strip - make co2 level visible
  colorFill(CO2);


  // Store measured value into point
  sensor.clearFields();
  sensor.addField("co2", CO2);
  sensor.addField("temp", Temp);
  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(influx.pointToLineProtocol(sensor));
  // If no Wifi signal, try to reconnect it
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi connection lost");
  }
  // Write point
  if (!influx.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(influx.getLastErrorMessage());
  }

  //Wait x seconds
  Serial.println("Wait 15s");
  delay(15000);

}
