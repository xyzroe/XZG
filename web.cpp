#include <Arduino.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <WebServer.h>
#include "FS.h"
#include "LITTLEFS.h"
#include "web.h"
#include "config.h"
#include "log.h"

extern struct ConfigSettingsStruct ConfigSettings;
extern unsigned long timeLog;



WebServer serverWeb(80);

void webServerHandleClient()
{
    serverWeb.handleClient();
}



const char HTTP_HEADER[] PROGMEM = 
  "<head>"
  "<script type='text/javascript' src='web/js/jquery-min.js'></script>"
  "<script type='text/javascript' src='web/js/bootstrap.min.js'></script>"
  "<script type='text/javascript' src='web/js/functions.js'></script>"
  "<link href='web/css/bootstrap.min.css' rel='stylesheet' type='text/css' />"
  "<link href='web/css/style.css' rel='stylesheet' type='text/css' />"
  " </head>"
    "<body>"
  "<nav class='navbar navbar-expand-lg navbar-light bg-light rounded'><a class='navbar-brand' href='/'><img src='web/img/logo.png'/> <strong>Config </strong>"
   VERSION
  "</a>"
  "<button class='navbar-toggler' type='button' data-toggle='collapse' data-target='#navbarNavDropdown' aria-controls='navbarNavDropdown' aria-expanded='false' aria-label='Toggle navigation'>"
  "<span class='navbar-toggler-icon'></span>"
  "</button>"
  "<div id='navbarNavDropdown' class='collapse navbar-collapse justify-content-md-center'>"
  "<ul class='navbar-nav'>"
    "<li class='nav-item'>"
      "<a class='nav-link' href='/'>Status</a>"     
    "</li>"
    "<li class='nav-item dropdown'>"
      "<a class='nav-link dropdown-toggle' href='#' id='navbarDropdown' role='button' data-toggle='dropdown' aria-haspopup='true' aria-expanded='false'>Config</a>"
      "<div class='dropdown-menu' aria-labelledby='navbarDropdown'>"         
          "<a class='dropdown-item' href='/ethernet'>Ethernet</a>"
          "<a class='dropdown-item' href='/wifi'>WiFi</a>"
      "</div>"
    "</li>"
     "<li class='nav-item'>"
      "<a class='nav-link' href='/tools'>Tools</a>"
      
    "</li>"
    "<li class='nav-item'>"
      "<a class='nav-link' href='#'>Help</a>"
    "</li>"
  "</ul></div>"
  "</nav>";


const char HTTP_WIFI[] PROGMEM = 
 "<h1>Config WiFi</h1>"
  "<div class='row justify-content-md-center' >"
    "<div class='col-sm-6'><form method='POST' action='saveWifi'>"
  "<div class='form-check'>"
    
    "<input class='form-check-input' id='wifiEnable' type='checkbox' name='wifiEnable' {{checkedWiFi}}>"
    "<label class='form-check-label' for='wifiEnable'>Enable</label>"
  "</div>"
  "<div class='form-group'>"
    "<label for='ssid'>SSID</label>"
    "<input class='form-control' id='ssid' type='text' name='WIFISSID' value='{{ssid}}'> <a onclick='scanNetwork();' class='btn btn-primary mb-2'>Scan</a><div id='networks'></div>"
  "</div>"
   "<div class='form-group'>"
    "<label for='pass'>Password</label>"
    "<input class='form-control' id='pass' type='password' name='WIFIpassword' value=''>"
  "</div>"
   "<div class='form-group'>"
    "<label for='ip'>@IP</label>"
  "<input class='form-control' id='ip' type='text' name='ipAddress' value='{{ip}}'>"
  "</div>"
   "<div class='form-group'>"
    "<label for='mask'>@Mask</label>"
  "<input class='form-control' id='mask' type='text' name='ipMask' value='{{mask}}'>"
  "</div>"
   "<div class='form-group'>"
    "<label for='gateway'>@Gateway</label>"
  "<input type='text' class='form-control' id='gateway' name='ipGW' value='{{gw}}'>"
  "</div>"
   "Server Port : <br>{{port}}<br><br>"
  "<button type='submit' class='btn btn-primary mb-2'name='save'>Save</button>"
  "</form>";

