/* ***********************************************************************
 *  This is used to configure and establish a WLAN-connection
 *  It works the following way:
 *  - Loading cerdentials from flash
 *       - ssid
 *       - password
 *       - ntpserver
 *  - Try to establish a connection with these credentials
 *  - If there is noch connection possible, following will happen:
 *      - a change in AP-mode (NOT rebooting!!!) 
 *      - user calls http://192.168.4.1
 *      - displays HTML-code from wlanconf.h with filled in credentials
 *      - user puts correct ceredentials in it
 *      - when sending the form by submit, the credentials are updated in flash (ONLÝ THEN*)
 *      - reboots in station-mode and begins the next try to connect with new credentials
 *  => This happens infinite, until connection can be established
 *      
 *  It is used bei call of "makeWLAN()" (usaly in 'setup()')
 *  It returns "true" when connection successful
 *  
 *  Libraries which are used and included here (no need to include them otherwhere for this use)
 *    - Wifi.h
 *    - Preferences.h
 *    - ESPAsyncWebServer.h
 *  
 *  The WLAN-instance is called "WiFi"
 *  The webserver-instance (only when in AP-mode) is called "server"
 *  
 *  When TESTMODUS is true, after a successful connection a wrong password is stored in flash. So
 *  it won't connect after next reboot and starts the procedure from the beginning.
 *  
 *  *: When your WLAN is offline and that's why this device asks for new data, you can ignore it
 *     and reboot this device, when your WLAN is right back - the credentials won't change until
 *     you submit the form with new ones.
 */

#include <WiFi.h>
#include <Preferences.h>
#include "ESPAsyncWebServer.h"

#include "wlanconf.h"  //HTML-code for WLAN-setup

#define TESTMODUS false

// Webserver on port 80
AsyncWebServer server(80);

// WLAN credentials
Preferences preferences;
String ssid;
String password;
String ntpserver;
int    bootAsAP;
#define MAXWLANTRY 10  // try to connect with stored credentials MAXWLANTRY times, before switching
                       // to AP-mode and asking for new credentials

// ### Usally should be called in 'setup()'
int makeWLAN()
{
  preferences.begin("credentials", false);

  ssid = preferences.getString("ssid", ""); 
  password = preferences.getString("password", "");
  ntpserver = preferences.getString("ntpserver", "at.pool.ntp.org");
    
  delay(250);
  Serial.println(F("ESP-Webradio"));
  Serial.println(ssid);

  int tryCount = 0;
  if (ssid == "" || password == ""){
    Serial.println("No WiFi login data saved!");
    ssid = "blub"; password = "blah";
    tryCount = MAXWLANTRY;
  }

  if (tryCount < MAXWLANTRY) {
    // Mit Wi-Fi verbinden
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.print("Connect to WiFi ...");
    while ((WiFi.status() != WL_CONNECTED) && (tryCount<MAXWLANTRY)) {
      Serial.print('.');
      delay(1000);
      tryCount++;
    }
    IPAddress lip(0,0,0,0);
    if (WiFi.localIP() == lip) {
      tryCount = MAXWLANTRY;
    }
  }

  if (tryCount < MAXWLANTRY) {
    Serial.print(F("IP address via DHCP is "));
    Serial.println(WiFi.localIP());  
    // Für den Automatik-Test
    if (TESTMODUS) {
      preferences.putString("password", "XXX"); 
    }
//    isWiFiConnected = true;
    return(true);   // when connection ok, stop here and return positive
  }

  // when no connection possible, start the configuration website in AP-mode 
  makeConfigAP();

  return(false);    // will never be reached, because 'makeConfigAP()' is infinite or
                    // reboots when user entered new credentilas
}


// ### Builds an own WLAN "WEBRADIO" and serves under http://192.168.4.1 a webinterface
// This function is infinite, until new credentials ar given - in this case it reboots
void makeConfigAP() {

  Serial.print("Create AP (Access Point)…");
  WiFi.mode(WIFI_AP);
  delay(500);  
  WiFi.softAP("WEBRADIO");

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  display.clearDisplay();
  display.println("WiFi AP mode");
  //display.drawLine(0,14,128,14,1);
  display.println("");
  display.println("Connect AP \"WEBRADIO\"");
  display.println("");
  display.println("Enter WiFi parameters");
  display.println("on http://192.168.4.1");
  display.display();
  
  server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request){

    int paramsNr = request->params();
    Serial.println(paramsNr);
    String ssid = "";
    String password = "";
    int paramCountOk = 0;
    int gotAnswer = false;
  
    for(int i=0;i<paramsNr;i++){
  
      AsyncWebParameter* p = request->getParam(i);
      Serial.print("Param name: ");
      Serial.println(p->name());
      Serial.print("Param value: ");
      Serial.println(p->value());
      Serial.println("------");
      if (p->name() == "ssid") {
        ssid = p->value();
        paramCountOk++;
      }
      if (p->name() == "password") {
        password = p->value();
        paramCountOk++;
      }
      // is used for my clock-projects :) and
      // here commented out in HTML-code.
      if (p->name() == "ntpserver") {   
        ntpserver = p->value();
        paramCountOk++;
      }

      if (paramCountOk >= paramsNr) {
        preferences.putString("ssid", ssid); 
        preferences.putString("password", password); 
        preferences.putString("ntpserver", ntpserver); 
        preferences.putInt("bootAP", 0); 
        preferences.end();
        gotAnswer = true;
      }
    }
  
    String s = MAIN_page; //Read HTML contents
    if (gotAnswer) {
      s.replace("*mark1begin*", "<!--");
      s.replace("*mark1end*", "-->");
      s.replace("*feedback*", "<div style='color:#CC0000'><h1>Connection is attempted...</h1><h2>If the radio does not start, please reconnect to the WLAN WEBRADIO in 20 seconds and reopen this page.</h2></div>");
    } else {
      s.replace("*mark1begin*", "");
      s.replace("*mark1end*", "");
      s.replace("*feedback*", "");
      s.replace("*ssid*", preferences.getString("ssid", ""));
      s.replace("*password*", preferences.getString("password", ""));
      s.replace("*ntpserver*", preferences.getString("ntpserver", ""));
    }
    request->send(200, "text/html", s);
    if (gotAnswer) {
      delay(3000);
      ESP.restart();
    }

  }); 
  server.begin();        //Start server
  Serial.println("HTTP server started");

  serverLoop();
}

// ### Here a simple loop to display status on serial, the webserver runs asynchronous
void serverLoop() {
  while (true) {
    delay(5000);
    Serial.println("Http server is running...");
  }
}
