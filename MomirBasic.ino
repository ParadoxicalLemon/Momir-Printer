#include "WiFiS3.h"
#include "WiFiSSLClient.h"
#include "arduino_secrets.h"
#include "jpgDecoding.h"
#include "Arduino_LED_Matrix.h"

const char ssid[] = SSID;
const char password[] = PASS;
const char server[] = "api.scryfall.com";
const char imageHost[] = "cards.scryfall.io";
const char api_ca[] = \
  "-----BEGIN CERTIFICATE-----\n" \
  "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\n" \
  "bP6MvPJwNQzcmRk13NfIRmPVNnGuV/u3gm3c\n" \
  "-----END CERTIFICATE-----\n";

const char image_ca[] = \
  "-----BEGIN CERTIFICATE-----\n" \
  "MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\n" \
  "RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\n" \
  "VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\n" \
  "DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\n" \
  "ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\n" \
  "VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\n" \
  "mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\n" \
  "IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\n" \
  "mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\n" \
  "XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\n" \
  "dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\n" \
  "jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\n" \
  "BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\n" \
  "DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\n" \
  "9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\n" \
  "jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\n" \
  "Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\n" \
  "ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\n" \
  "R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\n" \
  "-----END CERTIFICATE-----\n";

WiFiSSLClient  client;
int status = WL_IDLE_STATUS;

const char GetStart[] = "GET /cards/random?format=image&version=small&q=t:\/^[^\\/]*Creature\/+-is:funny+-is:alchemy+cmc=";
const char GetEnd[] = " HTTP/1.0";
char GET[140] = "";
bool loadingCard = false;

ArduinoLEDMatrix matrix;
const uint32_t digitLEDs[10][3] = {
  {0x00E01101,0x30150190,0x1100E000},
  {0x00400C01,0x40040040,0x0401F000},
  {0x00E01100,0x10020040,0x0801F000},
  {0x00E01100,0x10060010,0x1100E000},
  {0x00200600,0xA01201F0,0x02002000},
  {0x01F01001,0xE0010010,0x1100E000},
  {0x00600801,0x001E0110,0x1100E000},
  {0x01F01100,0x10020040,0x04004000},
  {0x00E01101,0x100E0110,0x1100E000},
  {0x00E01101,0x100F0010,0x0200C000}
};
const uint32_t OffLED[3] = {0x00000000,0x00000000,0x00000000};
const uint32_t OnLED[3] = {0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};
const uint8_t flickerTime = 100;
const uint8_t flickerCount = 4;
const uint8_t conversionShiftAmount = 6;

const uint8_t incCMCPin = 2;
const uint8_t decCMCPin = 3;
const uint8_t printPin = 4;

uint8_t incState = 0;
uint8_t decState = 0;
uint8_t printState = 0;
uint8_t incLastState = 0;
uint8_t decLastState = 0;
uint8_t printLastState = 0;

uint8_t curCMC = 0;
const uint8_t minCMC = 0;
const uint8_t maxCMC = 16;


void displayLEDNumber(int num){
  if(num < 0 || num > 99)
    return;

  int ones= num % 10;
  int tens = num / 10;
  uint32_t base[3] = {digitLEDs[tens][0], digitLEDs[tens][1], digitLEDs[tens][2]};
  uint32_t carry[2] = {base[1] >> (32-conversionShiftAmount), base[2] >> (32-conversionShiftAmount)};
  uint32_t shifted[3] = {
    base[0] << conversionShiftAmount | carry[0],
    base[1] << conversionShiftAmount | carry[1],
    base[2] << conversionShiftAmount};
  uint32_t buffer[3] = {
    shifted[0] | digitLEDs[ones][0], 
    shifted[1] | digitLEDs[ones][1],
    shifted[2] | digitLEDs[ones][2]
    };
  matrix.loadFrame(buffer);
}

void flickerLEDDisplay(){
  for(int i = 0; i < flickerCount; i++){
    matrix.loadFrame(OnLED);
    delay(flickerTime);
    matrix.loadFrame(OffLED);
    delay(flickerTime);
  }
}

void setup() {
  pinMode(incCMCPin, INPUT);
  pinMode(decCMCPin, INPUT);
  pinMode(printPin, INPUT);
  Serial.begin(115200);
  while (!Serial) {;}
  delay(250);
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  status = WiFi.begin(ssid, password);
  while (status != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.print("Connected to ");
  Serial.println(ssid);

  matrix.begin();
  displayLEDNumber(curCMC);
  getCardImage(0);
}

void getCardImage(int CMC){
  if(loadingCard)
    return;

  loadingCard = true;
  String URL;
  client.setCACert(api_ca);
  Serial.println("\nStarting connection to server...");
  if (!client.connect(server, 443)){
    Serial.println("Connection failed!");
    flickerLEDDisplay();
  }
  else {
    Serial.println("Connected to server!");

    char cmc[2];
    itoa(CMC,cmc,10);
    strcpy(GET, GetStart);
    strcat(GET, cmc);
    strcat(GET, GetEnd);

    client.println(GET);
    client.println("Host: api.scryfall.com");
    client.println("Connection: close");
    client.println();
    
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if(line.substring(0,8) == "location"){
        int loc = line.lastIndexOf('?');
        URL = line.substring(35, loc);
        Serial.println(URL);
        break;
      }
    }
    client.stop();
    getImageFromURI(URL);
  }
  delay(150);
  loadingCard = false;
}

