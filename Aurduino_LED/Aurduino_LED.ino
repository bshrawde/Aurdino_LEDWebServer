#include <WiFiNINA.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#define PIN 12
#define NUM_PIXELS 300
Adafruit_NeoPixel strip(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);


char ssid[] = "Valar_Morgules";
char pass[] = "Dohaeris__Valar2008!";
int keyIndex = 0;
int status = WL_IDLE_STATUS;
WiFiServer server(80);

WiFiClient client = server.available();

int ledPin = 2;
int action = 0;

const char data[] = "<!DOCTYPE HTML><html><head><title>TEST</title> \
</head><body> \
<p>Select Light Sequence Option:</p> \
<form action =\"/\" method =\"post\"> \
  <label for=\"cars\">Choose a car:</label> \
  <select name=\"cars\" id=\"cars\"> \
    <optgroup label=\"Swedish Cars\"> \
      <option value=\"volvo\">Volvo</option> \
      <option value=\"saab\">Saab</option> \
    </optgroup> \
    <optgroup label=\"German Cars\"> \
      <option value=\"mercedes\">Mercedes</option> \
      <option value=\"audi\">Audi</option> \
    </optgroup> \
  </select> \
  <br><br> \
  <input type=\"submit\" value=\"Submit\"> \
</form> \
</body></html>";

// <form action=\"/post\"> </form>
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  strip.begin();            // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();             // Turn OFF all pixels ASAP
  strip.setBrightness(50);  // Set BRIGHTNESS to about 1/5 (max = 255)

  while (!Serial)
    ;

  // Connect to WiFi network.
  connect_WiFi();

  // Start the Web Server.
  server.begin();
  // Get the IP address of the Server
  IPAddress ip = WiFi.localIP();
  // Print the IP address
  Serial.print("IP Address: ");
  Serial.println(ip);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  client = server.available();
  if (client) {
    printWEB();
  }
  //if (action == 1) {
  //  Strand();
  //}
}

void connect_WiFi() {
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Try to connect to the WiFi
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection
    delay(10000);
  }
}

void printWEB() {
  // Check if the client is still connected
  String currentLine = "";
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      Serial.write(c);
      if (c == '\n') {
        if (currentLine.length() == 0) {
          // Write our HTML page to the Client.
          client.write(data);
          break;
        } else {
          currentLine = "";
        }
      } else if (c != '\r') {
        currentLine += c;
      }
      if (currentLine.endsWith("POST /H")) {
        Serial.println("GOT POST");
      }
    }
  }
  client.stop();
}

void Strand() {
  colorWipe(strip.Color(255, 0, 0), 50);  // Red
  colorWipe(strip.Color(0, 255, 0), 50);  // Green
  colorWipe(strip.Color(0, 0, 255), 50);  // Blue
}


void colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < strip.numPixels(); i++) {  // For each pixel in strip...
    strip.setPixelColor(i, color);               //  Set pixel's color (in RAM)
    strip.show();                                //  Update strip to match
    delay(wait);                                 //  Pause for a moment
  }
}
