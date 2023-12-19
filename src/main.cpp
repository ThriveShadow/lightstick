#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <modes.h>

#define LED_PIN D7
#define NUM_LEDS 7
const int buttonPin = D8;

CRGB leds[NUM_LEDS];

lightstick light;

AsyncWebServer server(80);

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

int prevbuttonState = LOW;
void mainCode()
{
  // Debugging purposes, prints all files in LittleFS
  Dir dir = LittleFS.openDir("/");
  while (dir.next())
  {
    Serial.print(dir.fileName());
    Serial.print(" - ");
    Serial.println(dir.fileSize());
  }

  // Check for OTA updates
  if (LittleFS.exists("/update.bin"))
  {
    Serial.println("Update file found");
    File updateBin = LittleFS.open("/update.bin", "r");
    if (updateBin)
    {
      Serial.println("Update file opened");
      if (Update.begin(updateBin.size()))
      {
        Serial.println("Update started");
        size_t written = Update.writeStream(updateBin);
        if (written == updateBin.size())
        {
          Serial.println("Written : " + String(written) + " successfully");
        }
        else
        {
          Serial.println("Written only : " + String(written) + "/" + String(updateBin.size()) + ". Retry?");
        }
        if (Update.end())
        {
          Serial.println("OTA done!");
          if (Update.isFinished())
          {
            // cleanup, delete update file from LittleFS
            LittleFS.remove("/update.bin");
            Serial.println("Update successfully completed. Rebooting.");
            ESP.restart();
          }
          else
          {
            Serial.println("Update not finished? Something went wrong!");
          }
        }
        else
        {
          Serial.println("Error Occurred. Error #: " + String(Update.getError()));
        }
      }
      else
      {
        Serial.println("Not enough space to begin OTA");
      }
      updateBin.close();
      Serial.println("File closed");
      LittleFS.remove("/update.bin");
      Serial.println("Update file removed");
    }
    else
    {
      Serial.println("Failed to open file for reading");
    }
  }

  // Check for config file, parse it, and apply the presets
  if (LittleFS.exists("/config.json"))
  {
    File configFile = LittleFS.open("/config.json", "r");
    if (configFile)
    {
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, configFile);
      if (error)
      {
        Serial.println("Failed to read file, using default configuration");
      }
      else
      {
        Serial.println("\nConfig file content:");
        serializeJson(doc, Serial);
        Serial.println();
        JsonArray presets = doc["ledPresets"];
        for (int i = 0; i < presets.size(); i)
        {
          JsonObject preset = presets[i];
          String name = preset["name"].as<String>();
          String color = preset["hex"].as<String>();
          String mode = preset["mode"].as<String>();
          int speed = preset["speed"].as<int>();

          uint8_t red = (uint8_t)strtol(color.substring(1, 3).c_str(), NULL, 16);
          uint8_t green = (uint8_t)strtol(color.substring(3, 5).c_str(), NULL, 16);
          uint8_t blue = (uint8_t)strtol(color.substring(5, 7).c_str(), NULL, 16);

          int buttonState = digitalRead(buttonPin);
          int prevButtonState;

          if (i == 0 || buttonState == HIGH && prevbuttonState == LOW)
          {
            FastLED.clear();
            if (mode == "solid" && prevbuttonState == LOW)
            {
              light.Solid(leds, NUM_LEDS, CRGB(red, green, blue));
            }
            else if (mode == "rainbow")
            {
              light.Rainbow(leds, NUM_LEDS, speed);
              prevButtonState = LOW;
            }
            else if (mode == "blink")
            {
              light.Blink(leds, NUM_LEDS, speed, CRGB(red, green, blue));
              prevButtonState = LOW;
            }
            else if (mode == "chaseup")
            {
              light.ChaseUp(leds, NUM_LEDS, speed, CRGB(red, green, blue));
              prevButtonState = LOW;
            }
          }
          Serial.println("If the button is not pressed, it should be stuck here");

          buttonState = digitalRead(buttonPin);

          if (buttonState == HIGH && prevButtonState == LOW)
          {
            i++;
          }
          prevButtonState = buttonState;
          if (WiFi.softAPgetStationNum() != 0)
          {
            break;
          }
        }
      }
    }
  }
  else
  {
    Serial.println("Config file doesn't exist");
  }
}

