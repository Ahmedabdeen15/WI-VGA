#include "lib.h"

/*msg = "Wrong username/password! try again.";
Serial.println("Log in Failed");
AsyncWebServerResponse *response = request->beginResponse(301); //Sends 301 redirect
response->addHeader("Location", "/login.html?msg=" + msg);
response->addHeader("Cache-Control", "no-cache");
request->send(response);
return;

else if()
        {
          Sprites sprites(1, spritesPixels_loading, spritesOffsets, spritesRes, spritesPoints, spritesPointOffsets, Sprites::PixelFormat::R5G5B4A2);
          sprites.draw(vga,0, 160, 100);
          vTaskDelay(1000/portTICK_PERIOD_MS);
        }

*/

void IMAGE_SHOW_AND_RENDER(void * param)
{
    while (true)
    {
        File root = SD.open("/rendered_images");
        File image = root.openNextFile();
        if(!image|| loading)
        {
          log("true");
          Sprites sprites(1, spritesPixels, spritesOffsets, spritesRes, spritesPoints, spritesPointOffsets, Sprites::PixelFormat::R5G5B4A2);
          
          sprites.draw(vga,0, 160, 100);
          if (loading)
          {
            vga.setCursor(115, 150);
            vga.print("loading....");
          }
          vTaskDelay(20000/portTICK_PERIOD_MS);
        }
        
        else
          while(image && !loading){

            // Time recorded for test purposes
            t = millis();
            image_path="/rendered_images/";
            image_path+=image.name();
            //Serial.println(image_path);
            if(String(image.name()).endsWith(".txt")){
              // Determine the file size
              size_t fileSize = image.size();
  
              // Allocate a buffer to hold the file contents
              char* fileBuffer = (char*)ps_calloc(fileSize + 1,sizeof(char*));
  
              // Read the file into the buffer
              image.readBytes(fileBuffer, fileSize);
              fileBuffer[fileSize] = '\0'; // Null-terminate the buffer
  
              // Parse integers from the buffer
              char* ptr = fileBuffer;
              for (int i = 0; (i < no_pixel) && *ptr != '\0'; i++) {
                sprites_Pixels[i] = strtol(ptr, NULL, 0);
                
                // Find the next comma delimiter
                ptr = strchr(ptr, ',');
                if (ptr == nullptr) {
                  break; // No more commas found, exit the loop
                }
                
                ptr++; // Move the pointer past the comma
              }
  
            // Free the buffer
            free(fileBuffer);
            
            // Close the file
            image.close();

            Sprites sprites(1, sprites_Pixels, spritesOffsets, spritesRes, spritesPoints, spritesPointOffsets, Sprites::PixelFormat::R5G5B4A2);

            sprites.draw(vga,0, 160, 100);
            
            //show the rendering
            //vga.show();
            t = millis() - t;
            Serial.print(t); Serial.println(" ms");
            vTaskDelay(50000/portTICK_PERIOD_MS);
            }
            image = root.openNextFile();
            
          }
          
    }
    
}


void setup() {
  delay(3000); // power-up safety delay  
  Serial.begin(115200);
  Serial.println("Booting ...");

  
  init_psram(); 
  init_sd();
  check_file_structure();
  
  logfile = SD.open("/log.txt", FILE_WRITE);

  if (!logfile) {
    Serial.println("Error opening log file!");
    return;
  }

  init_config();
  Serial.println(config.connect_wifi);
  Serial.println(config.create_AP);
  init_wifi(config.create_AP,config.connect_wifi);


  MDNS_int();
  init_webserver();
  start_vga();
  xTaskCreatePinnedToCore(
    IMAGE_SHOW_AND_RENDER,
    "IMAGE_SHOW_AND_RENDER",
    3000,
    NULL,    
    9,
    NULL,
    1
  );
}

void loop() {
  // reboot if we've told it to reboot
  if (shouldReboot) {
    shouldReboot=false;
    rebootESP("Web Admin Initiated Reboot");
  }
  if(millis()>=tokenCreationTime + (30 * 60 * 1000))
  {
    loading=false;
    user=false;
    tokenCreationTime=0;
  }
  vTaskDelay(10/portTICK_PERIOD_MS);
}

