#include <SPI.h>
#include <Ethernet.h>
#include <HttpClient.h>
#include <Xively.h>
#include <OneWire.h>


// MAC address for your Ethernet shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34 };

// Your Xively key to let you upload data
char xivelyKey[] = "M7HPuPwbLjscak8W7VxBTM630lhPwCJRKb8CBEmnYzJPnxGP"; // enter the key, under API Keys
unsigned long feedId = 673942471; // enter your feed ID, under Activated
int frequency = 15000; // delay between updates (milliseconds)

// Analog pin which we're monitoring (0 and 1 are used by the Ethernet shield)
int sensorPin = 9;

// Define the strings for our datastream IDs
char sensorId[] = "temp";
XivelyDatastream datastreams[] = {
  XivelyDatastream(sensorId, strlen(sensorId), DATASTREAM_FLOAT),
};
// Finally, wrap the datastreams into a feed
XivelyFeed feed(feedId, datastreams, 1 /* number of datastreams */);

EthernetClient client;
XivelyClient xivelyclient(client);

// initialize the one-wire interface
OneWire ds(sensorPin);  // on pin 2 (a 4.7K resistor is necessary)
/*
5v - 4.7k resistor - 18B20 middle pin - D2
gnd - 18B20 both legs (joined together)
*/

byte addr[8];
byte type_s;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
 
  Serial.println("Starting single datastream upload to Xively...");
  Serial.println();

  while (Ethernet.begin(mac) != 1)
  {
    Serial.println("Error getting IP address via DHCP, trying again...");
    delay(5000);
  }
  Serial.print("IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++)
  {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }
  Serial.println();
}


float getTemp() {
  byte data[12];
  byte present = 0;
  byte i;
  
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);      
  delay(1000);     
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);        
  for ( i = 0; i < 9; i++) {          
    data[i] = ds.read();
  }
  
//  Преобразование данных в фактическую температуру
 
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; 
    if (data[7] == 0x10) {
        raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw & ~7;  
    else if (cfg == 0x20) raw = raw & ~3;
    else if (cfg == 0x40) raw = raw & ~1; 
 
  }
  return (float)raw / 16.0;
}
 


void loop() {
  
  float celsius, fahrenheit;
   
  if ( !ds.search(addr)) {
    ds.reset_search();
    delay(250);
    return;
  }
 
  if (OneWire::crc8(addr, 7) != addr[7]) {
      return;
  }
// Первый байт ROM указывает, какой чип
  switch (addr[0]) {
    case 0x10:
      type_s = 1;
      break;
    case 0x28:
      type_s = 0;
      break;
    case 0x22:
      type_s = 0;
      break;
    default:
      return;
  } 
 
  float temp = getTemp() + getTemp();
  temp /= 2;

  datastreams[0].setFloat( temp );
  Serial.println("Uploading it to Xively");
  int ret = xivelyclient.put(feed, xivelyKey);
  Serial.print("xivelyclient.put returned ");
  Serial.println(ret);

  Serial.println();
  delay(frequency);
}
