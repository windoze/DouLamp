M5Stick/Atom/AtomLite based lamp with iOS App
=============================================

* [M5StickC/M5Atom/M5AtomLite](https://m5stack.com/).
* [TB6612](https://wiki.seeedstudio.com/Grove-I2C_Motor_Driver-TB6612FNG/) for PWM control, can drive up to 15W LED (15V*1A).
* Two-color(white and yellow) 12V LED strip. RGB LED strip(NeoPixels) is not supported yet.


Build M5StickC/M5Atom firmware
------------------------------
Run following commands under `m5` directory

* Build and upload firmware for M5Stick-C:
```
pio init -e m5stick-c
pio run -t upload
```

* Build and upload firmware for M5Atom:
```
pio init -e m5atom
pio run -t upload
```

Generate project for IDE
------------------------
Run following commands under `m5` directory

```
pio init --ide [vscode|clion|...] -e [m5stick-c|m5atom]
```

Please note that generate project out of the source tree will fail for M5Atom, PlatformIO may have bug.