void rebootESP(String message) {
  //Serial.print("Rebooting ESP32: "); Serial.println(message);
  ESP.restart();

}

// list all of the files, if ishtml=true, return html rather than simple text
//remove
String listFiles(bool ishtml) {
  String returnText = "";
  Serial.println("Listing files stored on SD");
  File root = SD.open("/");
  File foundfile = root.openNextFile();
  if (ishtml) {
    returnText += "<table><tr><th align='left'>Name</th><th align='left'>Size</th><th></th><th></th></tr>";
  }
  while (foundfile) {
    //if(String(foundfile.name()).endsWith(".jpg"))
      if (ishtml) {
        if(foundfile.isDirectory())
        {
          returnText += "<tr align='left'><td>" + String(foundfile.name()) + "</td><td>" + humanReadableSize(foundfile.size()) + "</td>";
          returnText += "<td><button onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'delete\')\">Delete</button></tr>";
          returnText+= listDir_html(SD, "/"+String(foundfile.name()),0);
        }
        else
        {
        returnText += "<tr align='left'><td>" + String(foundfile.name()) + "</td><td>" + humanReadableSize(foundfile.size()) + "</td>";
        returnText += "<td><button onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'download\')\">Download</button>";
        returnText += "<td><button onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'delete\')\">Delete</button></tr>";
        }
      } else {
        returnText += "File: " + String(foundfile.name()) + " Size: " + humanReadableSize(foundfile.size()) + "\n";
      }
    foundfile = root.openNextFile();
  }
  if (ishtml) {
    returnText += "</table>";
  }
  root.close();
  foundfile.close();
  return returnText;
}

