#include <Arduino.h> 
class HuffTable{
  public:
    HuffTable(size_t size = 10){
      maxLength = size;
      keys = new String[size];
      values = new uint8_t[size];
    }

    ~HuffTable(){
      delete keys;
      delete values;
    }

    uint8_t insert(String key, uint8_t val){
      if(contains(key) > 0){
        Serial.print("already contains key: ");
        Serial.println(key);
        return -1;
      }
      if(length == maxLength){
        resize();
      }
      keys[length] = key;
      values[length] = val;
      length++;
      return length - 1;
    }

    uint8_t get(String key){
      size_t index = contains(key);
      if(index == -1)
        return 0b11111111;
      return values[index];
    }

    void remove(String key){
      size_t index = contains(key);
      if(index < 0)
        return;
      for(int i = index; i < length - 1; i++){
        keys[i] = keys[i + 1];
      }
      length--;
    }

    size_t count(){
      return length;
    }

    int8_t contains(String key){
      for(uint8_t i = 0; i < length; i++){
        if(keys[i] == key)
          return i;
      }
      return -1;
    }

    void resize(){
      String newKeys[maxLength * 2];
      uint8_t newValues[maxLength * 2];
      for(int i = 0; i < maxLength; i++){
        newKeys[i] = keys[i];
        newValues[i] = values[i];
      }
      maxLength *= 2;
      keys = newKeys;
      values = newValues;
    }

    uint8_t operator [](String key){
      return get(key);
    }

    void printValues(){
      Serial.print("Table Size: ");
      Serial.print(length);
      Serial.print("/");
      Serial.println(maxLength);
      for(int i = 0; i < length; i++){
        Serial.print(keys[i]);
        Serial.print(": ");
        Serial.println(values[i]);
      }
    }
  
  private:
    size_t length = 0;
    size_t maxLength;
    String* keys;
    uint8_t* values;

};

class SOSData{
  public:
    SOSData(uint8_t size){
      cNum = size;
      components = new uint8_t*[size];
      for(uint8_t i = 0; i < size; i++){
        components[i] = new uint8_t[2];
      }
    }
    void printData(){
      Serial.println("SOS:");
      Serial.println("Components:");
      for(int i = 0; i < cNum; i++){
        Serial.print(i);
        Serial.print(": ");
        Serial.print(components[i][0]);
        Serial.print(", ");
        Serial.println(components[i][1]);
      }
      Serial.print("Spectral: ");
      Serial.print(specMin);
      Serial.print("..");
      Serial.println(specMax);
      Serial.print("Approx: ");
      Serial.println(appr);
    }
  public:
    uint8_t cNum;
    uint8_t** components;
    uint8_t specMin;
    uint8_t specMax;
    uint8_t appr;
};

uint8_t readByte(WiFiSSLClient client){
  return client.read();
}

uint8_t* readBytes(WiFiSSLClient client, uint16_t num){
  uint8_t arr[num];
  for(uint16_t i = 0; i < num; i++){
    arr[i] = client.read();
  }
  return arr;
}

char getBit(uint8_t byte, uint8_t pos){
  if((byte >> pos) & 1 == 0)
    return '0';
  else
    return '1';
}

void skipBytes(WiFiSSLClient client, int num = 1){
  for(int i = num; i > 0; i--){
    client.read();
  }
}

uint16_t readWord(WiFiSSLClient client){
  return (client.read() << 8) | client.read();
}

void readSegment(WiFiSSLClient client){
  uint16_t len = readWord(client) - 2;
  skipBytes(client, len);
}

HuffTable* readDHT(WiFiSSLClient client, uint16_t count){
  HuffTable* hTable = new HuffTable(count);
  String code = "0";

  uint8_t lengths[16];
  for(int i = 0; i < 16; i++){
    lengths[i] = readByte(client);
  }

  uint8_t elements[count];
  for(int i = 0; i < count; i++){
    elements[i] = readByte(client);
  }

  int nextElement = 0;
  for(int i = 0; i < 16; i++){
    for(int j = 0; j < lengths[i]; j++){
      hTable->insert(code, elements[nextElement]);
      nextElement++;
      int index = code.lastIndexOf('0');
      code.setCharAt(index, '1');
      for(int k = index + 1; k < code.length(); k++){
        code.setCharAt(k, '0');
      }
    }
    code += '0';
  }
  return hTable;
}

