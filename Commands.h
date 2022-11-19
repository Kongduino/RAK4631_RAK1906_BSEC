#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
using namespace Adafruit_LittleFS_Namespace;

void handleCommand(char*);
void handleHelp(char *);
void evalCmd(char *, char *);
void handleBsecFreq(char *);
void handleIAQFreq(char *);
void handleSave(char *);
void pollSensor();
void handlePoll(char *);

void updateState(void);
void loadState(void);
void flash_reset(void);
void init_flash(void);
void hexDump(unsigned char *, uint16_t);

static const char bsec_name[] = "RAK_BSEC";
File file(InternalFS);
uint8_t bsecState[BSEC_MAX_STATE_BLOB_SIZE] = {0};
// Create an object of the class Bsec
Bsec iaqSensor;
uint32_t saveInterval = 3600000, iaqInterval = 10000;
double lastSave;
uint32_t t0;

int cmdCount = 0;
struct myCommand {
  void (*ptr)(char *); // Function pointer
  char name[12];
  char help[48];
};

myCommand cmds[] = {
  {handleHelp, "help", "Shows this help."},
  {handleBsecFreq, "bsec_fq", "Gets/sets the save interval in seconds."},
  {handleIAQFreq, "iaq_fq", "Gets/sets the IAQ polling in seconds."},
  {handleSave, "save", "Saves BSEC status."},
  {handlePoll, "poll", "Polls the BME680."},
};

void handleHelp(char *param) {
  Serial.printf("Available commands: %d\n", cmdCount);
  for (int i = 0; i < cmdCount; i++) {
    Serial.printf(" . %s: %s\n", cmds[i].name, cmds[i].help);
  }
  Serial.println("\n\n");
}

void handleSave(char *param) {
  updateState();
}

void evalCmd(char *str, char *fullString) {
  uint8_t ix, iy = strlen(str);
  Serial.println(fullString);
  for (ix = 0; ix < iy; ix++) {
    char c = str[ix];
    // lowercase the keyword
    if (c >= 'A' && c <= 'Z') str[ix] = c + 32;
  }
  //Serial.printf("Evaluating: `%s` [%d]\n", fullString, cmdCount);
  for (int i = 0; i < cmdCount; i++) {
    if (strcmp(str, cmds[i].name) == 0) {
      cmds[i].ptr(fullString);
      return;
    }
  }
  Serial.printf("Unknown command: `%s`\n", str);
  handleHelp("");
}

void handleCommand(char* str1) {
  char kwd[32];
  int i = sscanf(str1, "/%s", kwd);
  if (i > 0) evalCmd(kwd, str1);
}

void handleBsecFreq(char *param) {
  if (strcmp("/bsec_fq", param) == 0) {
    // no parameters
    Serial.printf("Save Interval: every %d s\n", (saveInterval / 1000));
    return;
  } else {
    // fq xxx.xxx set frequency
    uint32_t value = atoi(param + 8);
    // for some reason sscanf returns 0.000 as value...
    if (value < 60) {
      // Less than 60 seconds is going to kill your Flash
      Serial.printf("Invalid save interval: %d, %s\n", value, param);
      return;
    }
    saveInterval = value * 1000;
    Serial.printf("* Save interval set to %d s\n", (saveInterval / 1000));
  }
}

void handleIAQFreq(char *param) {
  if (strcmp("/iaq_fq", param) == 0) {
    // no parameters
    Serial.printf("Polling Interval: every %d s\n", (iaqInterval / 1000));
    return;
  } else {
    // fq xxx.xxx set frequency
    uint32_t value = atoi(param + 7);
    iaqInterval = value * 1000;
    Serial.printf("* Polling Interval set to %d s\n", (iaqInterval / 1000));
  }
}

void handlePoll(char *param) {
  Serial.println("Force-polling the BME680:");
  pollSensor();
}