SpiRamJsonDocument listFiles(SpiRamJsonDocument json,bool img,const char* folder_name) {
  File root = SD.open(folder_name);
  File foundfile = root.openNextFile();
  int x=0;
  while (foundfile) {
    /*if(String(foundfile.name()).endsWith(".jpg")||!img)
      {
        json[x]["name"] = String(foundfile.name());
        json[x]["size"] = humanReadableSize(foundfile.size());
        x++;
      }*/
    if(!foundfile.isDirectory())
      {
        json[x]["name"] = String(foundfile.name());
        json[x]["size"] = humanReadableSize(foundfile.size());
        x++;
      }

    foundfile = root.openNextFile();
  }

  root.close();
  foundfile.close();
  return json;
}
SpiRamJsonDocument listsettings(SpiRamJsonDocument json,bool img) {
  File file = SD.open("/settings.json");
   DeserializationError error = deserializeJson(json, file);
        if (error) {
            // if the file didn't open, print an error:
            log("Error parsing JSON ");Serial.println(F("Error parsing JSON "));
           log(error.c_str());Serial.println(error.c_str());
        }


  file.close();

  return json;
}
// Make size of files human readable
// source: https://github.com/CelliesProjects/minimalUploadAuthESP32
String humanReadableSize(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

// parses and processes webpages
// if the webpage has %SOMETHING% or %SOMETHINGELSE% it will replace those strings with the ones defined
String processor(const String& var) {
  if (var == "FIRMWARE") {
    return FIRMWARE_VERSION;
  }

  if (var == "FREESD") {
    return humanReadableSize((SD.totalBytes() - SD.usedBytes()));
  }

  if (var == "USEDSD") {
    return humanReadableSize(SD.usedBytes());
  }

  if (var == "TOTALSD") {
    return humanReadableSize(SD.totalBytes());
  }
}

void configureWebServer() {
  // configure web server
  //Serial.begin(server);
  // if url isn't found
  server->onNotFound(notFound);

  // run handleUpload function when any file is uploaded
  server->onFileUpload(handleUpload);

  
  server->on("/loginpage", HTTP_GET, [](AsyncWebServerRequest * request) {
    log("loginpage:");
    if(user)
    {
      log("user:true that is write");
      request->send(SD, "/web_app/error.html", String(), false);

    }
    if(!is_authenticated(request))
    {
      log("user:false first time");
      request->send(SD, "/web_app/login.html", String(), false);
    }
    else
      request->redirect("/");

  });
  server->on("/login", HTTP_POST, handleLogin);

  server->on("/logout", HTTP_GET, handleLogout);

  server->on("/error", HTTP_GET, [](AsyncWebServerRequest * request) {
      String logmessage = "Client:" + request->client()->remoteIP().toString() + + " " + request->url();

      if (is_authenticated(request)) {
          request->send(SD, "/web_app/error.html", String(), false);
      } else {
        logmessage += " Auth: Failed";
        Serial.println(logmessage);
        request->redirect("/loginpage");
      }

    });

  server->on("/admin", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + + " " + request->url();

    if (checkUserWebAuth_Admin(request)) {
        request->send(SD, "/web_app/file_upload.html", String(), false, processor);
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }

  });
  server->on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + + " " + request->url();

    if (is_authenticated(request)) {
        request->send(SD, "/web_app/index.html", String(), false);
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      request->redirect("/loginpage");
    }

  });
  server->on("/files", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + + " " + request->url();

    if (is_authenticated(request)) {
        request->send(SD, "/web_app/files.html", "text/html",false);
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      request->redirect("/loginpage");
    }

  });
  server->on("/uploadImage", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + + " " + request->url();

    if (is_authenticated(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      request->send(SD, "/web_app/uploadImage.html", String(), false);
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      request->redirect("/loginpage");
    }

  });


   server->on("/settings", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + + " " + request->url();

    if (is_authenticated(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      //request->send_P(200, "text/html", index_html, processor);
      request->send(SD, "/web_app/settings_list.html", String(), false);
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      request->redirect("/loginpage");
    }

  });
  server->on("/wifisettings", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + + " " + request->url();

    if (is_authenticated(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      //request->send_P(200, "text/html", index_html, processor);
      request->send(SD, "/web_app/wifi_settings.html", String(), false);
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      request->redirect("/loginpage");
    }

  });
  server->on("/apsettings", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + + " " + request->url();

    if (is_authenticated(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      //request->send_P(200, "text/html", index_html, processor);
      request->send(SD, "/web_app/ap_settings.html", String(), false);
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      request->redirect("/loginpage");
    }

  });
   server->on("/serversettings", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + + " " + request->url();

    if (is_authenticated(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      //request->send_P(200, "text/html", index_html, processor);
      request->send(SD, "/web_app/server_settings.html", String(), false);
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      request->redirect("/loginpage");
    }

  });
  server->on("/usersettings", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + + " " + request->url();

    if (is_authenticated(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      //request->send_P(200, "text/html", index_html, processor);
      request->send(SD, "/web_app/user_settings.html", String(), false);
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      request->redirect("/loginpage");
    }

  });
  server->serveStatic("/", SD, "/web_app");
  //remove
  server->on("/reboot", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();

    if (is_authenticated(request)||checkUserWebAuth_Admin(request)) {
      request->send(200, "text/html", reboot_html);
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      shouldReboot = true;
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      request->redirect("/loginpage");
    }
  });
