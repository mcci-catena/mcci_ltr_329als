# MCCI Arduino library for Lite-On LTR-329ALS Ambient Light Sensors

This library allows Arduino sketches to use the Lite-On LTR-329ALS ambient light sensor to measure light.

It has the following design principles.

1. It's written in pure C++; there are no `#defined` constants that are not encapsulated in a namespace or an object.

2. All I2C operations are carefully checked for errors.

3. APIs are mostly non-blocking; generally, one starts an operation and then polls the library to see if it's done.

4. There are no infinite delays that are not guarded by timeouts (whether blocking or non-blocking); all operations will eventually complete, even if the sensor hardware hangs.

## Quick Intro

This library uses the LTR-329ALS sensor to take light measurements in units of lux. Using it is easy:

1. Create an object of type `Ltr_329als`.
2. Call the `Ltr_329als::begin()` method to initialize it.
3. Call the `Ltr_329als::startSingleMeasurement()` method to launch an asynchronous measurement.
4. Poll the sensor using `Ltr_329als::queryReady()` until the measurement is ready or a hard error is indicated.
5. Convert the measurement to lux using `Ltr_329als::getLux()`.

See [sample code](#sample-code) below for details of the required steps.

## Sample Code

1. The header file

   ```c++
   #include <mcci_ltr_329als.h>
   ```

2. The namespace:

   ```c++
   using namespace Mcci_Ltr_329als;
   ```

3. The global object:

   ```c++
   // declare the global light-sensor instance object,
   // and attach it to the default TwoWire bus.
   Ltr_329als gLtr {Wire};
   ```

4. Code in `setup()`:

   ```c++
   // initialize the LTR
   if (! gLtr.begin())
        /* the light sensor isn't working */;
   ```

5. Code in `loop()`. Note that this is very simplistic, but it will always continue through to the bottom. Because `Ltr_329als::queryReady()` doesn't block, it is straightforward to refactor this so that the light sensor doesn't delay other processing.

   ```c++
   bool fHardError = false;
   if (gLtr.startSingleMeasurement()) {
        while (! gLtr.queryReady(fHardError)) {
                if (fHardError) {
                        break;
                }
        }
   } else {
        fHardError = true;
   }

   float lux;

   if (fHardError) {
        // print gLtr.getErrorName and do what's
        // best for your app
   } else {
        lux = gLtr.getLux();
        // do something with lux value
   }
   ```

## Meta

### License

This repository is released under the [MIT](./LICENSE.md) license. Commercial licenses are also available from MCCI Corporation.

### Support Open Source Hardware and Software

MCCI invests time and resources providing this open source code, please support MCCI and open-source hardware by purchasing products from MCCI, Adafruit and other open-source hardware/software vendors!

For information about MCCI's products, please visit [store.mcci.com](https://store.mcci.com/).

### Trademarks

MCCI and MCCI Catena are registered trademarks of MCCI Corporation. All other marks are the property of their respective owners.
