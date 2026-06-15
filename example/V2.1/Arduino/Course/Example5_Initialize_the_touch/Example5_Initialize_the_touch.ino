#include <TFT_eSPI.h>
TFT_eSPI lcd = TFT_eSPI();
uint16_t touchX, touchY;
uint16_t calData[5] = { 557, 3263, 369, 3493, 3 };

void setup() {
  // put your setup code here, to run once:
  Serial.begin( 9600 ); /*Initialize serial port*/
  //LCD Initialize
  lcd.begin(); 
  lcd.setRotation(1); /* Rotate */
  //Calibration mode. One is four-corner positioning, and the other is direct positioning by directly inputting analog values.
  //Screen calibration
//  touch_calibrate();
  lcd.setTouch(calData);

  /*Initialization*/
}

void loop() {
  // put your main code here, to run repeatedly:
  bool touched = lcd.getTouch( &touchX, &touchY, 600);
  if ( touched )
  {
    Serial.print( "Data x " );
    Serial.println( touchX );

    Serial.print( "Data y " );
    Serial.println( touchY );
  }
}

void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;
  Serial.println("Touch-screen calibration");

  //calibration
  Serial.println("Please touch the corners as directed");

 // lv_timer_handler();
  lcd.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
  Serial.println("calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15)");
  Serial.println(); Serial.println();
  Serial.println("//Use this calibration code in setup():");
  Serial.print("uint16_t calData[5] = ");
  Serial.print("{ ");

  for (uint8_t i = 0; i < 5; i++)
  {
    Serial.print(calData[i]);
    if (i < 4) Serial.print(", ");
  }

  Serial.println(" };");
  Serial.print("  tft.setTouch(calData);");
  Serial.println(); Serial.println();


}