//delete it if you don't need admin
  server->on("/listfiles", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    if (checkUserWebAuth_Admin(request)||checkUserWebAuth_Admin(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      Serial.println(listFiles(false));
      request->send(200, "text/plain", listFiles(true));
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      request->redirect("/loginpage");
    }
  });

  server->on("/file", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    if (is_authenticated(request)||checkUserWebAuth_Admin(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);

      if (request->hasParam("name") && request->hasParam("action")) {
        const char *fileName = request->getParam("name")->value().c_str();
        const char *fileAction = request->getParam("action")->value().c_str();
        if(String(fileName)!= image_path)
        {
        logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url() + "?name=" + String(fileName) + "&action=" + String(fileAction);

        if (!SD.exists(fileName)) {
          Serial.println(logmessage + " ERROR: file does not exist");
          request->send(400, "text/plain", "ERROR: file does not exist");
        } else {
          Serial.println(logmessage + " file exists");
          if (strcmp(fileAction, "download") == 0) {
            logmessage += " downloaded";
            request->send(SD, fileName, "application/octet-stream");
          } else if (strcmp(fileAction, "delete") == 0) {
            logmessage += " deleted";
            File root = SD.open(fileName);
            if(root.isDirectory())
            {
              if(removeDir(SD,fileName))
                request->send(200, "text/plain", "Deleted folder: " + String(fileName));
              else
                request->send(400, "text/plain", "Failed to delete folder: " + String(fileName));
            }
            else
              SD.remove(fileName);
            request->send(200, "text/plain", "Deleted File: " + String(fileName));
          } else {
            logmessage += " ERROR: invalid action param supplied";
            request->send(400, "text/plain", "ERROR: invalid action param supplied");
          }
          Serial.println(logmessage);
        }
      }
      else
      {
        request->redirect("/error");
      }
      } else {
        request->send(400, "text/plain", "ERROR: name and action params required");
      }
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      request->redirect("/loginpage");
    }
  });
  
  server->on("/api/listfiles", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (is_authenticated(request)) {
    if (request->hasParam("foldername")) {
      const char *foldername = request->getParam("foldername")->value().c_str();
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      SpiRamJsonDocument json(100000); //Create a JSON document of 100 KB
      json=listFiles(json,true,foldername);
      serializeJson(json, *response);
      request->send(response);
    }
    else
    {
      request->send(400, "text/plain", "ERROR: invalid action param supplied");
    }
    } else {
      request->redirect("/loginpage");
    }
  });
  //delete it
  server->on("/api/listsettings", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (checkUserWebAuth_Admin(request)) {
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      SpiRamJsonDocument json(100000); //Create a JSON document of 100 KB
      json=listsettings(json,true);
      serializeJson(json, *response);
      request->send(response);
     }else {
      request->redirect("/loginpage");
    }

  });
  server->on("/updatesettings", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (is_authenticated(request)) {
      //update_config(config.httpuser,config.httppassword,config.webserverporthttp,config.ssid,config.wifipassword,config.ssid_local,config.wifipassword_local,config.servername,config.connect_wifi,config.create_AP);
      if (request->hasArg("httpuser") && request->hasArg("httppassword")) {
        String httppassword = request->arg("httppassword");
        String httpuser = request->arg("httpuser");
        update_config(httpuser,httppassword,config.webserverporthttp,config.ssid,config.wifipassword,config.ssid_local,config.wifipassword_local,config.servername,String(config.connect_wifi),String(config.create_AP));

      }
      else if (request->hasArg("webserverporthttp") && request->hasArg("servername")) {
        String servername = request->arg("servername");
        int webserverporthttp = request->arg("webserverporthttp").toInt();
        update_config(config.httpuser,config.httppassword,webserverporthttp,config.ssid,config.wifipassword,config.ssid_local,config.wifipassword_local,servername,String(config.connect_wifi),String(config.create_AP));

      }
      else if (request->hasArg("connect_wifi")) {

        String connect_wifi = request->arg("connect_wifi");
        if (request->hasArg("ssid") && request->hasArg("wifipassword") )
        {
          if(String(request->hasArg("ssid"))!="" && String(request->hasArg("wifipassword"))!=""){
          String ssid = request->arg("ssid");
          String wifipassword = request->arg("wifipassword");
          update_config(config.httpuser,config.httppassword,config.webserverporthttp,ssid,wifipassword,config.ssid_local,config.wifipassword_local,config.servername,connect_wifi,String(config.create_AP));
        }
        else
        {
          update_config(config.httpuser,config.httppassword,config.webserverporthttp,config.ssid,config.wifipassword,config.ssid_local,config.wifipassword_local,config.servername,connect_wifi,String(config.create_AP));
        }
        }
        else
        {
            request->redirect("/error");
        }
      }
      else if ( request->hasArg("create_AP")) {
        String create_AP = request->arg("create_AP");
        if(request->hasArg("ssid_local") && request->hasArg("wifipassword_local"))
        {
          if(String(request->arg("ssid_local"))!="" && String(request->arg("wifipassword_local"))!="")
          {
            String ssid_local = request->arg("ssid_local");
            String wifipassword_local = request->arg("wifipassword_local");
            update_config(config.httpuser,config.httppassword,config.webserverporthttp,config.ssid,config.wifipassword,ssid_local,wifipassword_local,config.servername,String(config.connect_wifi),create_AP);
          }
           else
          {
            update_config(config.httpuser,config.httppassword,config.webserverporthttp,config.ssid,config.wifipassword,config.ssid_local,config.wifipassword_local,config.servername,String(config.connect_wifi),create_AP);
          }
        }
        else
        {
            request->redirect("/error");
        }
      }
      else
      {
        request->redirect("/error");
      }
      request->redirect("/logout");
        }else {
          request->redirect("/loginpage");
        }
  });
  AsyncElegantOTA.begin(server,config.httpuser.c_str(), config.admin_httppassword.c_str());

}