SOSData* readSOS(WiFiSSLClient client){

  uint16_t len = readWord(client) - 2;
  uint8_t cNum = readByte(client);
  SOSData* data = new SOSData(cNum);
  for(int i = 0; i < cNum; i++){
    readByte(client);
    uint8_t tables = readByte(client);
    data->components[i][0] = tables >> 4;
    data->components[i][1] = tables & 0b00001111;
  }
  data->specMin = readByte(client);
  data->specMax = readByte(client);
  data->appr = readByte(client);
  return data;
}

class IDCT{
  public:
    IDCT(){
      for(uint8_t y = 0; y < 8; y++){
        for(uint8_t x = 0; x < 8; x++){
          float norm = 1.0;
          if(y == 0)
            norm = 1.0 / sqrt(2.0);
          idctTable[y][x] = norm * cos(((2 * x + 1) * y * 3.14159) / 16.0);
        }
      }
    }

    uint8_t** getIDCT(uint8_t* base){
      uint8_t values[8][8];
      for(uint8_t y = 0; y < 8; y++){
        for(uint8_t x = 0; x < 8; x++){
          values[y][x] = base[zigzag[y][x]];
        }
      }

      uint8_t** out = new uint8_t*[8];
      for(uint8_t i = 0; i < 8; i++){
        out[i] = new uint8_t[8];
      }

      for(uint8_t y = 0; y < 8; y++){
        for(uint8_t x = 0; x < 8; x++){
          float localSum = 0;
          for(uint8_t v = 0; v < 8; v++){
            for(uint8_t u = 0; u < 8; u++){
              localSum += values[v][u] * idctTable[v][y] * idctTable[u][x];
            }
          }
          out[y][x] = (int)(localSum / 4);
        }
      }
      return out;
    }

  private:
    uint8_t zigzag[8][8] = {
      {0, 1, 5, 6, 14, 15, 27, 28},
      {2, 4, 7, 13, 16, 26, 29, 42},
      {3, 8, 12, 17, 25, 30, 41, 43},
      {9, 11, 18, 24, 31, 40, 44, 53},
      {10, 19, 23, 32, 39, 45, 52, 54},
      {20, 22, 33, 38, 46, 51, 55, 60},
      {21, 34, 37, 47, 50, 56, 59, 61},
      {35, 36, 48, 49, 57, 58, 62, 63}};
    float idctTable[8][8];
};


