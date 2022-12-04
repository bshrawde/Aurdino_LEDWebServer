#include <WiFiNINA.h>
#include <Adafruit_NeoPixel.h>


#define PIN        12
#define NUM_PIXELS 300
Adafruit_NeoPixel strip(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

char ssid[] ="";
char pass[] ="";
int keyIndex = 0;
int status = WL_IDLE_STATUS;
WiFiServer server(80);

WiFiClient client = server.available();

int ledPin = 2;
int action = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(ledPin,OUTPUT);
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

  while(!Serial);

  enable_WiFi();
  connect_WiFi();

  server.begin();
  printWifiStatus();

  
}

void loop() {
  // put your main code here, to run repeatedly:
  client = server.available();
  if(client)
  {
    printWEB();
  }
  if(action ==1)
  {
    Strand();
  }
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print ("SSID: ");
  Serial.println(WiFi.SSID());

  // Print your board's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");

  Serial.print("To See this page in action, open a broswer to http://");
  Serial.println(ip);
}

void enable_WiFi()
{
  Serial.println("Enabling WIFI");
  // check for WiFi module:
  if(WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Communication with WiFi Module Failed");
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0")
  {
    Serial.println("Please upgrade the firmware");
  }
}

void connect_WiFi()
{
  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.print(ssid);
    Serial.println();

    status = WiFi.begin(ssid,pass);

    // wait 10 seconds for connection
    delay(10000);
  }
}

void printWEB()
{
  if(client)
  {
    String currentLine = "";
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        Serial.write(c);
        if(c == '\n')
        {
          if (currentLine.length() == 0)
          {
            client.println("HTTP/1.1 200 OK") ;
            client.println("Content-type:text/html");
            client.println();

            client.print("Click <a href=\"/H\">here</a> turn the LED on <br>");
            client.print("Click <a href=\"/L\">here</a> turn the LED off<br><br>");
            client.print("Click <a href=\"/L\">here</a> turn the LEDS ON<br><br>");

            int randomReading = analogRead(A1);
            client.print("Random reading from analog pin:");
            client.print(randomReading);

            client.println();
            break;
          }
          else
          {
            currentLine = "";
          }
        }
        else if (c != '\r')
        {
          currentLine += c;
        }
        if (currentLine.endsWith("GET /H"))
        {
          //digitalWrite(ledPin,HIGH);
          action = 1;
        }
        if (currentLine.endsWith("GET /L"))
        {
          digitalWrite(ledPin,LOW);
          action = 0;
        }
        
      }
    }
    client.stop();
    Serial.println("client disconnected");
  }
}

void Strand()
{
  colorWipe(strip.Color(255,   0,   0), 50); // Red
  colorWipe(strip.Color(  0, 255,   0), 50); // Green
  colorWipe(strip.Color(  0,   0, 255), 50); // Blue
}


void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

