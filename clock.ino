/*
  Simple Arduino NTP clock for Arduino Uno with ENC28J60 ethernet
  module and HD44780 LCD module
*/
#include <LiquidCrystal.h>
#include <Time.h>
#include "EtherCard.h"

LiquidCrystal lcd(6, 7, 2, 3, 4, 5);

uint32_t last_update;
uint32_t last_time;
char *days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

static byte macaddr[] = {0x74, 0x69, 0x69, 0x2d, 0x30, 0x32};
byte Ethernet::buffer[700];
const static char ntpserver[] PROGMEM = "ntp2d.mcc.ac.uk";

void setup()
{
  Serial.begin(9600);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hello World!");

  if(!ether.begin(sizeof Ethernet::buffer, macaddr)){
    Serial.println("Error: accessing ethernet adapter");
  }

  if(!ether.dhcpSetup()){
    Serial.println("Error: DHCP fail");
  }
  else {
    ether.printIp("IP: ", ether.myip);
    ether.printIp("GW: ", ether.gwip);
    ether.printIp("DNS: ", ether.dnsip);
  }

  if(!ether.dnsLookup(ntpserver)){
    Serial.println("Error: DNS lookup");
  }
  else {
    ether.printIp("SERVER: ", ether.hisip);
  }

  ether.ntpRequest(ether.hisip, 9123);
  last_update = millis();
}

void loop()
{
int length;
uint32_t ntptime;
char buffer[50];

  length = ether.packetReceive();
  ether.packetLoop(length);

  if((length>0)&&ether.ntpProcessAnswer(&ntptime, 0)){
    Serial.println("Received NTP reply");
    setTime(ntptime-2208988800UL);
  }
  if(last_time!=now()){
    last_time = now();

    lcd.clear();
    lcd.setCursor(4, 0);
    sprintf(buffer, "%02d:%02d:%02d", hour(), minute(), second());
    lcd.print(buffer);
    lcd.setCursor(1, 1);
    sprintf(buffer, "%s %02d-%s-%04d", days[weekday()-1], day(), months[month()-1], year());
    lcd.print(buffer);
  }


  if(last_update+60000L<millis()){
    last_update = millis();
    ether.ntpRequest(ether.hisip, 9123);
  }
}
