#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <Timezone.h>    // https://github.com/JChristensen/Timezone
#include <RTCZero.h>
#include <Adafruit_NeoPixel.h>
RTCZero rtc;
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#define LED_PIN    6
#define LED_COUNT 86
const int pResistor = A0;
int minutevalue;
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
int lightvalue;
int status = WL_IDLE_STATUS;
char ssid[] = "";        // your network SSID (name)
char pass[] = "";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)
unsigned int localPort = 2390;      // local port to listen for UDP packets
IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
WiFiUDP Udp;
int sizeof4 = 12;
int secondcounter = 1;
const int rows = 10;
const int columns = 21;
int Sized[] = {18, 6, 15, 15, 12, 15, 18, 9, 21, 18};

//this array is all the pixel values for all the digits.  this tells the code later on which pixels to turn on for what digit.
int number0[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 18, 19, 20}; //ok
int number1[] = {3, 4, 5, 6, 7, 8}; //ok
int number2[] = {0, 1, 2, 6, 7, 8, 9, 10, 11, 15, 16, 17, 18, 19, 20}; //ok
int number3[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 15, 16, 17}; //question
int number4[] = {3, 4, 5, 6, 7, 8, 12, 13, 14, 15, 16, 17}; //ok
int number5[] = {0, 1, 2, 3, 4, 5, 9, 10, 11, 12, 13, 14, 15, 16, 17}; //ok
int number6[] = {0, 1, 2, 3, 4, 5, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20}; //ok
int number7[] = {3, 4, 5, 6, 7, 8, 9, 10, 11}; //ok
int number8[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20}; //good
int number9[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17}; //good



// United Kingdom (London, Belfast)
TimeChangeRule BST = {"BST", Last, Sun, Mar, 1, 60};        // British Summer Time
TimeChangeRule GMT = {"GMT", Last, Sun, Oct, 2, 0};         // Standard Time
Timezone UK(BST, GMT);


// US Pacific Time Zone (Las Vegas, Los Angeles)
TimeChangeRule usPDT = {"PDT", Second, Sun, Mar, 2, -420};
TimeChangeRule usPST = {"PST", First, Sun, Nov, 2, -480};
Timezone usPT(usPDT, usPST);
TimeChangeRule utcRule = {"UTC", Last, Sun, Mar, 1, 0};     // UTC
Timezone UTC(utcRule);
void setup()
{
  pinMode(pResistor, INPUT);
  //    Serial.begin(115200);
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.clear();

  strip.setBrightness(150); // Set BRIGHTNESS to about 1/5 (max = 255)
  strip.setPixelColor(42, strip.Color(255, 255, 255));
  strip.setPixelColor(43, strip.Color(255, 255, 255));
  strip.show();
  getNTPTime();
}

void loop() {

  //so we now have an RTC going with UTC.  now every tick, we need to get two separate times, one for PST and GMT and then pass those times off onto the output function
  if (secondcounter == 86400) {
    //this should make sure the time is upto date every day
    getNTPTime();
  }
  gettimeasstringdigits(rtc.getHours(), rtc.getMinutes(), rtc.getSeconds());
  
  //here's the Photoresistor stuff.  Lightvalue is the value given by the photoresistor, and based on different thresholds, adjust the strip brightness.
  lightvalue = analogRead(pResistor);
  if (lightvalue > 700) {
    strip.setBrightness(255);
    strip.show();
  }
  if ((lightvalue > 450) && (lightvalue < 700)) {
    strip.setBrightness(150);
    strip.show();
  }
  if (lightvalue < 450) {
    strip.setBrightness(20);
    strip.show();
  }
  secondcounter++;
  delay(1000);
}