void notFound(AsyncWebServerRequest *request) {
  if (is_authenticated(request)) {
      request->send(SD, "/web_app/404.html", String(), false);
    } else {
      request->redirect("/loginpage");
    }
}


// handles uploads to the filserver
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  // make sure authenticated before allowing upload
  if (is_authenticated(request)||checkUserWebAuth_Admin(request)) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    Serial.println(logmessage);
    if(filename.endsWith(".jpg"))
      filename="images/"+filename;
    if(filename.endsWith(".jpeg"))
      filename="images/"+filename;
    if(filename.endsWith(".png"))
      filename="images/"+filename;
    if(filename.endsWith(".txt"))
      filename="rendered_images/"+filename;
    if(filename.endsWith(".html")||filename.endsWith(".css")||filename.endsWith(".js"))
      filename="web_app/"+filename;
    if (!index) {
      logmessage = "Upload Start: " + String(filename);
      // open the file on first call and store the file handle in the request object
      request->_tempFile = SD.open("/" + filename, "w");
      Serial.println(logmessage);
    }
    

    if (len) {
      // stream the incoming chunk to the opened file
      request->_tempFile.write(data, len);
      logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
      Serial.println(logmessage);
    }

    if (final) {
      logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
      // close the file handle as the upload is now done
      request->_tempFile.close();
      Serial.println(logmessage);
      request->redirect("/");
    }
    
  } else {
    Serial.println("Auth: Failed");
    request->redirect("/loginpage");
  }
}
void start_vga()
{
  vga.init(vga.MODE320x200, redPins, greenPins, bluePins, hsyncPin, vsyncPin);
    //MODE320x240
    //MODE360x350
    //setting the font
    vga.setFont(Font6x8);
    vga.setTextColor(vga.RGB(0, 0, 0));
  Sprites sprites(1, spritesPixels, spritesOffsets, spritesRes, spritesPoints, spritesPointOffsets, Sprites::PixelFormat::R5G5B4A2);

  sprites.draw(vga,0, 160, 100);
  
	//show the rendering
	vga.show();
}
void init_psram()
{
  //4192123
  psramInit();
  Serial.println((String)"Memory available in PSRAM : " +ESP.getFreePsram());
  /*
  sprites_Pixels=(unsigned short**) ps_calloc(3, sizeof(unsigned short **));
  for (int i = 0; i < 3; i++)
  {
    sprites_Pixels[i]=(unsigned short*) ps_calloc(no_pixel, sizeof(unsigned short *));
  } */
  sprites_Pixels=(unsigned short*) ps_calloc(no_pixel, sizeof(unsigned short *));

}
void MDNS_int(){
  //Set your preferred server name, if you use "WI-VGA" the address would be http://WI-VGA.local/
  while (!MDNS.begin(config.servername.c_str())) 
  {          
    Serial.println(F("Error setting up MDNS responder!")); 
    //ESP.restart(); 
  }
}

void init_webserver()
{
  // configure web server
  Serial.println("Configuring Webserver ...");
  server = new AsyncWebServer(config.webserverporthttp);
  configureWebServer();
  // startup web server
  Serial.println("Starting Webserver ...");
  server->begin();
  Serial.println("HTTP server started \n\n");
}

