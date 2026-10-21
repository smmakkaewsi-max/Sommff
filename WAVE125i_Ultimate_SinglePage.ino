name: Build
on: [push]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: arduino/setup-arduino-cli@v1
      - name: Prepare and Build
        run: |
          arduino-cli core update-index
          arduino-cli core install esp32:esp32@2.0.17
          arduino-cli lib install "TFT_eSPI@2.5.43"
          mkdir -p /tmp/sketch/WAVE125i_WORKING
          cp WAVE125i_WORKING.ino /tmp/sketch/WAVE125i_WORKING/
          # Fix TFT_eSPI setup
          cat > ~/Arduino/libraries/TFT_eSPI/User_Setup.h << 'EOF'
          #define ST7789_DRIVER
          #define TFT_WIDTH 240
          #define TFT_HEIGHT 320
          #define TFT_MOSI 23
          #define TFT_SCLK 18
          #define TFT_CS 15
          #define TFT_DC 2
          #define TFT_RST 4
          #define TFT_BL 21
          #define TFT_BACKLIGHT_ON HIGH
          #define LOAD_GLCD
          #define LOAD_FONT2
          #define LOAD_FONT4
          #define LOAD_FONT6
          #define LOAD_FONT8
          #define SMOOTH_FONT
          #define SPI_FREQUENCY 27000000
          EOF
          mkdir -p build
          arduino-cli compile --fqbn esp32:esp32:esp32dev --output-dir build /tmp/sketch/WAVE125i_WORKING --verbose
          ls -R build
      - name: Upload BIN
        uses: actions/upload-artifact@v4
        with:
          name: WAVE125i-Ultimate-BIN
          path: build/*.bin