const char HTTP_ETHERNET[] PROGMEM = 
 "<h1>Config Ethernet</h1>"
  "<div class='row justify-content-md-center' >"
    "<div class='col-sm-6'><form method='POST' action='saveEther'>"
    "<div class='form-check'>"
    
    "<input class='form-check-input' id='dhcp' type='checkbox' name='dhcp' {{modeEther}}>"
    "<label class='form-check-label' for='dhcp'>DHCP</label>"
  "</div>"
   "<div class='form-group'>"
    "<label for='ip'>@IP</label>"
  "<input class='form-control' id='ip' type='text' name='ipAddress' value='{{ipEther}}'>"
  "</div>"
   "<div class='form-group'>"
    "<label for='mask'>@Mask</label>"
  "<input class='form-control' id='mask' type='text' name='ipMask' value='{{maskEther}}'>"
  "</div>"
   "<div class='form-group'>"
    "<label for='gateway'>@Gateway</label>"
  "<input type='text' class='form-control' id='gateway' name='ipGW' value='{{GWEther}}'>"
  "</div>"
   "Server Port : <br>{{port}}<br><br>"
  "<button type='submit' class='btn btn-primary mb-2'name='save'>Save</button>"
  "</form>";


const char HTTP_ROOT[] PROGMEM = 
 "<h1>Status</h1>"
  "<div class='row'>"
      "<div class='col-sm-6'>"
        "<div class='card'>"
          "<div class='card-header'>Ethernet</div>"
          "<div class='card-body'>"
            "<div id='ethConfig'>"
              "<strong>Connected : </strong>{{connectedEther}}"
              "<br><strong>Mode : </strong>{{modeEther}}"
              "<br><strong>@IP : </strong>{{ipEther}}"
              "<br><strong>@Mask : </strong>{{maskEther}}"
              "<br><strong>@GW : </strong>{{GWEther}}"
            "</div>"
          "</div>"
        "</div>"
      "</div>"
  "</div>"
 "<div class='row'>"
      "<div class='col-sm-6'>"
        "<div class='card'>"
            "<div class='card-header'>Wifi</div>"
            "<div class='card-body'>"
              "<div id='wifiConfig'>"
                "<strong>Enable : </strong>{{enableWifi}}"
                "<br><strong>SSID : </strong>{{ssidWifi}}"
                "<br><strong>@IP : </strong>{{ipWifi}}"
                "<br><strong>@Mask : </strong>{{maskWifi}}"
                "<br><strong>@GW : </strong>{{GWWifi}}"
              "</div>"
            "</div>"
        "</div>"
     "</div>"
  "</div>"
 "<div class='row'>"
      "<div class='col-sm-6'>"
        "<div class='card'>"
            "<div class='card-header'>ZiGate</div>"
            "<div class='card-body'>"
              "<div id='ZiGateConfig'>"
                "<strong>Version : </strong>{{ZiGateVersion}}"
              "</div>"
            "</div>"
        "</div>"
     "</div>"
  "</div>"
 
 
 
 ;

void initWebServer()
{
  serverWeb.serveStatic("/web/js/jquery-min.js", LITTLEFS, "/web/js/jquery-min.js");
  serverWeb.serveStatic("/web/js/functions.js", LITTLEFS, "/web/js/functions.js");
  serverWeb.serveStatic("/web/js/bootstrap.min.js", LITTLEFS, "/web/js/bootstrap.min.js");
  serverWeb.serveStatic("/web/js/bootstrap.min.js.map", LITTLEFS, "/web/js/bootstrap.min.js.map");
  serverWeb.serveStatic("/web/css/bootstrap.min.css", LITTLEFS, "/web/css/bootstrap.min.css");
  serverWeb.serveStatic("/web/css/style.css", LITTLEFS, "/web/css/style.css");
  serverWeb.serveStatic("/web/img/logo.png", LITTLEFS, "/web/img/logo.png");
  serverWeb.serveStatic("/web/img/wait.gif", LITTLEFS, "/web/img/wait.gif");
  serverWeb.serveStatic("/web/img/nok.png", LITTLEFS, "/web/img/nok.png");
  serverWeb.serveStatic("/web/img/ok.png", LITTLEFS, "/web/img/ok.png");
  serverWeb.serveStatic("/web/img/", LITTLEFS, "/web/img/");
  serverWeb.on("/", handleRoot);
  serverWeb.on("/wifi", handleWifi);
  serverWeb.on("/ethernet", handleEther);
  serverWeb.on("/saveWifi", HTTP_POST, handleSaveWifi);
  serverWeb.on("/saveEther", HTTP_POST, handleSaveEther);
  serverWeb.on("/tools", handleTools);
  serverWeb.on("/logs", handleLogs);
  serverWeb.on("/reboot", handleReboot);
  serverWeb.on("/update", handleUpdate);
  serverWeb.on("/readFile", handleReadfile);
  serverWeb.on("/getLogBuffer", handleLogBuffer);
  serverWeb.on("/scanNetwork", handleScanNetwork);
  serverWeb.on("/cmdClearConsole", handleClearConsole);
  serverWeb.on("/cmdGetVersion", handleGetVersion);
  serverWeb.onNotFound(handleNotFound);
  serverWeb.begin();
}

