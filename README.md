# Picolight
A project where I try to create as good a project as possible for handling LED lights on the Raspberry Pico.

The idea is to create one as fast and versatile as possible.

This is done by creating algorithms that require virtually no memory, and have very high refresh rates.

# Basic overview
There main concepts of the whole project are:
* Pattern
    > The main driver of a particular type of LED theme. This could be `rainbow` or `snake` or similar.
* Pattern Data
    > An instance of `Pattern` attributes, randomly calculated only once at its creation on `Pattern` start. Unique to each `Pattern`.
* Pattern Intensity
    > Used when creating a `Pattern Data` to advice the random attributes on how to behave. Higher intensity: more speed/color.
* Pattern Data Creator
    > The method that creates the instance of the `Pattern Data`. Takes parameters like `time` and `intensity`.
* Pattern Data Destroyer
    > Handles the freeing of memory allocated by the `Pattern Data Creator`.
* Pattern Executor
    > Used to do the actual calculations of the pixels. Usually streaming pixels without any storage in memory to a `Pattern Printer`, hence theoretically giving no upper limit to number of lights (except refresh rate).
* Pattern Printer
    > Used to print a pixel to the strips. These can be wrapped over each other to give functionality such as mixing two patterns.

# Future plans
* Create better dimming by dithering between two levels of lights
* Create better adjustments to accomodate differences between different lights
* Handle RGB and RGBW on the same strips
* Handle multiple lines of strips connected to diffrent GPIO ports
* Easier and fast configuration to setup according to your own lights (hardcoded at the moment)
* Permanent storage of configuration using flash memory