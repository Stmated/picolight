#include "environment_pico.h"

#define DMA_CHANNEL (0) /* bit plane content DMA channel */
#define DMA_CHANNEL_MASK (1u << DMA_CHANNEL)

#define RESET_TIME_US (200) // Spec says 50 us, but we wait a bit extra. Needs some downtime for the LEDs to latch.

static alarm_id_t reset_delay_alarm_id; // handle for the delay alert
static dma_channel_config channel_config;

void WS2812_Transfer();
void allocateBuffer();

static int64_t reset_delay_complete(alarm_id_t id, void *user_data)
{
    reset_delay_alarm_id = 0; /* reset alarm id */

    if (state.nextLedCount != -1)
    {
        // Increase LED count and allocate a new buffer.
        // If different threads get to `allocateBuffer` we will just hope things work okay without contention.
        state.ledCount = state.nextLedCount;
        state.nextLedCount = -1;

        dma_channel_set_trans_count(DMA_CHANNEL, state.ledCount * 3, false);
        // channel_config_set_transfer_data_size(&channel_config, )

        // Set flag to invalidate the buffer, so it can be done outside of this IRQ (otherwise there is risk of deadlock from malloc_mutex)
        state.buffer_invalidated = true;
        return 0;
    }

    // TODO: Do we even need the semaphore, since we have control over the calling?
    // sem_release(&reset_delay_complete_sem); /* release semaphore */

    // Draw as fast as we possibly can!
    WS2812_Transfer();

    return 0; // OK -- no repeat
}

void dma_complete_handler(void)
{
    // Check so we only act for our DMA channel
    if (dma_hw->ints0 & DMA_CHANNEL_MASK)
    {
        dma_hw->ints0 = DMA_CHANNEL_MASK; // clear IRQ
        if (reset_delay_alarm_id != 0)
        {
            // Cancel any already running alarm
            cancel_alarm(reset_delay_alarm_id);
        }

        // Wait a certain time before releasing the sempahore, so the LEDs have time to latch their data.
        reset_delay_alarm_id = add_alarm_in_us(RESET_TIME_US, reset_delay_complete, NULL, false);
    }
}

static void dma_init(PIO pio, unsigned int sm)
{
    dma_claim_mask(DMA_CHANNEL_MASK);
    channel_config = dma_channel_get_default_config(DMA_CHANNEL);
    channel_config_set_dreq(&channel_config, pio_get_dreq(pio, sm, true));
    channel_config_set_transfer_data_size(&channel_config, DMA_SIZE_8);
    channel_config_set_read_increment(&channel_config, true);
    dma_channel_configure(DMA_CHANNEL, &channel_config, &pio->txf[sm], NULL, state.ledCount * 3, false);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_complete_handler); // Configure raise interrupt after all data sent
    dma_channel_set_irq0_enabled(DMA_CHANNEL, true);            // enable interrupt for DMA
    irq_set_enabled(DMA_IRQ_0, true);                           // enable interrupt
}

void allocateBuffer()
{
    size_t buffer_size = (state.ledCount * 3);
    size_t padding = (4 - (buffer_size % 4));
    size_t padded_buffer_size = buffer_size + padding;

    state.buffer = calloc(padded_buffer_size, sizeof(uint8_t));
}

void WS2812_Transfer()
{
    uint32_t address = (uint32_t)state.buffer;

    // sem_acquire_blocking(&reset_delay_complete_sem); // Get semaphore, block until available
    dma_channel_set_read_addr(DMA_CHANNEL, (void *)address, true); // Reset the read address and trigger immediately
}

uint8_t premul_lut[256][256];
uint8_t zero = 0;

// timing parameters in PIO clock cycles:
#define T0_HIGH_CYCLES 2   // e.g. ~0.35 µs at your PIO clock rate
#define T1_HIGH_CYCLES 5   // e.g. ~0.7 µs
#define BIT_TOTAL_CYCLES 8 // total cycles per WS2812 bit (1.25 µs)

static bool start_transfer_loop = true;

