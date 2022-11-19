// Helper functions declarations
void checkIaqSensorStatus(void);
void errLeds(void);

#include "bsec.h"
#include "Commands.h"

const uint8_t bsec_config_iaq[] = {
#include "config/generic_33v_3s_4d/bsec_iaq.txt"
};

void setup(void) {
  Serial.begin(115200);
  t0 = millis();
  while (!Serial && millis() - t0 < 5000) delay(100);
  Serial.println("\n\nLet's go!");
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  digitalWrite(GREEN_LED, HIGH);
  cmdCount = sizeof(cmds) / sizeof(myCommand);
  Wire.begin();
  iaqSensor.begin(BME680_I2C_ADDR_PRIMARY, Wire);
  Serial.println(
    "\nBSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) +
    "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix));
  checkIaqSensorStatus();
  iaqSensor.setConfig(bsec_config_iaq);
  checkIaqSensorStatus();
  init_flash();
  bsec_virtual_sensor_t sensorList[10] = {
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
  };
  iaqSensor.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_LP);
  checkIaqSensorStatus();
  lastSave = 0;
  delay(500);
  digitalWrite(GREEN_LED, LOW);
  t0 = millis();
}

// Function that is looped forever
void loop(void) {
  if (Serial.available()) {
    // incoming from user
    char incoming[256];
    memset(incoming, 0, 256);
    uint8_t ix = 0;
    while (Serial.available()) {
      char c = Serial.read();
      delay(25);
      if (c == 13 || c == 10) {
        // cr / lf
        if (ix > 0) {
          incoming[ix] = 0;
          handleCommand(incoming);
          ix = 0;
        }
      } else incoming[ix++] = c;
    }
  }
  if (millis() - t0 < iaqInterval) return;
  pollSensor();
}

// Helper function definitions
void checkIaqSensorStatus(void) {
  if (iaqSensor.status != BSEC_OK) {
    if (iaqSensor.status < BSEC_OK) {
      Serial.println("BSEC error code : " + String(iaqSensor.status));
      for (;;)
        errLeds(); /* Halt in case of failure */
    } else {
      Serial.println("BSEC warning code : " + String(iaqSensor.status));
    }
  }
  if (iaqSensor.bme680Status != BME680_OK) {
    if (iaqSensor.bme680Status < BME680_OK) {
      Serial.println("BME680 error code : " + String(iaqSensor.bme680Status));
      for (;;)
        errLeds(); /* Halt in case of failure */
    } else {
      Serial.println("BME680 warning code : " + String(iaqSensor.bme680Status));
    }
  }
}

void errLeds(void) {
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(BLUE_LED, LOW);
  delay(500);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BLUE_LED, HIGH);
  delay(500);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
  delay(500);
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(BLUE_LED, HIGH);
  delay(500);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
  delay(500);
}
