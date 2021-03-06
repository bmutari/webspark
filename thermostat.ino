//Particle Thermostat
//Brandon Mutari

//Library Inludes
#include "HIH61XX/HIH61XX.h" //Temperature and humidity sensor
#include "Adafruit_LEDBackpack/Adafruit_LEDBackpack.h" //LED matrices

//Define address of temperature sensor
HIH61XX hih(0x27);

//Setup global variables
double tempF = 0.0;
double hum = 0.0;
double tcal =-2.0; //Calibration for temperature sensor
int settemp = 55;
int furnace = D2;
bool altschd = FALSE;

//Setup matrices
Adafruit_8x8matrix matrix1 = Adafruit_8x8matrix ();
Adafruit_8x8matrix matrix2 = Adafruit_8x8matrix ();
Adafruit_8x8matrix matrix3 = Adafruit_8x8matrix ();

//Function to get the temperature set point
int getthesetpoint(String newset) {
    settemp = newset.toInt();
    displaytxt("Temp now set to " + newset);
    altschd = FALSE;
    return 1;
    }

//Function to scroll information on LED matrices
int displaytxt(String txt){
    int x = 24; //Length in px of the 3 matrices
    int textCount = -6 * txt.length(); //Each character is 6px, so -6 * number of characters is the start position
    int textLoop = textCount - x;

    for (int i=0; i>textLoop; i--){
        // Draw message in each matrix buffer, offseting each by 8 pixels
        matrix1.clear();
        matrix1.setCursor(x - 0, 0);
        matrix1.print(txt);

        matrix2.clear();
        matrix2.setCursor(x - 8, 0);
        matrix2.print(txt);

        matrix3.clear();
        matrix3.setCursor(x - 16, 0);
        matrix3.print(txt);

        //Write data to matrices in separate loop so it's less jumpy
        matrix1.writeDisplay();
        matrix2.writeDisplay();
        matrix3.writeDisplay();

        // Move text position left by 1 pixel.
        x--;
        delay(50);
        }
    return 1;
}


void setup() {
    //Particle functions and variables
    Particle.variable("temp", tempF);
    Particle.variable("hum", hum);
    Particle.function("getsetpoint", getthesetpoint);

    //Start serial com
    Serial.begin(9600);

    //Start I2C com
    Wire.begin();

    //Setup furnace control
    pinMode(furnace, OUTPUT);
    digitalWrite(furnace, LOW);

    //Start matrices
    matrix1.begin(0x70); //Address defined by solder pad on LED backpack
    matrix1.setTextWrap(false); // Allow text to run off edges
    matrix1.clear();

    matrix2.begin(0x71);
    matrix2.setTextWrap(false);
    matrix2.clear();

    matrix3.begin(0x72);
    matrix3.setTextWrap(false);
    matrix3.clear();

    //Set brightness levels
    matrix1.setBrightness(0.5);
    matrix2.setBrightness(0.5);
    matrix3.setBrightness(0.5);
}


void loop() {
    hih.start(); //Start the temperature and humidity sensor
    hih.update(); //request an update of the humidity and temperature

    tempF = hih.temperature() * 1.8 + 32 + tcal; //The HIH61XX library defaults to C, convert to F
    hum = hih.humidity() * 100;

    Serial.print("Hum: ");
    Serial.print(hum);
    Serial.print("% (");
    Serial.print(hih.humidity_Raw());
    Serial.println(")");

    Serial.print("Temp: ");
    Serial.print(tempF);
    Serial.print(" F (");
    Serial.print(hih.temperature_Raw());
    Serial.println(")");

    Serial.print("Set Temp: ");
    Serial.println(settemp);

    String tempstg = String(tempF,1); //Convert double to a string with 1 decimal place to display on the LED matrices

    //Select each character to display
    char tens = tempstg.charAt(0);
    char ones = tempstg.charAt(1);
    char tenths = tempstg.charAt(3);

    matrix1.clear();
    matrix1.setCursor(2, 0);
    matrix1.write(tens);

    matrix2.clear();
    matrix2.setCursor(0, 0);
    matrix2.write(ones);
    matrix2.drawRect(6,5, 2,2, LED_ON);

    matrix3.clear();
    matrix3.setCursor(1, 0);
    matrix3.write(tenths);

    matrix1.writeDisplay();
    matrix2.writeDisplay();
    matrix3.writeDisplay();

    //Thermostat logic
    if (tempF < settemp) {
        digitalWrite(furnace,HIGH);
    }
    if (tempF > settemp) {
        digitalWrite(furnace,LOW);
    }

    delay(250);
}