void handleNotFound() {

  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += serverWeb.uri();
  message += F("\nMethod: ");
  message += (serverWeb.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += serverWeb.args();
  message += F("\n");

  for (uint8_t i = 0; i < serverWeb.args(); i++) {
    message += " " + serverWeb.argName(i) + ": " + serverWeb.arg(i) + "\n";
  }

  serverWeb.send(404, F("text/plain"), message);

}

void handleWifi() {
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += FPSTR(HTTP_WIFI);
  result += F("</html>");
   
  result.replace("{{version}}","1.4");
  DEBUG_PRINTLN(ConfigSettings.enableWiFi);
  if (ConfigSettings.enableWiFi)
  {
    
    result.replace("{{checkedWiFi}}","Checked");
  }else{
    result.replace("{{checkedWiFi}}","");
  } 
  result.replace("{{ssid}}",String(ConfigSettings.ssid));
  result.replace("{{ip}}",ConfigSettings.ipAddressWiFi);
  result.replace("{{mask}}",ConfigSettings.ipMaskWiFi);
  result.replace("{{gw}}",ConfigSettings.ipGWWiFi);
  result.replace("{{port}}",String(ConfigSettings.tcpListenPort));


  serverWeb.send(200,"text/html", result);
  
}

void handleEther() {
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += FPSTR(HTTP_ETHERNET);
  result += F("</html>");
   
  if (ConfigSettings.dhcp)
  {
     result.replace("{{modeEther}}","Checked");
  }else{
     result.replace("{{modeEther}}","");
  }
  result.replace("{{ipEther}}",ConfigSettings.ipAddress);
  result.replace("{{maskEther}}",ConfigSettings.ipMask);
  result.replace("{{GWEther}}",ConfigSettings.ipGW);
  result.replace("{{port}}",String(ConfigSettings.tcpListenPort));
  
  serverWeb.send(200,"text/html", result);
  
}


void handleRoot() {
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += FPSTR(HTTP_ROOT);
  result += F("</html>");

  if (ConfigSettings.enableWiFi)
  {
    result.replace("{{enableWifi}}","<img src='/web/img/ok.png'>");
  }else{
    result.replace("{{enableWifi}}","<img src='/web/img/nok.png'>");
  } 
  result.replace("{{ssidWifi}}",String(ConfigSettings.ssid));
  result.replace("{{ipWifi}}",ConfigSettings.ipAddressWiFi);
  result.replace("{{maskWifi}}",ConfigSettings.ipMaskWiFi);
  result.replace("{{GWWifi}}",ConfigSettings.ipGWWiFi);

  if (ConfigSettings.dhcp)
  {
     result.replace("{{modeEther}}","DHCP");
  }else{
     result.replace("{{modeEther}}","STATIC");
  }
  if (ConfigSettings.connectedEther)
  {
    result.replace("{{connectedEther}}","<img src='/web/img/ok.png'>");
  }else{
    result.replace("{{connectedEther}}","<img src='/web/img/nok.png'>");
  }
  result.replace("{{ipEther}}",ConfigSettings.ipAddress);
  result.replace("{{maskEther}}",ConfigSettings.ipMask);
  result.replace("{{GWEther}}",ConfigSettings.ipGW);

  serverWeb.send(200,"text/html", result);
  
}

void handleSaveWifi()
{
  if(!serverWeb.hasArg("WIFISSID")) {serverWeb.send(500, "text/plain", "BAD ARGS"); return;}

   String StringConfig;
   String enableWiFi;
    if (serverWeb.arg("wifiEnable")=="on")
    {
      enableWiFi="1";
    }else{
      enableWiFi="0";
    }
    String ssid = serverWeb.arg("WIFISSID");
    String pass = serverWeb.arg("WIFIpassword");
    String ipAddress = serverWeb.arg("ipAddress");
    String ipMask = serverWeb.arg("ipMask");
    String ipGW = serverWeb.arg("ipGW");
    String tcpListenPort = serverWeb.arg("tcpListenPort");

    const char * path = "/config/config.json";

   StringConfig = "{\"enableWiFi\":"+enableWiFi+",\"ssid\":\""+ssid+"\",\"pass\":\""+pass+"\",\"ip\":\""+ipAddress+"\",\"mask\":\""+ipMask+"\",\"gw\":\""+ipGW+"\",\"tcpListenPort\":\""+tcpListenPort+"\"}";    
   StaticJsonDocument<512> jsonBuffer;
   DynamicJsonDocument doc(1024);
   deserializeJson(doc, StringConfig);
   
   File configFile = LITTLEFS.open(path, FILE_WRITE);
   if (!configFile) {
    DEBUG_PRINTLN(F("failed open"));
   }else{
     serializeJson(doc, configFile);
   }
  serverWeb.send(200, "text/html", "Save config OK ! <br><form method='GET' action='reboot'><input type='submit' name='reboot' value='Reboot'></form>");
  
 
}

void handleSaveEther()
{
  if(!serverWeb.hasArg("ipAddress")) {serverWeb.send(500, "text/plain", "BAD ARGS"); return;}

   String StringConfig;
   String dhcp;
    if (serverWeb.arg("dhcp")=="on")
    {
      dhcp="1";
    }else{
      dhcp="0";
    }
    String ipAddress = serverWeb.arg("ipAddress");
    String ipMask = serverWeb.arg("ipMask");
    String ipGW = serverWeb.arg("ipGW");

    const char * path = "/config/configEther.json";

    
   StringConfig = "{\"dhcp\":"+dhcp+",\"ip\":\""+ipAddress+"\",\"mask\":\""+ipMask+"\",\"gw\":\""+ipGW+"\"}";    
   DEBUG_PRINTLN(StringConfig);
   StaticJsonDocument<512> jsonBuffer;
   DynamicJsonDocument doc(1024);
   deserializeJson(doc, StringConfig);
   
   File configFile = LITTLEFS.open(path, FILE_WRITE);
   if (!configFile) {
    DEBUG_PRINTLN(F("failed open"));
   }else{
     serializeJson(doc, configFile);
   }
  serverWeb.send(200, "text/html", "Save config OK ! <br><form method='GET' action='reboot'><input type='submit' name='reboot' value='Reboot'></form>");
  
 
}

void handleLogs() {
  String result;
  
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
   result +=F("<h1>Console</h1>");
   result += F("<div class='row justify-content-md-center'>");
   result += F("<div class='col-sm-6'>");
    result += F("<button type='button' onclick='cmd(\"ClearConsole\");document.getElementById(\"console\").value=\"\";' class='btn btn-primary'>Clear Console</button> ");
    result += F("<button type='button' onclick='cmd(\"GetVersion\");' class='btn btn-primary'>Get Version</button> ");
    result += F("<button type='button' onclick='cmd(\"ErasePDM\");' class='btn btn-primary'>Erase PDM</button> ");
   result += F("</div></div>");
   result += F("<div class='row justify-content-md-center' >");
   result += F("<div class='col-sm-6'>");
   
   result += F("Raw datas : <textarea id='console' rows='16' cols='100'>");
   
   result += F("</textarea></div></div>");
  //result += F("</div>");
  result += F("</body>");
  result +=F("<script language='javascript'>");
  result +=F("logRefresh(1000);");
  result +=F("</script>");
  result +=F("</html>");

  serverWeb.send(200, F("text/html"), result);
}

void handleTools() {
  String result;
  
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
   result +=F("<h1>Tools</h1>");
   result += F("<div class='btn-group-vertical'>");
   result += F("<a href='/logs' class='btn btn-primary mb-2'>Console</button>");
  // result += F("<a href='/fsbrowser' class='btn btn-primary mb-2'>FSbrowser</button>");
   //result += F("<a href='/update' class='btn btn-primary mb-2'>Update</button>");
   result += F("<a href='/reboot' class='btn btn-primary mb-2'>Reboot</button>");
  result += F("</div></body></html>");

  serverWeb.send(200, F("text/html"), result);
}

void handleReboot() {
  String result;
  
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
   result +=F("<h1>Reboot ...</h1>");
  result = result + F("</body></html>");
  serverWeb.sendHeader(F("Location"),F("/"));
  serverWeb.send(303); 

  ESP.restart();
}

void handleUpdate() {
  String result; 
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
   result +=F("<h1>Update ...</h1>");
   result += F("<div class='btn-group-vertical'>");
   result += F("<a href='/setchipid' class='btn btn-primary mb-2'>setChipId</button>");
   result += F("<a href='/setmodeprod' class='btn btn-primary mb-2'>setModeProd</button>");
  result += F("</div>");

  result = result + F("</body></html>");

  serverWeb.send(200, F("text/html"), result);
}

void handleFSbrowser()
{
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER); 
   result +=F("<h1>FSBrowser</h1>");
   result += F("<nav id='navbar-custom' class='navbar navbar-default navbar-fixed-left'>");
      result += F("      <div class='navbar-header'>");
        result += F("        <!--<a class='navbar-brand' href='#'>Brand</a>-->");
      result += F("      </div>");
   result +=F("<ul class='nav navbar-nav'>");
        
    String str = "";
    File dir = LITTLEFS.open("/config/");
    while (dir.openNextFile()) {
      String tmp =  dir.name();
        tmp = tmp.substring(8);    
        result += F("<li><a href='#' onClick=\"readfile('");
        result +=tmp;
        result+=F("');\">");
        result +=  tmp;
        result +=F(  " ( ");
        result +=  dir.size();
        result +=  F(" o)</a></li>");
        
    }
    result += F("</ul></nav>");
    result +=F("<div class='container-fluid' >");
    result +=F("  <div class='app-main-content'>");
    result+= F("<form method='POST' action='saveFile'>");
    result += F("<div class='form-group'>");
      result +=F(" <label for='file'>File : <span id='title'></span></label>");
       result +=F("<input type='hidden' name='filename' id='filename' value=''>");
      result +=F(" <textarea class='form-control' id='file' name='file' rows='10'>");     
      result +=F("</textarea>");
    result += F("</div>");
    result += F("<button type='submit' class='btn btn-primary mb-2'>Save</button>");
     result +=F("</Form>");            
    result +=F("</div>");
    result +=F("</div>");
  result +=  F("</body></html>");

  serverWeb.send(200, F("text/html"), result);
}

