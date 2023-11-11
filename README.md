# DoggoLights
[WIP]
RGB LED Lights for dogs. Configure your dogs' leash color using a WiFi connection, no app needed. Rechargable and USB C.


We have a happy small black dachshund running around with his friends behind our house. Last year I made a quick and dirty prototype where I reused an IC I found in an old LED light, it worked fine, but then I had the idea, that I could make some for the whole squad. About 10 doggies running around.

Imagine every doggie has his own color, so you know which is which in the dark evenings. And you have a light show!



So I set out to make a plan with this. An ESP32, some RGB LEDs and reused batteries from the "disposable" vapes.



This is still in development stage, but the PCB is finalized more or less. All I need to do is program all the states, animations and configuring the LEDs through a WiFi webserver.

I didn't want anyone to download some app I made and develop for iOS and Android, this is not my cup of tea, so I chose to use WiFi.



All you would need to do is press the power button for about 5 seconds and then the webserver starts, connect to the WiFi access point the Doggie Lights make, configure the colors, access point name, animations and timings, save and you're set.



The components are small, so hand solder only if you like hand soldering 0402 passives.



Hope to finish it while it's dark outside so the doggies have all winter evenings to make a light show!