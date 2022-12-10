#include <WiFiNINA.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <String.h>
#define PIN 12
#define NUM_PIXELS 240
Adafruit_NeoPixel strip(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);


char ssid[] = "";
char pass[] = "";
int keyIndex = 0;
int status = WL_IDLE_STATUS;

int16_t m_R,m_G,m_B = 0;
WiFiServer server(80);

WiFiClient client = server.available();

int ledPin = 2;
// The action that we want the LEDs to do.
int action = 0;

const char data[] = "<!DOCTYPE HTML><html><head><title>TEST</title> \
</head><body> \
<p>Select Light Sequence Option:</p> \
<form action =\"/\" method =\"post\"> \
  <label for=\"Sequences\">Choose a Sequence:</label> \
  <select name=\"Sequence\" id=\"Sequence\"> \
    <optgroup label=\"Normal Sequences\"> \
      <option value=\"IntroSweep\">Intro Sweep</option> \
      <option value=\"SlowRGB\">Slow RGB</option> \
      <option value=\"OFF\">OFF</option \
    </optgroup> \
    <optgroup label=\"Favorite Colors\"> \
      <option value=\"Mark\">Mark</option> \
      <option value=\"Tajh\">Tajh</option> \
    </optgroup> \
  </select> \
  <br><br> \
  <input type=\"submit\" value=\"Submit\"> \
</form> \
<form action =\"/\" method =\"post\"> \
  <label for=\"favcolor\">Select your favorite color:</label> \
  <input type=\"color\" id=\"favcolor\" name=\"favcolor\" value=\"#ff0000\"> \
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
  if (client) 
  {
    printWEB();
  }
  if (action == 1)
  {
    Strand();
  }
  if(action==2)
  {
    TurnOFF();
  }
  
  if(action==3)
  {
    IntroSweep();
  }
  if(action==4)
  {
    OneColor(m_R,m_G,m_B);
  }
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
    Serial.print(F("postParameter:")); Serial.println(postParameter);
     //Do special checks here for what the POST REquest was.
     if(strncmp(postParameter, "Sequence",8) == 0)
     {
       Serial.println("GOT A SEQUNCE");
       // Get the postion of the '=' which comes after the word Sequence
       const char * ptr = strchr(postParameter, '=');
       // check to make sure pointer is not NULL
       if(ptr !=NULL)
       {
         size_t pos = ptr - postParameter +1;
         if(strncmp(postParameter + pos, "SlowRGB",7)==0)
         {
           action = 1;
         }
         if(strncmp(postParameter+pos,"OFF",3)==0)
         {
           action = 2;
         }
         if(strncmp(postParameter+pos,"IntroSweep",10) ==0)
         {
           action = 3;
         }
      }
     }
     if(strncmp(postParameter, "favcolor",8)==0)
     {
       Serial.println("Got a Favorite Color");
        action = 4;
        const char * ptr = strchr(postParameter, '=');
        if(ptr !=NULL)
        {
          size_t pos = ptr - postParameter +4;
          Serial.println(pos);
          //char postParameter[smallbuffersize];
          char R[2]{}; 
          char blh[2]{};
          char B[2]{};
          char full[6]{};

          strlcpy(blh,postParameter+pos,3);
          Serial.print("Blh: ");
          Serial.println(blh);
          int8_t val = atoi(blh);
          Serial.println(val);
          pos  = pos +2;
          strlcpy(R,postParameter+pos,3);
          Serial.print("R: ");
          Serial.println(R);
          pos  = pos +2;
          strlcpy(B,postParameter+pos,3);
          Serial.print("B: ");
          Serial.println(B);

          strlcpy(full,postParameter+pos,7);
          Serial.print("full:");
          Serial.println(full);
          char * end;
        int16_t var = strtol(blh,&end,16);
        Serial.print("VAL: ");
        Serial.println(var);

          if(strcmp(R,"ff") == 0)
          {
            Serial.println("GOT LOWER");
          }     
        }
     }
      
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
void TurnOFF()
{
  colorWipe(strip.Color(0, 0, 0), 50);  // Red  
}

void IntroSweep()
{
  // Sweep through the favorite colors of the players before the game starts.
  //NOTE: currently a new command will not be heard until a the current sequnece that is running is finished. 
  //Do Tahj First.
  colorWipe(strip.Color(253,238,0),25); // Auerolin Yellow.
  // Then do Andrew
  colorWipe(strip.Color(32,69,1),25);
  // Then do Christine
  colorWipe(strip.Color(3,198,252),25);
  // Then do Me
  colorWipe(strip.Color(204,85,0),25);
  
}
void OneColor(int16_t R, int16_t G, int16_t B){
  //colorWipe(strip.Color(R,G,B,),50);
  
}
void colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < strip.numPixels(); i++) {  // For each pixel in strip...
    strip.setPixelColor(i, color);               //  Set pixel's color (in RAM)
    strip.show();                                //  Update strip to match
    delay(wait);                                 //  Pause for a moment
  }
}


//client.write(data);