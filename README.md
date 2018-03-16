# Deepsleep button
a button with an LED.

1. connect to wifi (or fallback to softap)
2. enable arduinoOTA
3. do your thing (send http request)
4. deepsleep

# build

## USB-TTL
```
platformio run --target upload  --environment esp12e --upload-port /dev/ttyUSB0
```
## OTA
1. connect to wifi
2. check ip in your router
3. reset and wait 5 seconds
4. :
```
platformio run --target upload  --environment esp12e --upload-port <button-ip>
```

# wiring

RST -> Button -> GND
GPIO5 -> LED -> GND
