#include <WiFiNINA.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#define PIN 12
#define NUM_PIXELS 300
Adafruit_NeoPixel strip(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);


char ssid[] = "";
char pass[] = "";
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
  // Set up the state machine
  uint8_t i = 0;
  const uint16_t buffersize = 100;
  const uint16_t smallbuffersize = 30;
  char lineBuffer[buffersize] {'\0'};
  char method[8]; // Used to find the method of the Request (GET/POST etc)
  char uri[smallbuffersize];
  char requestParameter[smallbuffersize];
  char postParameter[smallbuffersize];
  enum class Status {REQUEST,CONTENT_LENGTH, EMPTY_LINE, BODY};
  Status status = Status::REQUEST;
  
  // Check if the client is still connected
  String currentLine = "";
  while (client.connected()) {
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
      if (c == '\n') {
        if (currentLine.length() == 0) {
          if(status == Status::REQUEST)
          {
            char *ptr;
            ptr = strtok(lineBuffer, " ");
            strlcpy(method, ptr, smallbuffersize);
            Serial.print(F("method=")); Serial.println(method);
            ptr = strtok(NULL, " ");
            strlcpy(uri, ptr, smallbuffersize);

            if (strchr(uri, '?') != NULL)
            {
              ptr = strtok(uri, "?");  // split URI from parameters
              strcpy(uri, ptr);
              ptr = strtok(NULL, " ");
              strcpy(requestParameter, ptr);
              Serial.print(F("requestParameter=")); Serial.println(requestParameter);              
            }
            Serial.print(F("uri=")); Serial.println(uri);
            status = Status::EMPTY_LINE;
          }
          else if(status == Status::CONTENT_LENGTH)                                
          {
            status = Status::EMPTY_LINE;            
          }
          else if(status > Status::REQUEST && i <2)
          {
            status = Status::BODY;
          }
          else if(status == Status::BODY)
          {
            strlcpy(postParameter, lineBuffer, smallbuffersize);
            break;
          }
          i=0;
          strcpy(lineBuffer, "");
        } else {
          currentLine = "";
        }
      }
       else 
      {
        if( i< buffersize)
        {
            lineBuffer[i] = c;
            i++;
            lineBuffer[i] = '\0';          
        }        
      }
    }
    if(status == Status::BODY)
    {
      strlcpy(postParameter,lineBuffer, smallbuffersize);
    }    
    Serial.println();
    Serial.print(F("postParameter"));
    Serial.println(postParameter);
     //Do special checks here for what the POST REquest was. 
    Serial.print("URI: ");
    Serial.println(uri);     
    if(!strcmp(uri, "/"))
    {
      client.write(data);    
    }
    delay(50);
    client.stop();
  }
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


//client.write(data);