void printArray( const int a[][ columns ] ) {
  // loop through array's rows
  for ( int i = 0; i < rows; ++i ) {
    // loop through columns of current row
    for ( int j = 0; j < columns; ++j )
      //      Serial.print (a[ i ][ j ] );
      Serial.print ("\r" ) ; // start new line of output
  }
  // end outer for
}
void gettimeasstringdigits(byte hours, byte minutes, byte seconds) {
  char bufPST[40];
  char mPST[4];
  char bufUK[40];
  char mUK[4];
  TimeChangeRule *tcr;

  time_t tUK = UK.toLocal(rtc.getEpoch(), &tcr);
  time_t tPST = usPT.toLocal(rtc.getEpoch(), &tcr);
  char PSTHour[3];
  char PSTMin[3];
  char PSTSec[3];
  char UKHour[3];
  char UKMin[3];
  char UKSec[3];

  strcpy(mUK, monthShortStr(month(tUK)));
  sprintf(bufUK, "%.2d:%.2d:%.2d %s %.2d %s %d %s", hour(tUK), minute(tUK), second(tUK), dayShortStr(weekday(tUK)), day(tUK), mUK, year(tUK), tcr -> abbrev);
  strcpy(mPST, monthShortStr(month(tPST)));
  sprintf(bufPST, "%.2d:%.2d:%.2d %s %.2d %s %d %s", hour(tPST), minute(tPST), second(tPST), dayShortStr(weekday(tPST)), day(tPST), mPST, year(tPST), tcr -> abbrev);
  if (minutevalue != rtc.getMinutes()) {
    //this is to avoid the flicker than I saw when it was updating every second.  Basically as far as I can tell, I need to clear the strip before changing the numbers, else the old number remains
    //well... doing that every second introduced a noticable flicker every second.  so now instead of updating the display every second, I'm doing it every
    //minute.
    strip.clear();
    minutevalue = rtc.getMinutes();
    sprintf(PSTHour, "%.2d", hour(tPST));
    sprintf(PSTMin, "%.2d", minute(tPST));
    sprintf(PSTSec, "%.2d", second(tPST));
    sprintf(UKHour, "%.2d", hour(tUK));
    sprintf(UKMin, "%.2d", minute(tUK));
    sprintf(UKSec, "%.2d", second(tUK));

    //this is very convoluted, probably much better ways of doing this.  but essentially all this is here to split the time out into 4 separate digits.
    String PSTHours = String(PSTHour);
    String PSTMins = String(PSTMin);
    String PSTSeconds = String(PSTSec);
    String UKHours = String(UKHour);
    String UKMins = String(UKMin);
    String UKSeconds = String(UKSec);

    String PSTMinute1 = String(PSTMin).substring(0, 1);
    String PSTMinute2 = String(PSTMin).substring(1);
    String UKMinute1 = String(UKMin).substring(0, 1);
    String UKMinute2 = String(UKMin).substring(1);

    String PSTHour1 = String(PSTHour).substring(0, 1);
    String PSTHour2 = String(PSTHour).substring(1);
    String UKHour1 = String(UKHour).substring(0, 1);
    String UKHour2 = String(UKHour).substring(1);

    //now that the time's been split up, we can output it all to the digits of the clock.
    //I'm doing each digit separately.  again as with before, there's probably way better ways of doing this.
    outputdigit (PSTMinute2.toInt(), 0, 150, 0, 0);
    outputdigit (PSTMinute1.toInt(), 21, 150, 150, 0);
    outputdigit (PSTHour2.toInt(), 44, 0, 150, 0);
    outputdigit (PSTHour1.toInt(), 65, 0, 0, 150);
    strip.setPixelColor(42, strip.Color(150, 0, 150));
    strip.setPixelColor(43, strip.Color(150, 0, 150));
    strip.show();
  }

}
void outputdigit(int digit, int offset, int redvalue, int greenvalue, int bluevalue) {
  //This is where we actually set the digits.  we take in the number to display.  the Offset is the pixel start value of the specific digit
  //so the first digit starts with pixel 0, but the second digit will start with pixel 21
  //so to avoid complications, I just take that offset value and add it to the value from the numbers array, so we can get the right start digit each time
  //Redvalue, Greenvalue and BlueValue, are fairly self explanatory I think.  The RGB values for each pixel.
  switch (digit) {
    case 0:
      for (int wi = 0; wi < Sized[digit]; wi++) {
        strip.setPixelColor(number0[wi] + offset, strip.Color(redvalue, greenvalue, bluevalue));
      }
      strip.show(); // Update strip with new contents
      break;
    case 1:
      for (int wi = 0; wi < Sized[digit]; wi++) {
        strip.setPixelColor(number1[wi] + offset, strip.Color(redvalue, greenvalue, bluevalue));
      }
      strip.show(); // Update strip with new contents
      break;
    case 2:
      for (int wi = 0; wi < Sized[digit]; wi++) {
        strip.setPixelColor(number2[wi] + offset, strip.Color(redvalue, greenvalue, bluevalue));
      }
      strip.show(); // Update strip with new contents
      break;
    case 3:
      for (int wi = 0; wi < Sized[digit]; wi++) {
        strip.setPixelColor(number3[wi] + offset, strip.Color(redvalue, greenvalue, bluevalue));
      }
      strip.show(); // Update strip with new contents
      break;
    case 4:
      for (int wi = 0; wi < Sized[digit]; wi++) {
        strip.setPixelColor(number4[wi] + offset, strip.Color(redvalue, greenvalue, bluevalue));
      }
      strip.show(); // Update strip with new contents
      break;
    case 5:
      for (int wi = 0; wi < Sized[digit]; wi++) {
        strip.setPixelColor(number5[wi] + offset, strip.Color(redvalue, greenvalue, bluevalue));
      }
      strip.show(); // Update strip with new contents
      break;
    case 6:
      for (int wi = 0; wi < Sized[digit]; wi++) {
        strip.setPixelColor(number6[wi] + offset, strip.Color(redvalue, greenvalue, bluevalue));
      }
      strip.show(); // Update strip with new contents
      break;
    case 7:
      for (int wi = 0; wi < Sized[digit]; wi++) {
        strip.setPixelColor(number7[wi] + offset, strip.Color(redvalue, greenvalue, bluevalue));
      }
      strip.show(); // Update strip with new contents
      break;
    case 8:
      for (int wi = 0; wi < Sized[digit]; wi++) {
        strip.setPixelColor(number8[wi] + offset, strip.Color(redvalue, greenvalue, bluevalue));
      }
      strip.show(); // Update strip with new contents
      break;
    case 9:
      for (int wi = 0; wi < Sized[digit]; wi++) {
        strip.setPixelColor(number9[wi] + offset, strip.Color(redvalue, greenvalue, bluevalue));
      }
      strip.show(); // Update strip with new contents
      break;
  }
}
void getNTPTime() {
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();
  Serial.println("\nStarting connection to server...");
  Udp.begin(localPort);
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  delay(1000);
  secondcounter = 0; //reset the counter, so we always get the correct time every 24 hrs
  if (Udp.parsePacket()) {
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    const unsigned long seventyYears = 2208988800UL;
    unsigned long epoch = secsSince1900 - seventyYears;
    SetDateTime(usPT, epoch, " Los Angeles");
  }
}
// given a Timezone object, UTC and a string description, convert and print local time with time zone
void SetDateTime(Timezone tz, time_t utc, const char *descr) {
  strip.clear();
  strip.setPixelColor(42, strip.Color(0, 255, 0));
  strip.setPixelColor(43, strip.Color(0, 255, 0));
  strip.show();
  rtc.begin(); // initialize RTC
  rtc.setEpoch(utc);
  minutevalue = rtc.getMinutes();

}
//
//void print2digits(int number) {
//
//  if (number < 10) {
//
//    Serial.print("0"); // print a 0 before if the number is < than 10
//
//  }
//
//  Serial.print(number);
//}
unsigned long sendNTPpacket(IPAddress& address) {

  Serial.println("1");

  strip.clear();
  strip.setPixelColor(42, strip.Color(0, 0, 0));
  strip.setPixelColor(43, strip.Color(0, 0, 0));
  strip.show();
  // set all bytes in the buffer to 0

  memset(packetBuffer, 0, NTP_PACKET_SIZE);

  // Initialize values needed to form NTP request

  // (see URL above for details on the packets)

  Serial.println("2");

  strip.clear();
  strip.setPixelColor(42, strip.Color(100, 0, 0));
  strip.setPixelColor(43, strip.Color(100, 0, 0));
  strip.show();
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode

  packetBuffer[1] = 0;     // Stratum, or type of clock

  packetBuffer[2] = 6;     // Polling Interval

  packetBuffer[3] = 0xEC;  // Peer Clock Precision

  packetBuffer[12]  = 49;

  packetBuffer[13]  = 0x4E;

  packetBuffer[14]  = 49;

  packetBuffer[15]  = 52;

  Serial.println("3");
  strip.clear();
  strip.setPixelColor(42, strip.Color(255, 0, 0));
  strip.setPixelColor(43, strip.Color(255, 0, 0));
  strip.show();
  // all NTP fields have been given values, now

  // you can send a packet requesting a timestamp:

  Udp.beginPacket(address, 123); //NTP requests are to port 123

  Serial.println("4");
  strip.clear();
  strip.setPixelColor(42, strip.Color(255, 100, 0));
  strip.setPixelColor(43, strip.Color(255, 100, 0));
  strip.show();

  Udp.write(packetBuffer, NTP_PACKET_SIZE);

  Serial.println("5");
  strip.clear();
  strip.setPixelColor(42, strip.Color(255, 255, 0));
  strip.setPixelColor(43, strip.Color(255, 255, 0));
  strip.show();
  Udp.endPacket();

  Serial.println("6");
  strip.clear();
  strip.setPixelColor(42, strip.Color(255, 250, 100));
  strip.setPixelColor(43, strip.Color(255, 255, 100));
  strip.show();
}
void printWifiStatus() {

  // print the SSID of the network you're attached to:

  Serial.print("SSID: ");

  Serial.println(WiFi.SSID());

  // print your board's IP address:

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");

  Serial.println(ip);

  // print the received signal strength:

  long rssi = WiFi.RSSI();

  Serial.print("signal strength (RSSI):");

  Serial.print(rssi);

  Serial.println(" dBm");
  //  if (Udp.parsePacket()) {
  //
  //    Serial.println("packet received");
  //    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
  //    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  //
  //    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
  //    unsigned long secsSince1900 = highWord << 16 | lowWord;
  //    const unsigned long seventyYears = 2208988800UL;
  //    unsigned long epoch = secsSince1900 - seventyYears;
  //
  //  }
}
