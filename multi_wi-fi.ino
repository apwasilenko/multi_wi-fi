#include <ESP8266WiFi.h>        // Библиотека создания Wi-Fi подключения
#include <ESP8266WebServer.h>   //Библиотека для управления устройством по средствам веб сервера Wi-Fi
#include <FS.h>                 //Библиотека для работы с файловой системой модуля // Работает без него
#include <ESP8266FtpServer.h>   //Библиотека для работы с SPIFFS по FTP
#include <ArduinoOTA.h>         // Библиотека для OTA-прошивки
#include <ESP8266WiFiMulti.h>   // Работает без него
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>


const byte relay = D4;         //Пин подключения сигнального контак
//const char *ssid = "MyESP";   //Названеие генерируемой точки доступа

String wifidata[][2]= {{"Keenetic-2107","VW2A4WYC"}, {"car-wi-fi","24242424"}, {"ELTEX-2F30","GP21440418"}, {"Swift 2 X","24242424"}};

long myTimeInt;
String myTimeStr;
int status_Wi_Fi;


ESP8266WebServer HTTP(80);    //Определяем объект и порт веб сервера для НТТР
FtpServer ftpSrv;             //Определяем объект для работы с FTP модулем 


void setup(){
  pinMode(relay, OUTPUT);     //Определяем пин реле как исходящий
  Serial.begin(115200);       //Инициализируем вывод данных на серийный порт
  delay(100);
  Serial.println("\n Start Serial ");

  // инициализируем работу Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.hostname("espnodemcu2");
  int n = WiFi.scanNetworks();
  int id_wi_fi;
  int maxrssi = -200;
  for (int i = 0; i < sizeof(wifidata) / sizeof(wifidata[0]);i++){
   Serial.printf("Зарегестрирована сеть - |%s| пароль |%s|\n", wifidata[i][0],wifidata[i][1]);
  }
  Serial.println();
  if (n > 0){
  for (int i = 0; i < n; i++){
       Serial.printf("%d: %s, Ch:%d (%ddBm) %s\n", i+1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "close");
      for (int j = 0; j <  sizeof(wifidata) / sizeof(wifidata[0]); j++){
           Serial.printf("Сверяем сеть |%s| c сетью |%s|\n",wifidata[j][0], WiFi.SSID(i).c_str());

        if (wifidata[j][0] == WiFi.SSID(i).c_str()) {
          if (maxrssi < WiFi.RSSI(i)){
            Serial.printf("\n\nНайдена лучшая сеть %dbBm заместо прежней %ddBm\n\n", WiFi.RSSI(i), maxrssi);
            id_wi_fi = j;
            maxrssi = WiFi.RSSI(i);            
          }
          Serial.printf("\n\n Найдена объявленная сеть \n\n");
        }
      }
    }
    if (id_wi_fi >= 0){
      Serial.printf("\n\n Подключаемся к сети номер %d с именем %s \n\n", id_wi_fi, wifidata[id_wi_fi][0]);    
      WiFi.begin(wifidata[id_wi_fi][0], wifidata[id_wi_fi][1]);
      while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        delay(1000);
        ESP.restart();
      }
      Serial.print(" IP address: ");
      Serial.println(WiFi.localIP());
      Serial.println(WiFi.hostname());  
    }
 }
 else {
  Serial.printf("/n Нет сетей\n");
 }

  ArduinoOTA.setHostname("NODEMCU-02"); // Задаем имя сетевого порта
  ArduinoOTA.setPassword((const char *)"24242424"); // Задаем пароль доступа для удаленной прошивки
  ArduinoOTA.begin(); // Инициализируем OTA


  SPIFFS.begin();                 //инициализирует работу с файловой системой
  HTTP.begin();                   //Инициализируем работу WEB-сервера
  ftpSrv.begin("relay", "relay"); //Инициализируем работу FTP-сервера



//Обработка HTTP-запросов
  HTTP.on("/relay_switch", [](){   //При HTTP запросе вида http://192.168.4.1/relay_switch_
    HTTP.send(200,"text/plain", relay_switch());//Отдаем клиенту код успешной обработки запроса
  });
  HTTP.on("/relay_status", [](){
    HTTP.send(200,"text/plain", relay_status());
  });  
   HTTP.on("/mytime", [](){
    HTTP.send(200,"text/plain", mytime());
  });  
  HTTP.onNotFound([](){
    if (!handleFileRead(HTTP.uri()))
      HTTP.send(404, "text/plain", "Not Found");    
  });
  if (status_Wi_Fi != WiFi.status()){
    status_Wi_Fi = WiFi.status();
    if (WiFi.status() == WL_CONNECTED) Serial.printf("%d – подключение выполнено успешно\n", WiFi.status());
    if (WiFi.status() == WL_NO_SSID_AVAIL) Serial.printf("%d – заданный SSID находится вне зоны доступа\n", WiFi.status());
    if (WiFi.status() == WL_CONNECT_FAILED) Serial.printf("%d – неправильный пароль\n", WiFi.status());
    if (WiFi.status() == WL_IDLE_STATUS) Serial.printf("%d – WiFi-сеть переключается с одного статуса на другой\n", WiFi.status());
    if (WiFi.status() == WL_DISCONNECTED) Serial.printf("%d – модуль не находится в режиме станции\n", WiFi.status());
  }
  
}