void getImageFromURI(String uri){
  client.setCACert(image_ca);
  Serial.println("\nStarting connection to server...");
  if (!client.connect(imageHost, 443)){
    Serial.println("Connection failed!");
    flickerLEDDisplay();
  }
  else {
    Serial.println("Connected to server!");

    strcpy(GET, "GET ");
    //strcat(GET, uri.c_str());
    String tempURI = "https://cards.scryfall.io/small/front/4/9/49b7a950-bf79-46fa-8b29-b7856b38e0fd.jpg";
    strcat(GET, tempURI.c_str());
    strcat(GET, GetEnd);

    client.println(GET);
    client.println("Host: cards.scryfall.io");
    client.println("Connection: close");
    client.println();
    String imgData;
    
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r")
        break;
    }


    int byteCount = 0;
    Serial.println("1");
    readJpeg(client);

    uint16_t dataLen = 177;
    uint16_t bytesRead = 0;
    uint16_t bytesInScan = 0;
    uint8_t scanNum = 0;
    uint8_t scansToSkip = 0;
    uint8_t* fullData[8];
    uint8_t scanData[2700];
    uint16_t bitOffsets[10] = {0,0,0,0,0,0,0,0,0,0};
    uint16_t scanLengths[10];

    Serial.print("Reading Scan ");
    Serial.println(scanNum);
    while(client.available()){
      uint8_t byte = client.read();
      if(byte != 0xff)
        scanData[bytesInScan] = byte;
      else{
        uint8_t nextByte = client.read();
        if(nextByte == 0xd9){
          Serial.print("End of File");
          fullData[scanNum] = new uint8_t[bytesInScan];
          memcpy(fullData[scanNum], scanData, bytesInScan);
          scanLengths[scanNum] = bytesInScan;
          Serial.print("Finished Scan ");
          Serial.print(scanNum);
          Serial.print(". Total bytes: ");
          Serial.println(bytesInScan);
          Serial.println("Done Reading");
          break;
        }
        else if(nextByte == 0xc4){
          if(scansToSkip > 1){
            fullData[scanNum] = new uint8_t[bytesInScan];
            memcpy(fullData[scanNum], scanData, bytesInScan);
            scanLengths[scanNum] = bytesInScan;
            Serial.print("Finished Scan ");
            Serial.print(scanNum);
            Serial.print(". Total bytes: ");
            Serial.println(bytesInScan);
            bytesInScan = 0;
            scanNum++;
            if(scanNum > 7)
              break;
            Serial.print("Reading Scan ");
            Serial.println(scanNum);
          }
          scansToSkip++;
        }
        scanData[bytesInScan] = byte;
        scanData[bytesInScan + 1] = nextByte;
        bytesInScan++;
        bytesRead++;
      }
      bytesRead++;
      bytesInScan++;
      if(bytesInScan > 3000){
        Serial.println("Error");
      }
    }
    Serial.println("Stopping client");
    client.stop();

    Serial.println("Stopped client");

    for(int i = 0; i < scanNum; i++){
      Serial.print("Scan #");
      Serial.print(i);
      Serial.println(":");
      for(int j = 0; j < scanLengths[i]; j++){
        String hex = String(fullData[i][j], HEX);
        if(hex.length() < 2){
          hex = '0' + hex;
        }
        Serial.print(hex);
        Serial.print(" ");
        if(j%64 == 63)
          Serial.println();
      }
      Serial.println();
    }


    return;
    {
      while (client.available()) {
        //imgData = client.readString();
        uint8_t c = client.read();
        String hex = String(c, HEX);
        if(hex.length() < 2){
          hex = '0' + hex;
        }
        Serial.print(hex);
        Serial.print(' ');
        byteCount++;
      
        if(byteCount % 64 == 0){
          Serial.println();
          if(byteCount % 3072 == 0){
            Serial.println();
            Serial.println(byteCount / 3072 + 1);
          }
        }
      }
      Serial.println();
      Serial.println(byteCount);
      client.stop();
    }
  }
}
void loop() {
  return;
  incState = digitalRead(incCMCPin);
  if (incState != incLastState) {
    if (incState == HIGH && curCMC < maxCMC) {
      curCMC++;
      displayLEDNumber(curCMC);
      Serial.print("Current CMC: ");
      Serial.println(curCMC);
    }
    delay(50);
  }
  incLastState = incState;
  
  decState = digitalRead(decCMCPin);
  if (decState != decLastState) {
    if (decState == HIGH && curCMC > minCMC) {
      curCMC--;
      displayLEDNumber(curCMC);
      Serial.print("Current CMC: ");
      Serial.println(curCMC);
    }
    delay(50);
  }
  decLastState = decState;

  printState = digitalRead(printPin);
  if(printState != printLastState){
    if(printState == HIGH && !loadingCard){
      //getCard(curCMC);
    }
    delay(50);
  }
  printLastState = printState;
}