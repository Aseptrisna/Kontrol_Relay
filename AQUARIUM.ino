  /*
   * 
   * Bandar Lampung, Januari 2021
  
  */
  
  #include <ESP8266WiFi.h>
  #include <PubSubClient.h>
  #include <DallasTemperature.h>
  #include <OneWire.h>
  #define ONE_WIRE_BUS 4
  OneWire oneWire(ONE_WIRE_BUS);
  DallasTemperature sensors(&oneWire); 
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <ArduinoJson.h>
  #include <FS.h> 
  #include <Ticker.h>
  #include <EasyNTPClient.h>
  String suhu;
  String Status_Lampu;
  String Status_Kipas;
  const int kipas  = D5;
  const int lampu  = D4; 
  WiFiClient client;
  String request_string;
  const char* host ="192.168.43.222";
  String path      = "/Api/lampu.json";  
  String path1     = "/Api/kipas.json"; 
  HTTPClient http;
  StaticJsonBuffer<200> jsonBuffer;
  const char* ssid = "DaBol";
  const char* password = "05050505";
  const char* mqtt_server = "XXXX";
  const char* mqtt_user = "XXXXXX";
  const char* mqtt_password = "XXXXX";
  const int   mqttPort = 1883;
   
  WiFiClient espClient;
  PubSubClient Client(espClient);
  void setup() {
  pinMode(kipas,OUTPUT);
  pinMode(lampu,OUTPUT);
  Serial.begin(9600);
   WiFi.disconnect();
     WiFi.begin("DaBol","05050505");
      while ((!(WiFi.status() == WL_CONNECTED))){
        delay(2000);
      }
       Serial.println("Konek Ke Wi-Fi");
       Client.setServer(mqtt_server, mqttPort);
   
    while (!Client.connected()) {
      Serial.println("Konek ke MQTT...");
      if (Client.connect("Aquarium...", mqtt_user, mqtt_password )) {
        Serial.println("Konek");
      } else {
        Serial.print("Gagal dong :(");
        Serial.print(Client.state());
        delay(2000);
   
      }
    }
  }
   
  void loop() {
    Client.loop();
    kontrol_kipas();
    kontrol_lampu();
    publish_data();   
    getsuhu();
    kirimdata();
    Triger();
    
  }
  
  void kirimdata(){
      delay(1000);
      if (!client.connect(host,80)) {
        Serial.println("Gagal Konek :(");
        return;
      }
      request_string = "/Api/input_sensor.php?data=";
      request_string +=suhu;
      Serial.print("requesting URL: ");
      Serial.println(request_string);
      client.print(String("GET ") + request_string + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
        unsigned long timeout = millis();
        while (client.available() == 0) {
        if (millis() - timeout > 1000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    } 
   }
    void getsuhu(){
      sensors.requestTemperatures();  
      suhu=sensors.getTempCByIndex(0);    
      Serial.println("Suhu:");
      Serial.println(sensors.getTempCByIndex(0));  
      delay(500);
    }
    void publish_data(){
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();
        root["mac"] = "1234567890";
        root["suhu"] =sensors.getTempCByIndex(0);
        root["kipas"] =Status_Kipas;
        root["lampu"] =Status_Lampu;
        String pubmsg;
        root.printTo(pubmsg);
        Client.publish("dani", pubmsg.c_str());
     
      }
   void kontrol_lampu(){
    Serial.print("connecting to ");
    Serial.println(host);
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      return;
    }
  
    client.print(String("GET ") + path + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Connection: keep-alive\r\n\r\n");
  
    delay(500);
    String section="header";
    while(client.available()){
      String line = client.readStringUntil('\r');
      if (section=="header") {
        Serial.print(".");
        if (line=="\n") { 
          section="json";
        }
      }
      else if (section=="json") { 
        section="ignore";
        String result = line.substring(1);
        int size = result.length() + 1;
        char json[size];
        result.toCharArray(json, size);
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& json_parsed = jsonBuffer.parseObject(json);
        if (!json_parsed.success()){
          Serial.println("parseObject() failed");
          return;
        }
        if (strcmp(json_parsed["light"], "on") == 0) {
          digitalWrite(lampu, LOW); 
          Serial.println("LAMPU HIDUP");
          Status_Lampu="on";
        }
        else {
          digitalWrite(lampu, HIGH);
          Serial.println("LAMPU MATI");
          Status_Lampu="on";
        }
      }
    }
   }
    void kontrol_kipas(){
    Serial.print("Konek ke?");
    Serial.println(host);
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
      Serial.println("Koneksi Gagal :(");
      return;
    }
  
    client.print(String("GET ") + path1 + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Connection: keep-alive\r\n\r\n");
  
    delay(500);
    String section="header";
    while(client.available()){
      String line = client.readStringUntil('\r');
      if (section=="header") {
        Serial.print(".");
        if (line=="\n") { 
          section="json";
        }
      }
      else if (section=="json") { 
        section="ignore";
        String result = line.substring(1);
        int size = result.length() + 1;
        char json[size];
        result.toCharArray(json, size);
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& json_parsed = jsonBuffer.parseObject(json);
        if (!json_parsed.success()){
          Serial.println("parseObject() failed");
          return;
        }
        if (strcmp(json_parsed["light"], "on") == 0) {
          digitalWrite(kipas, LOW); 
          Serial.println("KIPAS HIDUP");
          Status_Kipas="on";
        }
        else {
          digitalWrite(kipas, HIGH);
          Serial.println("KIPAS MATI");
          Status_Kipas="off";
        }
      }
    }
   }
   void Triger(){
    if(sensors.getTempCByIndex(0)< 27){
      digitalWrite(lampu, LOW);
      digitalWrite(kipas, HIGH);
      Status_Lampu="on";
      Status_Kipas="off";
    }else  if(sensors.getTempCByIndex(0)> 31){
      digitalWrite(lampu, HIGH);
      digitalWrite(kipas, LOW);
      Status_Lampu="off";
      Status_Kipas="on";
    }
   }