void readJpeg(WiFiSSLClient client){
  uint16_t marker;
  uint8_t dqt[64];
  uint16_t height;
  uint16_t width;
  uint8_t sampleFactor;
  HuffTable* huffTables[4];
  SOSData* sos;

  marker = readWord(client);
  if(marker != 0xffd8){
    Serial.println("No SOI found");
    return;
  }

  // APP0
  marker = readWord(client);
  if(marker != 0xffe0){
    Serial.println("No APP0 found");
    return;
  }
  readSegment(client);
  
  // DQT (Lumiance)
  marker = readWord(client); {
    if(marker != 0xffdb){
      Serial.println("No DQT found");
      return;
    }
    uint16_t len = readWord(client) - 3;
    readByte(client);
    for(int i = 0; i < len; i++){
      dqt[i] = readByte(client);
    }
  }

  // DQT (Chromiance)
  marker = readWord(client);
  if(marker != 0xffdb){
    Serial.println("No DQT found");
      return;
  }
  readSegment(client);

  // SOF2
  marker = readWord(client); {
    if(marker != 0xffc2){
      Serial.println("No SOF2 found");
      return;
    }
    skipBytes(client, 3);
    height = readWord(client);
    width = readWord(client);
    skipBytes(client, 2);
    sampleFactor = readByte(client);
    skipBytes(client, 7);
  }
  return;

  // DHT
  marker = readWord(client); {
    if(marker != 0xffc4){
      Serial.println("No DHT found");
      return;
    }
    uint16_t count = readWord(client) - 19;
    uint8_t info = readByte(client);
    int8_t index = (info & 0b00001111) + (2 * (info >> 4));
    huffTables[index] = readDHT(client, count);
  }

  // DHT
  marker = readWord(client); {
    if(marker != 0xffc4){
      Serial.println("No DHT found");
      return;
    }
    uint16_t count = readWord(client) - 19;
    uint8_t info = readByte(client);
    int8_t index = (info & 0b00001111) + (2 * (info >> 4));
    huffTables[index] = readDHT(client, count);
  }

  // SOS
  marker = readWord(client); {
    if(marker != 0xffda){
      Serial.println("No SOS found");
      return;
    }
    sos = readSOS(client);
  }

  IDCT* idct = new IDCT();
  int16_t dcCoef = 0;
  int16_t prevDCCoef = 0;
  //uint8_t scanLine[8][152];
  uint8_t imgData[204][19];
  uint8_t chunkData[64];
  uint8_t currentByte;
  uint8_t checkByte;
  uint8_t currentPos = 0;
  String currentCode = "";

  uint8_t xChunks = width % 8 == 0 ? width / 8 : width / 8 + 1;
  uint8_t yChunks = height % 8 == 0 ? height / 8 : height / 8 + 1;

  while(true){

  }


  while(true){
    for(uint8_t i = height / 8; i > 0; i--){
      for(uint8_t j = width / 8; j > 0; j--){
        currentByte = readByte(client);
        while(true){
          currentCode += getBit(currentByte, currentPos);
          currentPos++;
          if(currentPos > 7){
            currentByte = readByte(client);
            currentPos = 0;
          }
          uint8_t huffVal = huffTables[0]->get(currentCode);
          if(huffVal != 0b11111111){
            dcCoef = huffVal + prevDCCoef;
            break;
          } 
          if(currentCode.length() > 8){
            Serial.println("No huff Value found");
            return;
          }
        }
        chunkData[0] = dcCoef * dqt[0];
        uint8_t count = 1;
        while(count < 64){
          // First scan has no AC data


          uint8_t huffVal;
          while(true){
            currentCode += getBit(currentByte, currentPos);
            currentPos++;
            if(currentPos > 7){
              currentByte = readByte(client);
              if(currentByte == 0xff){
                checkByte = readByte(client);
                if(checkByte == 0x00){
                  break;
                }
                if(checkByte == 0xc4){
                  uint16_t count = readWord(client) - 19;
                  uint8_t info = readByte(client);
                  int8_t index = (info & 0b00001111) + (2 * (info >> 4));
                  huffTables[index] = readDHT(client, count);
                  count = 64;
                  break;
                }
                if(checkByte == 0xda){
                  sos = readSOS(client);
                  count = 64;
                  break;
                }
                if(checkByte == 0xd9){
                  Serial.println("End of File");
                  count = 65;
                  break;
                }

              }
              currentPos = 0;
            }
            huffVal = huffTables[0]->get(currentCode);
            if(huffVal != 0b11111111){
              break;
            } 
            if(currentCode.length() > 8){
              Serial.println("No huff Value found");
              return;
          }
          if(count == 64)
            continue;
          if(count == 65)
            break;
          if(huffVal == 0)
            break;
          
        }

        }
        //matL, oldlumdccoeff = BuildMatrix(AllData,0, quant[0], oldlumdccoeff)
        
      }
    }


    uint8_t nextByte = readByte(client);
    if(nextByte == 0xff){
      nextByte = readByte(client);
      if(nextByte == 0x00){
        continue;
      }
      if(nextByte == 0xc4){
        uint16_t count = readWord(client) - 19;
        uint8_t info = readByte(client);
        int8_t index = (info & 0b00001111) + (2 * (info >> 4));
        huffTables[index] = readDHT(client, count);
      }
      if(nextByte == 0xda){
        sos = readSOS(client);
        continue;
      }
      if(nextByte == 0xd9){
        Serial.println("End of File");
        break;
      }
    }
  }
}
