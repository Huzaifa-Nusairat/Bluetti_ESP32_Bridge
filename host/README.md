# Host tests

The firmware's unit tests run on a computer, not on the ESP32. `host/lib/` holds a stand-in for each
board-only library the firmware uses, so every source in `Bluetti_ESP32/` except the `.ino` compiles
and links on the host:

| Folder | Stands in for | A test can |
|---|---|---|
| `ArduinoHost` | Arduino-ESP32 core (String, Print, Serial, millis, FreeRTOS queues, sleep and Wi-Fi power calls) | set the clock, capture serial output |
| `NimBLEHost` | NimBLE-Arduino 1.4 client | advertise a device, refuse the connection or the service, deliver a notification, read every write |
| `PubSubClientHost` | PubSubClient 2.8 | refuse connect or publish, read what was published, deliver a message to the callback |
| `WiFiManagerHost` | WiFiManager 2.0 | make autoConnect fail, fill in portal parameters |
| `AsyncWebHost` | ESPAsyncWebServer, AsyncTCP, ElegantOTA | call a route and read the answer |
| `EEPROMHost` | Arduino-ESP32 EEPROM | erase it, see what was committed |
| `DisplayHost` | Adafruit GFX, Adafruit SSD1306, Wire, SPI | read printed text, count frames |

Nothing here talks to a radio, a network or a pin. Host-only controls start with `host_`. Each
stand-in carries only the calls this firmware makes.

## Run the tests

With PlatformIO:

    pio test -e native

With address and undefined-behaviour sanitizers (GCC or Clang, no PlatformIO needed):

    git clone --depth 1 --branch v2.6.0 https://github.com/ThrowTheSwitch/Unity /tmp/unity
    ASAN_OPTIONS=detect_leaks=0 host/run-native.sh . host/lib /tmp/unity/src sanitize

Leak checking is off because the tests create FreeRTOS queues per case and never free them; the
firmware itself is not checked for leaks by these tests.
