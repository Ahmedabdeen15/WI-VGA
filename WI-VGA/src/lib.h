#ifndef lib_h
#define lib_h
#include "Arduino.h"
#include "ESPAsyncWebServer.h"
#include <SD.h>
//C:\Users\ahmed\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.5\libraries\SD
//Not used: C:\Users\ahmed\AppData\Local\Arduino15\libraries\SD
#include <FS.h>
#include <WiFi.h>
#include <ESP32Lib.h>
#include <Ressources/Font6x8.h>
#include <ESPmDNS.h>
#include <WebSocketsServer.h>
#include <AsyncTCP.h>
#include "webpages.h"
#include <ArduinoJson.h>
//#include <WebSerial.h>
#include <AsyncElegantOTA.h>
#include "image.h"
#include "mbedtls/md.h"
#define FIRMWARE_VERSION "v0.3.0"

const char* default_ssid = "WGMA";
const char* default_wifipassword = "WMGA01003576914@am";
const char* default_httpuser = "user";
const char* default_httppassword = "user";
const char* default_admin_httpuser = "admin";
const char* default_admin_httppassword = "admin";
const int default_webserverporthttp = 80;
const char* ssid_local = "MyCircuits";
const char* wifipassword_local = "12345678";
bool default_create_AP=true;
bool default_connect_Wifi=true;//shoud be false
const char* servername = "WI-VGA";

// configuration structure
struct Config {
  String ssid;               // wifi ssid
  String wifipassword;       // wifi password
  String ssid_local;               // wifi ssid
  String wifipassword_local;       // wifi password
  String httpuser;           // username to access web 
  String httppassword;       // password to access web 
  String admin_httpuser;           // username to access web admin
  String admin_httppassword;       // password to access web admin
  int webserverporthttp;     // http port number for web admin
  bool connect_wifi;
  bool create_AP;
  String servername;
};

Config config;                        // configuration
bool shouldReboot = false;            // schedule a reboot
AsyncWebServer *server;               // initialise webserver
bool time_out=false;

bool is_pdf=false;
uint32_t time_;
File logfile;


//if spiffs us it
//#include "SPIFFS.h" // ESP32 only

// inti sd

#define SCK  21
#define MISO  36
#define MOSI  0
#define CS  1


//#define servername "WI-VGA" //Define the name to your server...
//WebSocketsServer webSocket = WebSocketsServer(81);    // the websocket uses port 81 (standard port for websockets



 uint32_t t;
 String image_path;
// init var to use ps-ram
int n_rows = 3;
int no_pixel=91000;
unsigned short *sprites_Pixels;


//pin configuration
const int redPins[] = {13, 27, 18, 2, 15};
const int greenPins[] = {14, 12, 4, 19, 5};
const int bluePins[] = {32, 33, 22, 23};
const int hsyncPin = 25;
const int vsyncPin = 26;

//VGA Device
VGA14BitI vga;

// Set your Static IP address
IPAddress local_IP(192, 168, 1, 184);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional


struct SpiRamAllocator {
        void* allocate(size_t size) {
                return ps_malloc(size);

        }
        void deallocate(void* pointer) {
                free(pointer);
        }
};

using SpiRamJsonDocument = BasicJsonDocument<SpiRamAllocator>;

void start_vga();
void init_psram();
void MDNS_int();
void init_webserver();
void init_sd();
void init_wifi(bool create_AP,bool connect_wifi);
void init_config();
void check_file_structure();
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
bool removeDir(fs::FS &fs, const char * path);
String listDir_html(fs::FS &fs, String dirname, uint8_t levels);
String listFiles();
void update_config(String httpuser,String httppassword,int webserverporthttp,String ssid,String wifipassword,String ssid_local,String wifipassword_local,String servername,String connect_Wifi,String create_AP);
bool String_To_Bool(String var){if(var=="true" || var=="1"){return true;}if(var=="false" || var=="0"){return false;}}
bool checkUserWebAuth_Admin(AsyncWebServerRequest * request);
void log(String log);

inline const char * typeStr (int   var) { return " int "; }
inline const char * typeStr (bool   var) { return " bool "; }
inline const char * typeStr (long  var) { return " long "; }
inline const char * typeStr (float var) { return " float "; }
inline const char * typeStr (const char *var) { return " char "; }

SPIClass spi = SPIClass(VSPI);
#endif
//Bitmap bmp();
//uint16_t *spaceship_data;
//Bitmap galaxy;


//uint8_t pixel_=r;
  //pixel_=pixel_<<2+g;
  //pixel_=pixel_<<4+b;
  //Serial.println(pixel_);
  //array2D[y * n_columns + x]=rgb565;


  //myBitmap.setPixel(x, y, RGB888(r, g, b));
  //cv.setPixel(x, y, RGB888(array2D[x][y].r, array2D[x][y].g, array2D[x][y].b));
  //spaceship_data[y * n_columns + x]=0;
  //galaxy.setPixel(x,y,fabgl::Bitmap::RGBA888(r, g, b,0));
//spaceship_data= (uint8_t *) ps_malloc(n_rows * n_columns * sizeof(uint8_t)); // Create an array of n_rows x n_columns
  //galaxy = Bitmap(n_rows, n_columns, &spaceship_data[0], PixelFormat::Native);


  //array2D[x+i][y]=bitmap[j * w + i];
  //array2D= (uint16_t *) ps_malloc(n_rows * n_columns * sizeof(uint16_t)); // Create an array of n_rows x n_columns

  
  /*
  void server_response(void * param)
{
  while(true)
  {
  server.handleClient(); //Listen for client connections
  Serial.println(uxTaskGetStackHighWaterMark(NULL));
    
  vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

  xTaskCreate(
    server_response,
    "server_response",
    1500,
    NULL,
    1,
    NULL
  );*/
  /*  //remove
  server->on("/test", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + + " " + request->url();

    if (is_authenticated(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      //request->send_P(200, "text/html", index_html, processor);
      request->send(SD, "/web_app/test.html", String(), false);
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      request->redirect("/loginpage");
    }

  });*/