void loop() {
    if (status_Wi_Fi != WiFi.status()){
    status_Wi_Fi = WiFi.status();
    if (WiFi.status() == WL_CONNECTED) Serial.printf("%d – подключение выполнено успешно\n", WiFi.status());
    if (WiFi.status() == WL_NO_SSID_AVAIL) Serial.printf("%d – заданный SSID находится вне зоны доступа\n", WiFi.status());
    if (WiFi.status() == WL_CONNECT_FAILED) Serial.printf("%d – неправильный пароль\n", WiFi.status());
    if (WiFi.status() == WL_IDLE_STATUS) Serial.printf("%d – WiFi-сеть переключается с одного статуса на другой\n", WiFi.status());
    if (WiFi.status() == WL_DISCONNECTED) Serial.printf("%d – модуль не находится в режиме станции\n", WiFi.status());
  }
/*
 *
   int n = WiFi.scanNetworks();
  int id_wi_fi;
  int maxrssi = -200;
  for (int i = 0; i < sizeof(wifidata) / sizeof(wifidata[0]);i++){
   Serial.printf("Зарегестрирована сеть - |%s| пароль |%s|\n", wifidata[i][0],wifidata[i][1]);
  }
  Serial.println();
  if (n > 0){
  for (int i = 0; i < n; i++){
       Serial.printf("%d: %s, Ch:%d (%ddBm) %s\n", i+1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "close");
      for (int j = 0; j <  sizeof(wifidata) / sizeof(wifidata[0]); j++){
           Serial.printf("Сверяем сеть |%s| c сетью |%s|\n",wifidata[j][0], WiFi.SSID(i).c_str());

        if (wifidata[j][0] == WiFi.SSID(i).c_str()) {
          if (maxrssi < WiFi.RSSI(i)){
            Serial.printf("\n\nНайдена лучшая сеть %dbBm заместо прежней %ddBm\n\n", WiFi.RSSI(i), maxrssi);
            id_wi_fi = j;
            maxrssi = WiFi.RSSI(i);            
          }
          Serial.printf("\n\n Найдена объявленная сеть \n\n");
        }
      }
    }
    if (id_wi_fi >= 0){
      Serial.printf("\n\n Подключаемся к сети номер %d с именем %s \n\n", id_wi_fi, wifidata[id_wi_fi][0]);    
      WiFi.begin(wifidata[id_wi_fi][0], wifidata[id_wi_fi][1]);
      while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        delay(1000);
        ESP.restart();
      }
      Serial.print(" IP address: ");
      Serial.println(WiFi.localIP());
      Serial.println(WiFi.hostname());  
    }
 }
 else {
  Serial.printf("/n Нет сетей\n");
 }
 *
 * /
  
  ArduinoOTA.handle(); // Всегда готовы к прошивке
  HTTP.handleClient(); // Запускаем HTTP сервер
  ftpSrv.handleFTP();  // Запускаем FTP сервер
}
String mytime(){     //Функция чтения времени с сайта
 // wait for WiFi connection
  if ((WiFi.waitForConnectResult() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, "https://yandex.com/time/sync.json?geo=100991")) {  // HTTP


      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload.substring(8,21));
          Serial.print("Time: ");
          unsigned long time;
          time = millis(); //выводит время прошедшее с момента начала выполнения программы
          Serial.println(time);

          return payload;
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        return "Нет данных";
      }
      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
      return "Нет данных";
    }
  }
  
}

String relay_switch(){     //Функция переключения реле
  byte state;
  if (digitalRead(relay))  //Если на пине высокий уровень
    state = 0;
  else 
    state = 1;
  digitalWrite(relay, state);
  return String(state);
}

String relay_status(){     //Функция определения состояния
  byte state;
  if (digitalRead(relay))  //Если на пине высокий уровень
    state = 1;
  else 
    state = 0;
    return String(state);
}

bool handleFileRead(String path){                     //Функция работы с файловой системой
  if (path.endsWith("/")) path += "index.html";       //Если устройство вызывает корень
String contentType = getContentType(path);            //С помощью функции getContentType определяем заголовок 
  if (SPIFFS.exists(path)) {                          //Если в файловой системе существует заархивированный или простой 
    File file = SPIFFS.open(path, "r");               //Открываем файл для чтения
    size_t sent = HTTP.streamFile(file, contentType); //Выводим содержимое файла по HTTP,
    file.close();                                     //Закрываем файл
    return true;                                      //Завершаем выполнение функции
  }
  return false;                                       //Завершаем выполнение функции
}

String getContentType(String filename) {              //Функция возврашшает необходимый тип файла
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".ipg")) return "image/jpeg";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".json")) return "application/json";
  return "text/plain";
}
