# Picolight
A project where I try to create as good a project as possible for handling LED lights on the Raspberry Pico.

The idea is to create one as fast and versatile as possible.

This is done by creating algorithms that require virtually no memory, and have very high refresh rates.

# Basic overview
There main concepts of the whole project are:
* Pattern Module
    > The main driver of a particular type of LED theme. This could be `rainbow` or `snake` or similar.
* Pattern Data
    > Instance of `Pattern Module` attributes, randomly calculated once at `Pattern Module` start.
* Pattern Intensity
    > Used when creating `Pattern Data`. Advices random attributes on behavior.
* Pattern Data Creator
    > Creates `Pattern Data`. Takes parameters `number of lights` (`len`), and `intensity`.
* Pattern Data Destroyer
    > Handles freeing of memory allocated by `Pattern Data Creator`.
* Frame Data
    > Frame attributes, created once at beginning of frame (time of first light in loop).
    
    > Caches values which are common between multiple pixels.
* Frame Creator
    > Creates `Frame Data`. Takes parameters `number of lights` (`len`) and `time` (`t`).
* Frame Destroyer
    > Handles the freeing of memory allocated by `Frame Creator`.
* Pattern Executor
    > Does final calculations that are then printed to a `Pattern Printer`
    
    > Pixels are streamed as much as possible, so to not keep them in memory. Refresh rate is only limit.
* Pattern Printer
    > Prints to the strips. Can be wrapped over each other for compositioning.

# How the rendering pipeline can work

## Simple
1. Enter `main()` (`picolight.c`)
2. Enter endless loop which calls `execute_for_led_pin` (in theory we could have multiple pins/strips, but not yet fully implemented)
3. Run `pattern_execute`, send along current time (`pattern.c`)
4. If a `next pattern` has been set, then initialize that pattern
    * Get the pattern module, run its `Data Creator`.
    * Save the data to global state (this hinders multiple strips, for now)
5. Get current pattern module
6. Create `Frame Data` based on current time
7. Run loop over each LED light index
8. Run `Pattern Executor`, sending along `light index`, `Pattern Data`, `Frame Data` and default `Pattern Printer`
9. Do whatever inside the executor, but as extremely few calculations as possible
10. Call the `Pattern Printer` with the output pixel `HsiaColor`
11. `Pattern Printer` calls the final target, be it to the Pico pin or to elsewhere
12. After all pixels are printed, destroy `Frame Data`
13. Repeat from `3.` over and over

## Complex
What can be done at `9.` is to create a custom `Pattern Printer` and send that along to one or multiple other `Pattern Module`s.
This way we can create composite patterns. A somewhat simplified example is the `Snakes` module, which just prints multiple snakes.

```C

typedef struct data_struct ...
typedef struct frame_struct ...

typedef struct SnakesPrinter
{
    Printer override;
    int pixelIndex;
    HsiaColor pixels[2];

} SnakesPrinter;

static inline void snakes_printer(uint16_t index, HsiaColor *c, Printer *printer)
{
    SnakesPrinter *ourPrinter = (void *)printer;
    ourPrinter->pixels[ourPrinter->pixelIndex] = *c;
    ourPrinter->pixelIndex++;
}

static inline void executor(uint16_t i, void *dataPtr, void *framePtr, Printer *printer)
{
    data_struct *data = (data_struct *)dataPtr;
    frame_struct *frame = framePtr;

    // Create a fake printer that buffers 2 pixels from 2 other modules.
    SnakesPrinter bufferingPrinter = {{snakes_printer}};

    data->snakeModule->executor(i, data->snake1data, frame->frame1, (void *)&bufferingPrinter);
    data->snakeModule->executor(i, data->snake2data, frame->frame2, (void *)&bufferingPrinter);

    HsiaColor c = math_average_hsia(&bufferingPrinter.pixels[0], &bufferingPrinter.pixels[1]);

    // Now send to our real printer.
    printer->print(i, &c, printer);
}
```

This works, because the `override` field of the `SnakesPrinter` will take the same memory address offset as just the simple `Printer` function reference.

This is called a `struct composition` or `struct inheritance`. So we can send a `Printer` function reference that actually holds more custom data for us, that we cast back into our custom `SnakesPrinter` inside the `snakes_printer` callback function.

# Future plans
* Create better dimming by dithering between two levels of lights
* Create better adjustments to accomodate differences between different lights
* Handle RGB and RGBW on the same strips
* Handle multiple lines of strips connected to diffrent GPIO ports
* Easier and fast configuration to setup according to your own lights (hardcoded at the moment)
* Permanent storage of configuration using flash memory