void init_sd()
{
  spi.begin(SCK, MISO, MOSI, CS);
    // Initialise SD
    while (!SD.begin(CS,spi)) {
    Serial.println("SD initialisation failed!");
    //while (1) yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nInitialisation done.");
  Serial.println(listFiles(false));
}

void init_wifi(bool create_AP,bool connect_wifi)
  {

    if(create_AP)
    {
      Serial.print("\ncreate AP: ");
      WiFi.softAP(config.ssid_local, config.wifipassword_local);
        Serial.print("\ncreate AP: ");
        IPAddress Ip(192, 168, 1, 4);    //set IP Access Point same as gateway
        IPAddress NMask(255, 255, 255, 0);
        WiFi.softAPConfig(Ip, Ip, NMask);
    }
    else
    {
      WiFi.softAPdisconnect (true);
    }
    if(connect_wifi)
    {
      Serial.print("\nConnecting to Wifi: ");
      Serial.println(config.ssid);
      Serial.println(config.wifipassword);
      WiFi.begin(config.ssid.c_str(), config.wifipassword.c_str());
      int x=0;
      while (WiFi.status() != WL_CONNECTED && x<15) {
        delay(500);
        Serial.print(".");
        x++;
      }
      
    }
  }
  void init_config()
  {
    SpiRamJsonDocument doc(100000);
    Serial.println("Loading Configuration ...");
    // Open file for reading
  File file = SD.open("/settings.json");
  if (!file) {
    File temp = SD.open("/settings.json", FILE_WRITE);
    doc["httpuser"] = default_httpuser;
    doc["httppassword"] = sha1_(default_httppassword);
    doc["webserverporthttp"] = default_webserverporthttp;
    doc["ssid"] = default_ssid;
    doc["ssid_local"] = ssid_local;
    doc["wifipassword"] = default_wifipassword;
    doc["wifipassword_local"] = wifipassword_local;
    doc["servername"] = servername;
    doc["connect_wifi"] = default_connect_Wifi;
    doc["create_AP"] = default_create_AP;
    
    if (serializeJson(doc, temp) == 0) {
      Serial.println(F("Failed to write to file"));
    
  }
  temp.close();
  }
  
  file = SD.open("/settings.json");
  // Extract each characters by one by one
  if(file) {
    DeserializationError error = deserializeJson(doc, file);
        if (error) {
            // if the file didn't open, print an error:
            Serial.println(F("Error parsing JSON "));
            Serial.println(error.c_str());
        }
    config.admin_httpuser = default_admin_httpuser;
    config.admin_httppassword = default_admin_httppassword;   
    config.httpuser = doc["httpuser"].as<String>();
    config.httppassword = doc["httppassword"].as<String>();
    config.webserverporthttp = doc["webserverporthttp"].as<int>();
    config.ssid = doc["ssid"].as<String>();
    config.wifipassword = doc["wifipassword"].as<String>();
    config.ssid_local=doc["ssid_local"].as<String>();
    config.wifipassword_local =doc["wifipassword_local"].as<String>();
    config.servername=doc["servername"].as<String>();
    config.connect_wifi=String_To_Bool(doc["connect_wifi"].as<String>());
    config.create_AP=String_To_Bool(doc["create_AP"].as<String>()); 
    Serial.print("connect_wifi:");Serial.println(String_To_Bool(doc["connect_wifi"].as<String>()));
    Serial.print("create_AP:");Serial.println(String_To_Bool(doc["create_AP"].as<String>()));
  }
   file.close();
  // Close the file

    /*
    config.httpuser = default_httpuser;
    config.httppassword = default_httppassword;
    config.webserverporthttp = default_webserverporthttp;
    config.ssid = default_ssid;
    config.wifipassword = default_wifipassword;
    config.ssid_local=ssid_local;
    config.wifipassword_local = wifipassword_local;
    config.connect_wifi=default_connect_Wifi;
    config.create_AP=default_create_AP;
    config.servername=servername;
    */
  }

  void update_config(String httpuser,String httppassword,int webserverporthttp,String ssid,String wifipassword,String ssid_local,String wifipassword_local,String servername,String connect_Wifi,String create_AP)
  {
    SpiRamJsonDocument doc(100000);
    File temp = SD.open("/settings.json", FILE_WRITE);
    doc["httpuser"] = httpuser;
    doc["httppassword"] = sha1_(httppassword);
    doc["webserverporthttp"] = webserverporthttp;
    doc["ssid"] = ssid;
    doc["ssid_local"] = ssid_local;
    doc["wifipassword"] = wifipassword;
    doc["wifipassword_local"] = wifipassword_local;
    doc["servername"] = servername;
    doc["connect_wifi"] = connect_Wifi;
    doc["create_AP"] = create_AP;
    doc["admin_httpuser"] = default_admin_httpuser;
    doc["admin_httppassword"] = default_admin_httppassword;
    if (serializeJson(doc, temp) == 0) {
      Serial.println(F("Failed to write to file"));
    
  
  temp.close();
  }
  
  shouldReboot=true;

  }
  void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void createDir(fs::FS &fs, const char * path){
  Serial.printf("Creating Dir: %s\n", path);
  if(fs.mkdir(path)){
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

bool removeDir(fs::FS &fs, const char * path){
  Serial.printf("Removing Dir: %s\n", path);
  File root = fs.open(path);
  if(!root){
    Serial.println("Failed to open directory");
    return false;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return false;
  }
  
  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      removeDir(fs, file.name());
    } else {
      String path_temp=path;
      path_temp+="/";
      path_temp+=file.name();
      SD.remove(path_temp);
    }
    file = root.openNextFile();
  }
  if(fs.rmdir(path)){
    Serial.println("Dir removed");
    return true;
  } else {
    Serial.println("rmdir failed");
    return false;
  }
}


void renameFile(fs::FS &fs, const char * path1, const char * path2){
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char * path){
  Serial.printf("Deleting file: %s\n", path);
  if(fs.remove(path)){
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}
void check_file_structure()
{
  File root = SD.open("/images");
  if(!root.isDirectory()){
    createDir(SD,"/images");
  }
  if(!root){
    removeDir(SD,"/images");
    createDir(SD,"/images");
  }

  root = SD.open("/rendered_images");
  if(!root.isDirectory()){
    createDir(SD,"/rendered_images");
  }
  if(!root){
    removeDir(SD,"/rendered_images");
    createDir(SD,"/rendered_images");
  }

  root = SD.open("/web_app");
  if(!root.isDirectory()){
    createDir(SD,"/web_app");
  }
  if(!root){
    removeDir(SD,"/web_app");
    createDir(SD,"/web_app");
  }
}
String listDir_html(fs::FS &fs, String dirname, uint8_t levels){

  String html;

  File root = fs.open(dirname.c_str());
  if(!root){
    Serial.println("Failed to open directory");
    html+="Failed to open directory";
    return html;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    html+="empty directory";
    return html;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      html += "<tr align='left'><td>" + String(file.name()) + "</td><td>" + humanReadableSize(file.size()) + "</td>";
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      html += "<tr align='left'><td>" + String(file.name()) + "</td><td>" + humanReadableSize(file.size()) + "</td>";
      html += "<td><button onclick=\"downloadDeleteButton(\'" +String(dirname)+'/'+ String(file.name()) + "\', \'download\')\">Download</button>";
      html += "<td><button onclick=\"downloadDeleteButton(\'" +String(dirname)+'/'+ String(file.name()) + "\', \'delete\')\">Delete</button></tr>";
    }
    file = root.openNextFile();
  }
  return html;

}

String sha1_(String payloadStr){
const char *payload = payloadStr.c_str();
int size = 20;
byte shaResult[size];
mbedtls_md_context_t ctx;
mbedtls_md_type_t md_type = MBEDTLS_MD_SHA1;
const size_t payloadLength = strlen(payload);
mbedtls_md_init(&ctx);
mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
mbedtls_md_starts(&ctx);
mbedtls_md_update(&ctx, (const unsigned char *) payload, payloadLength);
mbedtls_md_finish(&ctx, shaResult);
mbedtls_md_free(&ctx);
String hashStr = "";
for(uint16_t i = 0; i < size; i++) {
String hex = String(shaResult[i], HEX);
if(hex.length() < 2) {
hex = "0" + hex;
}
hashStr += hex;
}
return hashStr;
}

void handleLogin(AsyncWebServerRequest *request) {
Serial.println("Handle login");
String msg;
if(!user){
  if (request->hasHeader("Cookie")) {
  // Print cookies
  Serial.print("Found cookie: ");
  String cookie = request->header("Cookie");
  Serial.println(cookie);
  }
  if (request->hasArg("username") && request->hasArg("password")) {
  Serial.print("Found parameter: ");
  if (request->arg("username") == String(config.httpuser) && sha1_(request->arg("password")) ==String(config.httppassword)) {
  AsyncWebServerResponse *response = request->beginResponse(301); //Sends 301 redirect
  response->addHeader("Location", "/");
  response->addHeader("Cache-Control", "no-cache");
  // set current time to calculate expiration
  tokenCreationTime =millis();
  String token = sha1_(String(config.httpuser) + ":" +String(config.httppassword) + ":" +request->client()->remoteIP().toString()+":"+String(tokenCreationTime));
  Serial.print("Token: ");
  Serial.println(token);
  response->addHeader("Set-Cookie", "ESPSESSIONID=" + token);
  //loading image
  loading=true;
  log("true");
  user=true;
  request->send(response);
  Serial.println("Log in Successful");

  return;
}

msg = "Wrong username/password! try again.";
Serial.println("Log in Failed");
AsyncWebServerResponse *response = request->beginResponse(301); //Sends 301 redirect
response->addHeader("Location", "/login.html?msg=" + msg);
response->addHeader("Cache-Control", "no-cache");
request->send(response);
return;
}
}
else
{
  log("login.html?msg=ONLY SINGLE USER ALLOWED");
  AsyncWebServerResponse *response = request->beginResponse(301); //Sends 301 redirect
  response->addHeader("Location", "/login.html?msg=ONLY SINGLE USER ALLOWED");
  response->addHeader("Cache-Control", "no-cache");
  request->send(response);
  return;
}
}
/**
* Manage logout (simply remove correct token and redirect to login form)
*/
void handleLogout(AsyncWebServerRequest *request) {
Serial.println("Disconnection");
AsyncWebServerResponse *response = request->beginResponse(301); //Sends 301 redirect
response->addHeader("Location", "/loginpage");
response->addHeader("Cache-Control", "no-cache");
response->addHeader("Set-Cookie", "ESPSESSIONID=0");
request->send(response);
user=false;
loading=false;
return;
}
//Check if header is present and correct
bool is_authenticated(AsyncWebServerRequest *request) {
  Serial.println("Enter is_authenticated");

  //Check if the token has expired
  if(millis()<=tokenCreationTime + (30 * 60 * 1000))
  {
    log("check cookies token");
    if (request->hasHeader("Cookie")) {
      Serial.print("Found cookie: ");
      String cookie = request->header("Cookie");
      Serial.println(cookie);
      String token = sha1_(String(config.httpuser) + ":" +String(config.httppassword) + ":" +request->client()->remoteIP().toString()+":"+String(tokenCreationTime));
      //  token = sha1_(token);
      log(token);
      if (cookie.indexOf("ESPSESSIONID=" + token) != -1) {
        Serial.println("Authentication Successful");
        return true;
      }
      else
        log("Authentication Failed_token problem");
        return false;
      }
  }
  else
    handleLogout(request);

  log("Authentication Failed_End or token time out");
  Serial.println("Authentication Failed");
  return false;
}

bool checkUserWebAuth_Admin(AsyncWebServerRequest * request) {
  bool isAuthenticated = false;

  if (request->authenticate(config.admin_httpuser.c_str(), config.admin_httppassword.c_str())) {
    Serial.println("is authenticated via username and password");
    isAuthenticated = true;
  }
  return isAuthenticated;
}


void log(String log)
{
  logfile.println(log);
  logfile.flush();
}
