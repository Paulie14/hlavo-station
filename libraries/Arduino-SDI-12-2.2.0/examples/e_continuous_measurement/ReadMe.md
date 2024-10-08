## Example E: Check all Addresses for Active Sensors and Start Continuous Measurements<!-- {#example_e_page} -->

This is a simple demonstration of the SDI-12 library for Arduino.

It discovers the address of all sensors active on a single bus and takes continuous measurements from them.

Every SDI-12 device is different in the time it takes to take a measurement, and the amount of data it returns.  This sketch will not serve every sensor type, but it will likely be helpful in getting you started.

Each sensor should have a unique address already - if not, multiple sensors may respond simultaneously to the same request and the output will not be readable by the Arduino.

To address a sensor, please see Example B: b_address_change.ino

[//]: # ( @section e_continuous_measurement_pio PlatformIO Configuration )

[//]: # ( @include{lineno} e_continuous_measurement/platformio.ini )

[//]: # ( @section e_continuous_measurement_code The Complete Example )

[//]: # ( @include{lineno} e_continuous_measurement/e_continuous_measurement.ino )
