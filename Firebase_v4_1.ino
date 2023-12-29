
/////////////////////////////   CODE CAN RUN ////////////////////////////


#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>

#include <SoftwareSerial.h>
SoftwareSerial sim800(2, 3);

const char FIREBASE_HOST[] = ".................firebaseio.com";
const String FIREBASE_AUTH = "";
const String FIREBASE_PATH = "TEMP";
const int SSL_PORT = 443;

char apn[] = "internet";
char user[] = "";
char pass[] = "";

float TEMP1 = 0,TEMP2 = 2;

TinyGsm modem(sim800);
TinyGsmClientSecure gsm_client_secure_modem(modem, 0);
HttpClient http_client = HttpClient(gsm_client_secure_modem, FIREBASE_HOST, SSL_PORT);

unsigned long previousMillis = 0;

void setup()
{
  Serial.begin(9600);
  Serial.println("Seri bağlantı başladı");

  sim800.begin(9600);
  Serial.println("SIM800L bağlantısı açıldı");

  Serial.println("MODEM ACILIYOR...");
  modem.restart();
  String modemInfo = modem.getModemInfo();
  Serial.print("Modem: ");
  Serial.println(modemInfo);

  http_client.setHttpResponseTimeout(15 * 1000);
}

void loop()
{
  Serial.print(F("baglanıyor "));
  Serial.print(apn);
  if (!modem.gprsConnect(apn, user, pass))
  {
    Serial.println(" HATA");
    delay(1000);
    return;
  }
  Serial.println(" OK");

  http_client.connect(FIREBASE_HOST, SSL_PORT);

  while (true)
  {
    if (!http_client.connected())
    {
      Serial.println();
      http_client.stop(); 
      Serial.println("HTTP BAGLI DEGİL");
      break;
    }
    else
    {
      
      PostFixedValuesToFirebase(&http_client);
    }
  }
}

void PostToFirebase(const char *method, const String &path, const String &data, HttpClient *http)
{
  String response;
  int statusCode = 0;
  http->connectionKeepAlive(); 

  String url;
  if (path[0] != '/')
  {
    url = "/";
  }
  url += path + ".json";
  url += "?auth=" + FIREBASE_AUTH;
  Serial.print("POST:");
  Serial.println(url);
  Serial.print("Data:");
  Serial.println(data);

  String contentType = "application/json";
  http->put(url, contentType, data);

  statusCode = http->responseStatusCode();
  Serial.print("DURUM KODU: ");
  Serial.println(statusCode);
  response = http->responseBody();
  Serial.print("CEVAP: ");
  Serial.println(response);

  if (!http->connected())
  {
    Serial.println();
    http->stop(); // HTTP BAĞLANTISINI KESME FONKSİYONU
    Serial.println("HTTP POST baglantı kesildi");
  }
}

void PostFixedValuesToFirebase(HttpClient *http)
{
  /* burda yaptığım firebaase e gönderdiğimiz değer aynı olmaması için 0.1 arttırıyoruz bunun sebebi firebase e aynı değer gönderildiğinde veri geldi diye algılamıyor sadece farklı olanları 
        alıyor alıcı tarafında bunu bilerek azaltıp alacağız.
  */

  TEMP1+=0.1,TEMP2+=0.1;
  String data = "{";
  data += "\"TEMP1\":" + String(TEMP1) + ",";
  data += "\"TEMP2\":" + String(TEMP2) ;
 

  data += "}";
/*
 
  
 */
  PostToFirebase("PATCH", FIREBASE_PATH, data, http);

  
  delay(1000);
}
