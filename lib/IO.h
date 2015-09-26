//-------------------------------------------------
// Gestion des Boutons
// -
class ClickButton
{
  public:
    ClickButton(void);
//    ClickButton(uint8_t buttonPin, boolean active);
//    ClickButton(uint8_t buttonPin, boolean active, boolean internalPullup);
    void Update();
    int clicks;                   // button click counts to return
    boolean pressed;            // the currently debounced button (press) state (presumably it is not sad :)
    unsigned long debounceTime;
    unsigned long multiclickTime;
    unsigned long longClickTime;
    unsigned long lastBounceTime;         // the last time the button input pin was toggled, due to noise or a press
  private:
    boolean _activeHigh;          // Type of button: Active-low = 0 or active-high = 1
    int _clickCount;              // Number of button clicks within multiclickTime milliseconds
    unsigned long _multiClick;
};
// - end
//-------------------------------------------------
// Capteur d'Humidite
// -
// how many timing transitions we need to keep track of. 2 * number bits + extra
#define MAXTIMINGS 85
class DHT {
	private:
		uint8_t data[6];
		uint8_t _pin, _count;
		unsigned long _lastreadtime;
		boolean firstreading;
		boolean read(void);

	public:
		DHT(uint8_t pin, uint8_t count=6);
		void begin(void);
		float getHumidity();
		float getTempCelcius();
};
// - end
//----------------------
class Temperature{
private:
    OneWire* ds;
//    byte data[12];
    byte resp[9];
    byte rom[8];
//    byte type_s;
//    byte chiptype;
//    char szName[MAX_NAME];
    
public:
    char szROM[80];
    Temperature(uint16_t pin);
    
//    boolean search();
//    void resetsearch();
    void getROM(char szROM[]);
//    byte getChipType();
//    char* getChipName();
    float getTemperature();
//    float convertToFahrenheit(float celsius);
};
