#include "IO.h"


ClickButton::ClickButton(void)
{
  _activeHigh    = LOW;           // Assume active-low button
  _clickCount    = 0;
  clicks         = 0;
  pressed      = false;
  lastBounceTime= 0;
  debounceTime   = 20;            // Debounce timer in ms
  multiclickTime = 200;           // Time limit for multi clicks
  longClickTime  = 1000;          // time until long clicks register
}


void ClickButton::Update()
{
    long now = (unsigned long)millis();      // get current time
    clicks = 0;
    if ( pressed && _clickCount==0) _multiClick = lastBounceTime;
    if ( pressed) _clickCount++;
    if (!pressed && (now - _multiClick) > multiclickTime)
    {
        // positive count for released buttons
        clicks = _clickCount;
        _clickCount = 0;
    }
    pressed = false;
}
// -end Button
// ----------------------------------------------------------------
// DHT humidite et temperature
// -
DHT::DHT(uint8_t pin, uint8_t count) {
	_pin = pin;
	_count = count;
	firstreading = true;
}

void DHT::begin(void) {
// set up the pins!
	pinMode(_pin, INPUT);
	digitalWrite(_pin, HIGH);
	_lastreadtime = 0;
}
float DHT::getTempCelcius(void) {
	float f;
	if (read()) {
		f = data[2];
		return f;
    }
	return -1;
}
float DHT::getHumidity(void) {
	float f;
	if (read()) {
		f = data[0];
		return f;
    }
	return -1;
}
boolean DHT::read(void) {
	uint8_t laststate = HIGH;
	uint8_t counter = 0;
	uint8_t j = 0, i;
	unsigned long currenttime;

// Check if sensor was read less than two seconds ago and return early
// to use last reading.
	currenttime = millis();
	if (currenttime < _lastreadtime) {
// ie there was a rollover
		_lastreadtime = 0;
	}
	if (!firstreading && ((currenttime - _lastreadtime) < 2000)) {
		return true; // return last correct measurement
//		delay(2000 - (currenttime - _lastreadtime));
	}
	firstreading = false;
	_lastreadtime = millis();
	data[0] = data[1] = data[2] = data[3] = data[4] = 0;
// pull the pin high and wait 250 milliseconds
	digitalWrite(_pin, HIGH);
	delay(250);
// now pull it low for ~20 milliseconds
	pinMode(_pin, OUTPUT);
	digitalWrite(_pin, LOW);
	delay(20);
	noInterrupts();
	digitalWrite(_pin, HIGH);
	delayMicroseconds(40);
	pinMode(_pin, INPUT);
// read in timings
	for ( i=0; i< MAXTIMINGS; i++) {
		counter = 0;
		while (digitalRead(_pin) == laststate) {
			counter++;
			delayMicroseconds(1);
			if (counter == 255) {
				break;
			}
		}
		laststate = digitalRead(_pin);
		if (counter == 255) break;
// ignore first 3 transitions
		if ((i >= 4) && (i%2 == 0)) {
// shove each bit into the storage bytes
			data[j/8] <<= 1;
			if (counter > _count)
				data[j/8] |= 1;
			j++;
		}
	}
	interrupts();
// check we read 40 bits and that the checksum matches
	if ((j >= 40) && 
	   (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) ) {
		return true;
	}
	return false;
}
// - end Humidite et Temperature

Temperature::Temperature(uint16_t pin){
    ds = new OneWire(pin);
}


// Get the ROM address
void Temperature::getROM(char szROM[]) {
    ds->reset();
    ds->write(0x33);
    ds->read_bytes(rom, 8);
    sprintf(szROM, "%X %X %X %X %X %X %X %X", rom[0], rom[1], rom[2], rom[3], rom[4], rom[5], rom[6], rom[7]);
}

float Temperature::getTemperature() {
/*    ds->reset();
    ds->write(0x33);
    ds->read_bytes(rom, 8);
    sprintf(szROM, "%X%X%X%X%X%X%X%X", rom[0], rom[1], rom[2], rom[3], rom[4], rom[5], rom[6], rom[7]);
    delay(10);
*/
    ds->reset();
    ds->write(0x55);
    ds->write_bytes(rom,8);
    ds->write(0x44);        // start conversion, with parasite power on at the end
     
    delay(10);     // maybe 750ms is enough, maybe not
    // we might do a ds.depower() here, but the reset will take care of it.
 
    ds->reset();
    ds->write(0x55);
    ds->write_bytes(rom,8);
//    ds->select(addr);    
    ds->write(0xBE);         // Read Scratchpad
    ds->read_bytes(resp,9);

    // Convert the data to actual temperature
    // because the result is a 16 bit signed integer, it should
    // be stored to an "int16_t" type, which is always 16 bits
    // even when compiled on a 32 bit processor.
    byte MSB = resp[1];
    byte LSB = resp[0];

    int16_t intTemp = ((MSB << 8) | LSB); //using two's compliment 16-bit

/*
    if (type_s) {
        raw = raw << 3; // 9 bit resolution default
        if (data[7] == 0x10) {
            // "count remain" gives full 12 bit resolution
            raw = (raw & 0xFFF0) + 12 - data[6];
        }
    } else {
        byte cfg = (data[4] & 0x60);
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
        else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
        //// default is 12 bit resolution, 750 ms conversion time
    }
*/    
    return (float)((double)intTemp)/16.0;
}
