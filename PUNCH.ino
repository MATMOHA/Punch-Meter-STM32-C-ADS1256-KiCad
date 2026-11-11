#include <SPI.h>
#include <TM1637Display.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// SPI Pins for ADS1256
SPIClass mySPI(PA7, PA6, PA5);
const int CS_PIN = 10;   // D10
const int DRDY_PIN = 9;  // D9

// TM1637 Display Pins
const int CLK_PIN = 4;
const int DIO_PIN = 5;
TM1637Display display(CLK_PIN, DIO_PIN);

// DFPlayer
// A0 = Nucleo receives (from DFPlayer TX)
// A1 = Nucleo sends (to the 1k resistor, then DFPlayer RX)
SoftwareSerial dfSerial(A0, A1);
DFRobotDFPlayerMini myDFPlayer;

long zeroOffset = 0;
float calibrationFactor = 891.5;

// Variables for peak tracking
float peakForce = 0.0;
bool isStriking = false;
unsigned long strikeStartTime = 0;
const unsigned long strikeWindowMs = 150;

void setup() {
  Serial.begin(115200);
  delay(1500); // Allow serial to wake up safely

  // Initialize Display
  display.setBrightness(5);
  display.showNumberDec(0); // Show 0 on startup

  // Initialize DFPlayer
  dfSerial.begin(9600);
  Serial.println("Starting DFPlayer...");
  if (!myDFPlayer.begin(dfSerial)) {
    Serial.println("DFPlayer NOT found. Check wires and SD card.");
    while (true) {}
  }
  myDFPlayer.setTimeOut(300);
  myDFPlayer.volume(30);   // 0 to 30
  myDFPlayer.EQ(0);        // normal sound

  pinMode(CS_PIN, OUTPUT);
  pinMode(DRDY_PIN, INPUT);
  digitalWrite(CS_PIN, HIGH);

  mySPI.begin();
  mySPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));

  Serial.println("Initializing ADS1256 Punch Meter...");

  // Send Reset Command
  digitalWrite(CS_PIN, LOW);
  mySPI.transfer(0xFE);
  digitalWrite(CS_PIN, HIGH);
  delay(10);

  // Set MUX Register (0x01): AIN0 (+) and AIN1 (-)
  writeRegister(0x01, 0x01);

  // Set ADCON Register (0x02): PGA Gain = 64
  writeRegister(0x02, 0x06);

  // Set DRATE Register (0x03): 100 SPS
  writeRegister(0x03, 0x82);

  // Perform Self Offset/Gain Calibration
  digitalWrite(CS_PIN, LOW);
  mySPI.transfer(0xF0); // SELFCAL
  digitalWrite(CS_PIN, HIGH);
  delay(100);

  mySPI.endTransaction();

  // TARE: Take an average of baseline readings at startup
  Serial.println("Taring scale... Keep load cell completely unloaded!");
  long sum = 0;
  for (int i = 0; i < 50; i++) {
    sum += readRawDirect();
    delay(10);
  }
  zeroOffset = sum / 50;
  Serial.print("Tare complete. Ready for punches! Zero Offset: ");
  Serial.println(zeroOffset);

  display.showNumberDec(8888); // Flash display to signal ready state
  delay(500);
  display.showNumberDec(0);
}

void loop() {
  long raw = readRawDirect();
  long adjustedRaw = raw - zeroOffset;

  float currentForce = (-1.0 * adjustedRaw) / calibrationFactor;

  // Filter out tiny noise (must exceed 3 lbs to count as a strike)
  if (currentForce < 3.0) {
    currentForce = 0.0;
  }

  if (currentForce > 3.0 && !isStriking) {
    // Strike detected! Start tracking peak
    isStriking = true;
    peakForce = currentForce;
    strikeStartTime = millis();
  }
  else if (isStriking) {
    // While the punch is happening, capture the maximum force
    if (currentForce > peakForce) {
      peakForce = currentForce;
    }

    // Once the impact window closes, lock in and report the score
    if (millis() - strikeStartTime > strikeWindowMs) {
      Serial.print(">>> PUNCH DETECTED! Peak Force: ");
      Serial.print(peakForce, 1);
      Serial.println(" lbs <<<\n");

      // Update the 7-segment display with the final peak force integer
      display.showNumberDec((int)peakForce);

      // Play the sound (change the number to pick the track)
      if (peakForce < 15) {
        myDFPlayer.play(1);
      } else if (peakForce <= 45) {
        myDFPlayer.play(2);
      } else{
        myDFPlayer.play(3);
      }

      // Reset for the next punch
      isStriking = false;
      peakForce = 0.0;
      delay(500); // Short breather before listening again
    }
  }
}

long readRawDirect() {
  long timeout = millis();
  while (digitalRead(DRDY_PIN) == HIGH) {
    if (millis() - timeout > 100) break;
  }

  mySPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
  digitalWrite(CS_PIN, LOW);

  mySPI.transfer(0x01); // RDATA command
  delayMicroseconds(5);

  long rawValue = 0;
  rawValue |= (long)mySPI.transfer(0xFF) << 16;
  rawValue |= (long)mySPI.transfer(0xFF) << 8;
  rawValue |= (long)mySPI.transfer(0xFF);

  digitalWrite(CS_PIN, HIGH);
  mySPI.endTransaction();

  // 24-bit sign extension
  if (rawValue & 0x800000) {
    rawValue |= 0xFF000000;
  }

  return rawValue;
}

void writeRegister(byte reg, byte value) {
  digitalWrite(CS_PIN, LOW);
  mySPI.transfer(0x50 | reg);
  mySPI.transfer(0x00);
  mySPI.transfer(value);
  digitalWrite(CS_PIN, HIGH);
}