inline void put_pixel(const uint16_t index, const uint16_t len, const uint64_t t_us, const uint32_t dt_us, const RgbwaColor *restrict c)
{
    if (state.buffer == NULL)
    {
        allocateBuffer();
    }
    else if (state.buffer_invalidated && index == 0)
    {
        state.buffer_invalidated = false;
        start_transfer_loop = true;

        free(state.buffer);
        state.buffer = NULL;
        
        allocateBuffer();
    }

    if (c->a < ALPHA_NEGLIGIBLE_MIN)
    {
        size_t address = (index * 3);
        state.buffer[address] = zero;
        state.buffer[address + 1] = zero;
        state.buffer[address + 2] = zero;

        // state.buffer[index] = 0;
    }
    else
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        if (c->a < ALPHA_NEGLIGIBLE_MAX)
        {
            uint8_t a = c->a;

            r = premul_lut[c->r][a];
            g = premul_lut[c->g][a];
            b = premul_lut[c->b][a];

            // TODO: Should add time-based dithering here
            //          - But seems the only stable way is to add a pixel buffer (which goes against the idea of the project...)
        }
        else
        {
            r = c->r;
            g = c->g;
            b = c->b;
        }

        if (!isRgbw(index) && c->w != 0)
        {
            // Distribute white component evenly to R, G, B
            // Scale it down slightly to preserve color balance (e.g., 2/3 strength)
            // Fast approximation: (w * 170) >> 8  ≈ w * 2/3
            uint8_t w_contrib = (c->w * 170) >> 8;

            // Saturated add
            r = MIN(255, r + w_contrib);
            g = MIN(255, g + w_contrib);
            b = MIN(255, b + w_contrib);
        }

        size_t address = (index * 3);
        state.buffer[address] = g;
        state.buffer[address + 1] = r;
        state.buffer[address + 2] = b;

        // uint32_t packed = (g << 24) | (r << 16) | (b << 8);
        // state.buffer[index] = packed;
    }

    if (start_transfer_loop && index == len - 1)
    {
        // We will only call this if we do not have a rendering alarm.
        start_transfer_loop = false;
        WS2812_Transfer();
    }

    /*
    if (isRgbw(index))
    {
        pio_sm_put_blocking(pio0, 0, ((uint32_t)c->g) << 24u);
        pio_sm_put_blocking(pio0, 0, ((uint32_t)c->r) << 24u);
        pio_sm_put_blocking(pio0, 0, ((uint32_t)c->b) << 24u);
        pio_sm_put_blocking(pio0, 0, ((uint32_t)c->w) << 24u);
    }
    else
    {
        pio_sm_put_blocking(pio0, 0, ((uint32_t)c->g) << 24u);
        pio_sm_put_blocking(pio0, 0, ((uint32_t)c->r) << 24u);
        pio_sm_put_blocking(pio0, 0, ((uint32_t)c->b) << 24u);
    }
    */
}

void put_pin(int gpio, bool value)
{
    gpio_put(gpio, value);
}

bool get_pin(int gpio)
{
    return gpio_get(gpio);
}

void program_init(int offset, int gpio)
{
    ws2812_program_init(pio0, 0, offset, gpio, 800000, false);
}

void init_pin_button(int gpio)
{
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_set_pulls(gpio, true, false);
}

int get_pio_offset()
{
    return pio_add_program(pio0, &ws2812_program);
}

void launch_thread(void (*entry)(void))
{
    multicore_launch_core1(entry);
}

void picolight_blink(bool on)
{
    gpio_put(PICO_DEFAULT_LED_PIN, on);
}

uint32_t get_running_ms()
{
    return to_ms_since_boot(get_absolute_time());
}

uint64_t get_running_us()
{
    return to_us_since_boot(get_absolute_time());
}

void picolight_boot(int led_count)
{
    // TODO: Check how to store this on the flash and read that memory on demand? Is that slow or fast?
    for (int val = 0; val < 256; val++)
    {
        for (int alpha = 0; alpha < 256; alpha++)
        {
            premul_lut[val][alpha] = (val * alpha) >> 8;
        }
    }

    // gpio_init(25);
    // gpio_set_dir(25, GPIO_OUT);

    // adc_init();
    // adc_gpio_init(26);
    // adc_select_input(0);
}

void picolight_post_boot(uint32_t offset)
{
    stdio_init_all();

    // PIO pio = pio0; /* the PIO used */
    // WS2812_sm = 0;  /* state machine used */

    // uint offset = pio_add_program(pio, &ws2812_program);
    // ws2812_program_init(pio, WS2812_sm, offset, NEOC_PIN_START, 800000, false); /* initialize it for 800 kHz */

    // sem_init(&reset_delay_complete_sem, 1, 1); /* semaphore initially posted so we don't block first time */
    dma_init(pio0, 0);
}

int getPermanentInt(int i)
{
    // char *p = (char *)XIP_BASE;
    // char *first_byte = *p;

    // XIP_BASE

    return 0;
}

void setPermanentInt(int i, int v)
{
    uint32_t interrupts = save_and_disable_interrupts();
    // flash_range_erase (...);

    // flash_range_program(FLASH_TARGET_OFFSET, );

    restore_interrupts(interrupts);
}

void register_action_listeners()
{
    // Add GPIO triggers for all action pins
}