void handleReadfile()
{
  String result;
  String filename = "/config/"+serverWeb.arg(0);
  File file = LITTLEFS.open(filename, "r");
 
  if (!file) {
    return;
  }
 
  while (file.available()) {
    result += (char) file.read();
  }
  file.close();
  serverWeb.send(200, F("text/html"), result);
}

void handleLogBuffer()
{
  String result;
  result = logPrint();
  serverWeb.send(200, F("text/html"), result);

}

void handleScanNetwork()
{
   String result="";
   int n = WiFi.scanNetworks();
   if (n == 0) {
      result = " <label for='ssid'>SSID</label>";
      result += "<input class='form-control' id='ssid' type='text' name='WIFISSID' value='{{ssid}}'> <a onclick='scanNetwork();' class='btn btn-primary mb-2'>Scan</a><div id='networks'></div>";
    } else {
      
       result = "<select name='WIFISSID' onChange='updateSSID(this.value);'>";
       result += "<OPTION value=''>--Choose SSID--</OPTION>";
       for (int i = 0; i < n; ++i) {
            result += "<OPTION value='";
            result +=WiFi.SSID(i);
            result +="'>";
            result +=WiFi.SSID(i)+" ("+WiFi.RSSI(i)+")";
            result+="</OPTION>";
        }
        result += "</select>";
    }  
    serverWeb.send(200, F("text/html"), result);
}
void handleClearConsole()
{
  logClear();
  
  serverWeb.send(200, F("text/html"), "");
}

void handleGetVersion()
{
  //\01\02\10\10\02\10\02\10\10\03
  char output_sprintf[2];
  uint8_t cmd[10];
  cmd[0]=0x01;
  cmd[1]=0x02;
  cmd[2]=0x10;
  cmd[3]=0x10;
  cmd[4]=0x02;
  cmd[5]=0x10;
  cmd[6]=0x02;
  cmd[7]=0x10;
  cmd[8]=0x10;
  cmd[9]=0x03;

  Serial2.write(cmd,10);
  Serial2.flush();
  
  String tmpTime; 
  String buff="";
  timeLog = millis();
  tmpTime = String(timeLog,DEC);
  logPush('[');
  for (int j =0;j<tmpTime.length();j++)
  {
    logPush(tmpTime[j]);
  }
  logPush(']');
  logPush('-');
  logPush('>');

  for (int i=0;i<10;i++)
  {
    sprintf(output_sprintf,"%02x",cmd[i]);
    logPush(' ');
    logPush(output_sprintf[0]);
    logPush(output_sprintf[1]);
  }
  logPush('\n');
  serverWeb.send(200, F("text/html"), "");
}