// Handle OTA upload, save binary file to LittleFS
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  static fs::File fsUploadFile;
  static const String uploadFilename = "/update.bin"; // Change this to your desired filename

  if (!index)
  {
    fsUploadFile = LittleFS.open(uploadFilename, "w");
    if (!fsUploadFile)
    {
      Serial.println("Failed to open file for writing");
      request->send(500, "text/plain", "Failed to open file for writing");
      return;
    }
    Serial.printf("UploadStart: %s\n", filename.c_str());
  }

  if (len)
  {
    if (fsUploadFile.write(data, len) != len)
    {
      Serial.println("Write error in file");
      fsUploadFile.close();
      request->send(500, "text/plain", "Write error in file");
      return;
    }
  }

  if (final)
  {
    fsUploadFile.close();
    Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index + len);
    request->send(200, "text/plain", "File upload completed");
  }
}

void setup()
{
  Serial.begin(115200);
  if (!LittleFS.begin())
  {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }
  pinMode(buttonPin, OUTPUT);
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);

  WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));

  if (LittleFS.exists("/config.json"))
  {
    File configFile = LittleFS.open("/config.json", "r");
    if (configFile)
    {
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, configFile);
      if (error)
      {
        Serial.println("Failed to read file, using default configuration");
      }
      else
      {
        Serial.println("\nConfig file content:");
        serializeJson(doc, Serial);
        Serial.println();
        const char *ssid = doc["ssid"];
        const char *password = doc["password"];
        WiFi.softAP(ssid, password);
      }
    }
  }
  else
  {
    Serial.println("Config file doesn't exist");
  }

  File htmlFile = LittleFS.open("/index.html", "r");
  if (!htmlFile)
  {
    Serial.println("Failed to open file for reading");
    return;
  }
  String index_html;
  while (htmlFile.available())
  {
    index_html += char(htmlFile.read());
  }
  htmlFile.close();

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [index_html](AsyncWebServerRequest *request)
            { request->send(200, "text/html", index_html); });

  server.on("/wifi", HTTP_GET, [index_html](AsyncWebServerRequest *request)
            {
    String ssid;
    String password;

        if (request->hasParam("ssid") && request->hasParam("password"))
        {
          ssid = request->getParam("ssid")->value();
          password = request->getParam("password")->value();

          if (ssid.isEmpty() || password.isEmpty())
          {
            request->send(400, "text/plain", "400: SSID or password cannot be empty");
          } else {
            request->send(200, "text/html", index_html);
            if (LittleFS.exists("/config.json"))
            {
              File configFile = LittleFS.open("/config.json", "r");
              if (configFile)
              {
                DynamicJsonDocument doc(1024);
                DeserializationError error = deserializeJson(doc, configFile);
                if (error)
                {
                  Serial.println("Failed to read file, using default configuration");
                }
                else
                {
                  Serial.println("\nConfig file content:");
                  serializeJson(doc, Serial);
                  Serial.println();
                  doc["ssid"] = ssid;
                  doc["password"] = password;
                  File configFile = LittleFS.open("/config.json", "w");
                  if (!configFile)
                  {
                    Serial.println("Failed to open config file for writing");
                  }
                  JsonObject wifi = doc["wifi"];
                  wifi["ssid"] = ssid;
                  wifi["password"] = password;

                  serializeJson(doc, configFile);
                  configFile.close();
                  ESP.restart();
                }
              }
            }
            else
            {
              Serial.println("Config file doesn't exist");
            }
          }}
        else
        {
          request->send(400, "text/plain", "400: Invalid Request");
        } });

  // save client request to LittleFS
  server.on("/get", HTTP_GET, [index_html](AsyncWebServerRequest *request)
            {
              String inputMessage;
              String inputParam;
              String name;
              String color;
              String mode;
              int speed;

              if (request->hasParam("name") && request->hasParam("color") && request->hasParam("mode") && request->hasParam("speed"))
              {
                inputMessage = request->getParam("name")->value() + "," + request->getParam("color")->value() + "," + request->getParam("mode")->value();
                inputParam = "preset";
                name = request->getParam("name")->value();
                color = request->getParam("color")->value();
                mode = request->getParam("mode")->value();
                speed = request->getParam("speed")->value().toInt();
              }
              else
              {
                inputMessage = "No message sent";
                inputParam = "none";
              }
              request->send(200, "text/html", index_html);
              Serial.println(inputMessage);
              uint8_t red = (uint8_t)strtol(color.substring(1, 3).c_str(), NULL, 16);
              uint8_t green = (uint8_t)strtol(color.substring(3, 5).c_str(), NULL, 16);
              uint8_t blue = (uint8_t)strtol(color.substring(5, 7).c_str(), NULL, 16);
              Serial.println(red);
              Serial.println(green);
              Serial.println(blue);
              FastLED.clear();
              if(mode == "solid")
              {
                light.Solid(leds, NUM_LEDS, CRGB(red, green, blue));
              }
              else if(mode == "rainbow")
              {
                light.Rainbow(leds, NUM_LEDS, speed);
              }
              else if(mode == "blink")
              {
                light.Blink(leds, NUM_LEDS, speed, CRGB(red, green, blue));
              }
              else if(mode == "chaseup")
              {
                light.ChaseUp(leds, NUM_LEDS, speed, CRGB(red, green, blue));
              }
              

  if (LittleFS.exists("/config.json"))
  {
    File configFile = LittleFS.open("/config.json", "r");
    if (configFile)
    {
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, configFile);
      if (error)
      {
        Serial.println("Failed to read file, using default configuration");
      }
      else
      {
        Serial.println("\nConfig file content:");
        serializeJson(doc, Serial);
        Serial.println();
        JsonObject newPreset = doc["ledPresets"].createNestedObject();
        newPreset["name"] = name;
        newPreset["hex"] = color;
        newPreset["mode"] = mode;
        newPreset["speed"] = speed;
        File configFile = LittleFS.open("/config.json", "w");
        if (!configFile)
        {
          Serial.println("Failed to open config file for writing");
        }
        serializeJson(doc, configFile);
        configFile.close();
      }
    }
  }
  else
  {
    Serial.println("Config file doesn't exist");
  } });

  server.on(
      "/update", HTTP_POST, [](AsyncWebServerRequest *request)
      { request->send(200); },
      handleUpload);

  server.on("/delete", HTTP_GET, [index_html](AsyncWebServerRequest *request)
            {
              String name;
              if (request->hasParam("name"))
              {
                name = request->getParam("name")->value();
              }
              else
              {
                name = "No name sent";
              }
              if (LittleFS.exists("/config.json"))
              {
                File configFile = LittleFS.open("/config.json", "r");
                if (configFile)
                {
                  DynamicJsonDocument doc(1024);
                  DeserializationError error = deserializeJson(doc, configFile);
                  if (error)
                  {
                    Serial.println("Failed to read file, using default configuration");
                  }
                  else
                  {
                    Serial.println("\nConfig file content:");
                    serializeJson(doc, Serial);
                    Serial.println();
                    JsonArray presets = doc["ledPresets"];
                    for (int i = 0; i < presets.size(); i++)
                    {
                      JsonObject preset = presets[i];
                      if (preset["name"] == name)
                      {
                        presets.remove(i);
                        break;
                      }
                    }
                    File configFile = LittleFS.open("/config.json", "w");
                    if (!configFile)
                    {
                      Serial.println("Failed to open config file for writing");
                    }
                    serializeJson(doc, configFile);
                    configFile.close();
                  }
                }
                request->send(200, "text/html", index_html);
              }
              else
              {
                Serial.println("Config file doesn't exist");
              } });
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    if (LittleFS.exists("/config.json"))
    {
      File configFile = LittleFS.open("/config.json", "r");
      if (configFile)
      {
        request->send(200, "application/json", configFile.readString());
        configFile.close();
      }
      else
      {
        Serial.println("Config file doesn't exist");
        request->send(404, "text/plain", "Config file doesn't exist");
      }
    }
    else
    {
      Serial.println("Config file doesn't exist");
      request->send(404, "text/plain", "Config file doesn't exist");
    } });
  server.onNotFound(notFound);
  server.begin();
}

void loop()
{
  int i = 0;
  while (WiFi.softAPgetStationNum() == 0)
  {
    Serial.println("Waiting for device to connect to WiFi..");

    fill_solid(leds, NUM_LEDS, CRGB::Black); // Clear the LED buffer

    mainCode();
  }
}