void pollSensor() {
  if (iaqSensor.run()) {
    // If new data is available
    unsigned long time_trigger = millis();
    digitalWrite(BLUE_LED, HIGH);
    Serial.printf("Timestamp: %.2f secs\n", time_trigger / 1000.0);
    Serial.printf(" . Raw Temperature: %.2f C\n", iaqSensor.rawTemperature);
    Serial.printf(" . Pressure: %.2f HPa\n", iaqSensor.pressure / 100.0);
    Serial.printf(" . Raw Humidity: %.2f%%\n", iaqSensor.rawHumidity);
    Serial.printf(" . Gas Resistance: %d\n", iaqSensor.gasResistance);
    Serial.printf(" . IAQ: %.2f\n", iaqSensor.iaq);
    Serial.printf(" . IAQ Accuracy: %.2f\n", iaqSensor.iaqAccuracy);
    Serial.printf(" . Temperature: %.2f C\n", iaqSensor.temperature);
    Serial.printf(" . Humidity: %.2f%%\n", iaqSensor.humidity);
    Serial.printf(" . Static IAQ: %.2f\n", iaqSensor.staticIaq);
    Serial.printf(" . CO2 Equivalent: %.2f\n", iaqSensor.co2Equivalent);
    Serial.printf(" . Breath VOC EquivalentQ: %.2f\n\n", iaqSensor.breathVocEquivalent);
    if (lastSave == 0 || millis() - lastSave > saveInterval) {
      updateState();
      lastSave = millis();
    }
    delay(500);
    digitalWrite(BLUE_LED, LOW);
  } else {
    digitalWrite(GREEN_LED, HIGH);
    checkIaqSensorStatus();
    delay(500);
    digitalWrite(GREEN_LED, LOW);
  }
  t0 = millis();
}

/**
   @brief Reset content of the filesystem
*/
void flash_reset(void) {
  Serial.println("File doesn't exist, forcing format!");
  delay(100);
  InternalFS.format();
  updateState();
  file.open(bsec_name, FILE_O_READ);
}

/**
   @brief Initialize access to nRF52 internal file system
*/
void init_flash(void) {
  Serial.println("init_flash");
  // Initialize Internal File System
  InternalFS.begin();
  // Check if file exists
  file.open(bsec_name, FILE_O_READ);
  if (!file) flash_reset();
  loadState();
}

void loadState(void) {
  Serial.println("loadState:");
  file.read((uint8_t *)&bsecState, BSEC_MAX_STATE_BLOB_SIZE);
  file.close();
  hexDump(bsecState, BSEC_MAX_STATE_BLOB_SIZE);
}

void updateState(void) {
  Serial.println("updateState");
  iaqSensor.getState(bsecState);
  checkIaqSensorStatus();
  Serial.print("Writing state to Flash... ");
  if (file.open(bsec_name, FILE_O_WRITE)) {
    file.write((uint8_t *)&bsecState, BSEC_MAX_STATE_BLOB_SIZE);
    file.flush();
    file.close();
    Serial.println("done!");
    hexDump(bsecState, BSEC_MAX_STATE_BLOB_SIZE);
  } else {
    Serial.printf("Unable to open file `%s` for output!\n", bsec_name);
    return;
  }
}

void hexDump(unsigned char *buf, uint16_t len) {
  char alphabet[17] = "0123456789abcdef";
  Serial.print(F("   +------------------------------------------------+ +----------------+\n"));
  Serial.print(F("   |.0 .1 .2 .3 .4 .5 .6 .7 .8 .9 .a .b .c .d .e .f | |      ASCII     |\n"));
  for (uint16_t i = 0; i < len; i += 16) {
    if (i % 128 == 0)
      Serial.print(F("   +------------------------------------------------+ +----------------+\n"));
    char s[] = "|                                                | |                |\n";
    uint8_t ix = 1, iy = 52;
    for (uint8_t j = 0; j < 16; j++) {
      if (i + j < len) {
        uint8_t c = buf[i + j];
        s[ix++] = alphabet[(c >> 4) & 0x0F];
        s[ix++] = alphabet[c & 0x0F];
        ix++;
        if (c > 31 && c < 128) s[iy++] = c;
        else s[iy++] = '.';
      }
    }
    uint8_t index = i / 16;
    if (i < 256) Serial.write(' ');
    Serial.print(index, HEX); Serial.write('.');
    Serial.print(s);
  }
  Serial.print(F("   +------------------------------------------------+ +----------------+\n"));
}
