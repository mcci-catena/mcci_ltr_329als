# MCCI Arduino library for Lite-On LTR-329ALS Ambient Light Sensors

This library allows Arduino sketches to use the Lite-On LTR-329ALS ambient light sensor to measure light.

It has the following design principles.

1. It's written in pure C++; there are no `#defined` constants that are not encapsulated in a namespace or an object.

2. All I2C operations are carefully checked for errors.

3. APIs are mostly non-blocking; generally, one starts an operation and then polls the library to see if it's done.

4. There are no infinite delays that are not guarded by timeouts (whether blocking or non-blocking); all operations will eventually complete, even if the sensor hardware hangs.

## Meta

### License

This repository is released under the [MIT](./LICENSE.md) license. Commercial licenses are also available from MCCI Corporation.

### Support Open Source Hardware and Software

MCCI invests time and resources providing this open source code, please support MCCI and open-source hardware by purchasing products from MCCI, Adafruit and other open-source hardware/software vendors!

For information about MCCI's products, please visit [store.mcci.com](https://store.mcci.com/).

### Trademarks

MCCI and MCCI Catena are registered trademarks of MCCI Corporation. All other marks are the property of their respective owners.
