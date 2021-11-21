#include <FS.h>
#include <NTPClient.h>
// ligne suivante si réseau CST
#include <ESP8266WiFi.h>
// ligne suivante si réseau perso
//#include <ESP8266WebServer.h>
#include <WiFiUdp.h>

#define ssid "######"
#define password "######"

// Set web server port number to 80
WiFiServer server ( 80 );

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

const int pinContactSec = 0;
int value;
int counter;
bool depassement = false;

String fileTxt; 

// Debug
//...
//

void setup() {
  Serial.begin(115200);
  
//Starts WiFi Connexion
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

// Start Web Server
  server.begin();
  
// Get the time (through NTP server) 
  timeClient.begin();
  timeClient.setTimeOffset(3600);
  Serial.println();

//demarrage file system
  SPIFFS.begin();
  File f = SPIFFS.open("/coupures.txt", "r");   
  fileTxt = f.readString();
  int nbrCaracteres = fileTxt.length();
  int storage = nbrCaracteres / 4194304*100;
  
  Serial.println("Mémoire occupées par " + String(nbrCaracteres) + " caractères / 4194304 octets soit environ " + String(storage) + "%");
  Serial.println("Liste des fronts montants en mémoire :");
 
  Serial.println(fileTxt + "---");

  String lastCaractere = fileTxt.substring(fileTxt.length()-6, fileTxt.length()-1);
  Serial.print("lastCaractere :");
  Serial.println(lastCaractere);
  counter = abs(lastCaractere.toInt());
  Serial.print("counter :");
  Serial.println(counter);
  f.close();

  Serial.println();
  Serial.println("------- debut d'acquisition -------");  

//Declaration de la LED de l'ESP8266
  pinMode(BUILTIN_LED, OUTPUT);
}

void loop() {
  WiFiClient client = server.available();  
  if (!client) {
    
    timeClient.update();
    String d = timeClient.getFormattedDate();
  //  int s = ESP.getFlashChipRealSize();
  //  Serial.print("real Chip size :" + s);
    value = analogRead(pinContactSec);
  
    Serial.print(counter);
    Serial.println(" coupures | Date (actuelle): " +  d + " | Value actualisée du PIN A0: "  + value );
  
    if (value > 500 && depassement == false){
      depassement = true;
      ++counter;
      Serial.print(counter);
      Serial.print(" ");
      Serial.print("due to value: ");
      Serial.println(value);
  
      //Ecriture dans le fichier "coupures.txt"
      File  f = SPIFFS.open("/coupures.txt", "w");
      f.println(fileTxt + d + "-" + counter);
      int nbrCaracteres = fileTxt.length(); // pour afficher le % de stockage occupé si besoin. Non utilisé ici
      f.close();
      
      digitalWrite(BUILTIN_LED, LOW);
      delay(100);
      digitalWrite(BUILTIN_LED, HIGH);
      delay(100);
      digitalWrite(BUILTIN_LED, LOW);
      delay(100);
      digitalWrite(BUILTIN_LED, HIGH);
    }
        
    else if (value < 500 && depassement == true){
      depassement = false;
    }
    delay(2000); 
    File  f = SPIFFS.open("/coupures.txt", "r");
    fileTxt = f.readString();
    Serial.println(fileTxt);
    f.close();
  
    return;
   
  } 
  
  //Display content on Web browser
  client.println("Coupures");
  client.println("");
  client.println(fileTxt);
  
}
