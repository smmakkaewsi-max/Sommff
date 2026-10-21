#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
void setup() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(20, 20);
  tft.println("WAVE125i");
  tft.setTextSize(2);
  tft.setCursor(20, 60);
  tft.setTextColor(TFT_WHITE);
  tft.println("Ultimate Ready!");
  tft.setCursor(20, 90);
  tft.setTextColor(TFT_YELLOW);
  tft.println("【entity-GitHub¦canonical_name=GitHub】 Build OK");
}
void loop() {
  tft.invertDisplay(false);
  delay(1000);
  tft.invertDisplay(true);
  delay(1000);
}
