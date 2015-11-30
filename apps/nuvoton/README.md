# uNabto Nuvoton NuMaker TRIO Demo

This project demonstrates running uNabto on the Nuvoton NuMaker TRIO board (https://www.nuvoton.com/hq/support/reference-design/NuMaker-TRIO-IoT.html?__locale=en).

Demo enables user to connect to device via AP or STA mode and toggle OLED text. The demo application is easily extended to fetch multiple sensor data as well.

## Running the demo

1. Download Nu-Link Keil driver from http://nuvoton.com to program the module.
2. Open `unabto/apps/nuvoton/KEIL/NuMaker_TRIO.uvproj` using KEIL MDK-ARM IDE.
3. Compile and Load to board.
4. Reset board to load new application.
5. Install browser plugin from [Nabto](http://nabto.com/#download-list), or use the Nabto iOS or Android application.
6. Go to `nabto://self/discover`. If you are on the same network as the module, you will see a `nuvoton.u.nabto.net` device.
7. Tap device, login and you will be presented with the light bulb demo.

## Current limitations

- DNS resolution is not supported on the RAK module yet.
- Enabling `NABTO_ENABLE_LOGGING` inside unabto_config.h can make the code exceed the 32kB limitation of KEIL lite.
- Demo does not use crypto, so connections are unencrypted.

## Notes about the project

- Project is using `RAK_MODULE_WORK_MODE==ASSIST_CMD_TYPE` to get/set socket info.
- WiFi socket configuration should be dual socket enabled, both sockets set to UDP server, socket A at port 5570 and socket B at port 25001.
- If connecting to the module in AP mode, make sure you have fetched the HTML-DD while still having an internet connection. You can do this by using the 'Prepare offline access' button on `nabto://self/discover`.
