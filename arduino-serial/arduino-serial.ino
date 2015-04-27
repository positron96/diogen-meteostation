#include <OneWire.h> 
 
OneWire  ds(9); 
 
byte addr[8];

byte type_s;
  
void setup(void) {
  Serial.begin(9600); 
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
 
void loop(void) { 
  
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
  Serial.println(temp);
  
  delay(5*1000);
}
