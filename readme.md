# OLED_Humidity
Implements an SHT31 temperature and humidity sensor, and an onboard calculation for dew point.

## Hardware
As written, the sketch targets a Heltec ESP32 dev kit with OLED display [Amazon Link](https://www.amazon.com/MakerFocus-Development-0-96inch-Display-Compatible/dp/B076KJZ5QM/ref=sr_1_2?ie=UTF8&qid=1542564126&sr=8-2&keywords=heltec). The ESP32 provides a main application processor as well as WiFi and possibly in the future, BLE connectivity. This dev kit was picked specifically for the addition of an OLED display, which is used to display the three values live. Since BLE is currently unused, it's probably trivial to port this sketch to an ESP8266 with OLED, like [this one](https://www.amazon.com/MakerFocus-ESP8266-Development-Display-Support/dp/B076JDVRLP/ref=sr_1_3?ie=UTF8&qid=1542564286&sr=8-3&keywords=esp8266+oled). Drawing code would need to be changed for the new display dimensions.

The sensor itself is a Sensiron SHT31-D, on a [breakout board from Adafruit](https://www.adafruit.com/product/2857). This sensor promises 2% RH accuracy from 0-100% RH, and .2°C temperature accuracy over the full temperature range of the part, which is about as good as any such sensor promises.

## Software
The sketch has 3 interfaces for the temperature, humidity, and dew point data.

### Display
Temp, humidity, and dew point are shown on the OLED display locally for instantaneous checking.

### UART
Temp, humidity, and dew point are echoed (in fahrenheit) over UART in the form of a python Tuple. For instance, (76.7, 53.6, 58.4), in the form (temp, humdiity, dew point).

### Network
REST endpoints are provided to query the sensor over the network. 

## Host Software
I've implemented a Python tool to interface with the sensor. The original version listens for sensor values over the UART interface. An updated version queries the sensor on the local network, so that the computer doesn't need to remain tethered to the sensor.