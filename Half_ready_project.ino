/************************************************************************************
    My Channel: https://www.youtube.com/c/TeachMeSomething
 
 *  *********************************************************************************
 *  Preferences--> Aditional boards Manager URLs : 
 *  For ESP32:
 *  https://dl.espressif.com/dl/package_esp32_index.json
 *  
 **********************************************************************************/

#include <WiFi.h>
#include <HTTPClient.h>
//#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

#define PIN 25

#ifndef PSTR
#define PSTR  // Make Arduino Due happy
#endif

const char* ssid = "UPCED7EFB8";        // type your wifi name
const char* password = "tFax8Er3yycw";  // Type your wifi password
String GOOGLE_SCRIPT_ID = "AKfycbwSjyzDmYfSyo5gKUHzYbMYpuSeTOOfx7y99PspT9s-znPwGS4o9M6WxXza6qCyQWYR";

WiFiClientSecure client;
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32, 8, PIN,
                                               NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
                                               NEO_GRB + NEO_KHZ800);
const uint16_t colors[] = {matrix.Color(80, 80, 80), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255)};

struct Pixel
{
    Pixel(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0) {
        this->r = r;
        this->g = g;
        this->b = b;
    }
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

Pixel map(int x, int in_min, int in_max, Pixel out_min, Pixel out_max) {

    if (!x) {
        return { 0, 0, 0 };
    }
    else {
        //TODO: DIVISION ON ZERO FIX
        uint8_t r;
        uint8_t g;
        uint8_t b;

        if (x == 1) {
            r = out_min.r;
            g = out_min.g;
            b = out_min.b;
        }
        else {
            r = (x - in_min) * (out_max.r - out_min.r) / (in_max - in_min) + out_min.r;
            g = (x - in_min) * (out_max.g - out_min.g) / (in_max - in_min) + out_min.g;
            b = (x - in_min) * (out_max.b - out_min.b) / (in_max - in_min) + out_min.b;
        }
        return { r, g, b };
    }
}

unsigned* createArrayFromextractNumbersFromString(const String &str, const unsigned max_extracted) {
  /*
    The current function takes a string str containing comma-separated numbers 
    and extracts them into an unsigned integer array of size max_extracted. 
  */

  // Input validation.
  if (!str.length() || (max_extracted == 0)) {
    return NULL;
  }

  // Memory allocation.
  unsigned* pNumbers = (unsigned*)calloc(max_extracted, sizeof(unsigned));
  if (pNumbers == NULL) { printf("Memory not allocated.\n"); }
 
  // Extract numbers from input string.
  unsigned index = 0;
  unsigned n = 0;
  for (unsigned long long i = 0; i < str.length() && index < max_extracted; ++i) {
    if (isdigit(str[i])) {
      n *= 10;
      n += str[i] - '0';
    } else if (str[i] == ',') {
      pNumbers[index++] = n;
      n = 0;
    }
  }
  
  // Insert the last number if not inserted yet.
  if (index < max_extracted) {
    pNumbers[index++] = n;
  }
  return pNumbers;
}

void setup() {

  Serial.begin(115200);
  delay(10);

  matrix.begin();
  matrix.setBrightness(20);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("OK");
}

String loadDataFromGoogleSheet(){
  HTTPClient http;
  String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?read";
  Serial.println("Reading Data From Google Sheet.....");
  http.begin(url.c_str());
  //Removes the error "302 Moved Temporarily Error"
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  //Get the returning HTTP status code
  int httpCode = http.GET();
  Serial.print("HTTP Status Code: ");
  Serial.println(httpCode);

  String payload;
  if (httpCode <= 0) {
    Serial.println("Error on HTTP request");
    http.end();
  } else if (httpCode == 200) {
    payload = http.getString();
  }

  //-------------------------------------------------------------------------------------
  http.end();
  return payload;
}

unsigned max(unsigned * pArr, unsigned size){
  int max_num = *pArr;
  for(int i = 0; i < size; ++i){
      if(pArr[i] > max_num){
        max_num = pArr[i];
      }
  }
  return max_num;
}

unsigned min(unsigned * pArr, unsigned size){
  int min_num = *pArr;
  for(int i = 0; i < size; ++i){
      if(pArr[i] < min_num){
        min_num = pArr[i];
      }
  }
  return min_num;
}

void loop() {
  matrix.fillScreen(0);


  uint16_t first_color = matrix.Color(80, 80, 80);
  uint16_t second_color = matrix.Color(80, 80, 80);
  
  String data = loadDataFromGoogleSheet();
  unsigned * pArr = createArrayFromextractNumbersFromString(data, 7 * 32);

  int min_score = min(pArr, 7*32);
  int max_score = max(pArr, 7*32);

  //(14,68,41)
  Pixel min_pixel(14,68,0);
  //(57,211,83)
  Pixel max_pixel(57,211,0);

  Pixel hash[7 * 32];

  for(int i = 0; i <7 * 32; ++i){
    hash[i] = map(pArr[i],  min_score,  max_score, min_pixel, max_pixel);
  }

  // Show data in bord.
  const int ROWS = 7;
  const int COLS = 32;
  for (int row = 0; row < ROWS; ++row) {
    for (int col = 0; col < COLS; ++col) {
      int index = ROWS * (COLS - col) - row - 1;
      //int value = pArr[index];
      Pixel p = hash[index];
      matrix.drawPixel(col, row, matrix.Color(p.r, p.g, p.b));
    }
  }
  matrix.show();
  free(pArr);
  delay(60000*10);
}
