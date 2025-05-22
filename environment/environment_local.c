#include "environment_local.h"

FILE *fp;
uint32_t time_startup = 0;

const int BYTES_PER_PIXEL = 3; /// red, green, & blue
const int FILE_HEADER_SIZE = 14;
const int INFO_HEADER_SIZE = 40;
const int width = 1;
const int spacing = 0;

FILE *imageFile = NULL;

void put_pixel(uint16_t index, uint16_t len, const RgbwColor *c)
{
    int widthInBytes = width * BYTES_PER_PIXEL;
    int paddingSize = (4 - (widthInBytes) % 4) % 4;
    int stride = (widthInBytes) + paddingSize;

    int row = (index / width);
    int column = (index % width);
    int byteStartIndex = FILE_HEADER_SIZE + INFO_HEADER_SIZE + (row * stride + (row * spacing * stride)) + (column * BYTES_PER_PIXEL);

    if (imageFile == NULL)
    {
        imageFile = fopen("local_dev_output.bmp", "r+b");
    }

    fseek(imageFile, byteStartIndex, SEEK_SET);

    fwrite(&c->b, 1, 1, imageFile);
    fwrite(&c->g, 1, 1, imageFile);
    fwrite(&c->r, 1, 1, imageFile);
}

void put_pin(int gpio, bool value)
{
    // Do what here? Write to another file?
}

bool get_pin(int gpio)
{
    // TODO: Implement

    return false;
}

void init_pin_button(int gpio)
{
    // Do we care?
}

void launch_thread(void (*entry)(void))
{
    // Does this work well enough?
    // thrd_t thr[1];
    // thrd_create(&thr[0], entry, NULL);
}

void program_init(int offset, int gpio)
{
    // We do not care
}

int get_pio_offset()
{
    return 0;
}

void sleep_ms(uint32_t ms)
{
    // Do we even care to? Maybe?
    sleep_us(ms * 1000);
}

void sleep_us(uint64_t us)
{
    // Do we even care to? Maybe?
    // sleep(us * 1000);
}

uint32_t get_running_ms()
{
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return (time.tv_sec * 1000) + (time.tv_nsec / 1000000);
}

uint64_t get_running_us()
{
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return (time.tv_sec * 1000 * 1000) + (time.tv_nsec / 1000);
}

void picolight_blink(bool on)
{
    // ??
}

unsigned char *createBitmapFileHeader(int fileSize)
{
    static unsigned char fileHeader[] = {
        0, 0,       /// signature
        0, 0, 0, 0, /// image file size in bytes
        0, 0, 0, 0, /// reserved
        0, 0, 0, 0, /// start of pixel array
    };

    fileHeader[0] = (unsigned char)('B');
    fileHeader[1] = (unsigned char)('M');
    fileHeader[2] = (unsigned char)(fileSize);
    fileHeader[3] = (unsigned char)(fileSize >> 8);
    fileHeader[4] = (unsigned char)(fileSize >> 16);
    fileHeader[5] = (unsigned char)(fileSize >> 24);
    fileHeader[10] = (unsigned char)(FILE_HEADER_SIZE + INFO_HEADER_SIZE);

    return fileHeader;
}

unsigned char *createBitmapInfoHeader(int height, int width)
{
    static unsigned char infoHeader[] = {
        0, 0, 0, 0, /// header size
        0, 0, 0, 0, /// image width
        0, 0, 0, 0, /// image height
        0, 0,       /// number of color planes
        0, 0,       /// bits per pixel
        0, 0, 0, 0, /// compression
        0, 0, 0, 0, /// image size
        0, 0, 0, 0, /// horizontal resolution
        0, 0, 0, 0, /// vertical resolution
        0, 0, 0, 0, /// colors in color table
        0, 0, 0, 0, /// important color count
    };

    infoHeader[0] = (unsigned char)(INFO_HEADER_SIZE);
    infoHeader[4] = (unsigned char)(width);
    infoHeader[5] = (unsigned char)(width >> 8);
    infoHeader[6] = (unsigned char)(width >> 16);
    infoHeader[7] = (unsigned char)(width >> 24);
    infoHeader[8] = (unsigned char)(height);
    infoHeader[9] = (unsigned char)(height >> 8);
    infoHeader[10] = (unsigned char)(height >> 16);
    infoHeader[11] = (unsigned char)(height >> 24);
    infoHeader[12] = (unsigned char)(1);
    infoHeader[14] = (unsigned char)(BYTES_PER_PIXEL * 8);

    return infoHeader;
}

void picolight_boot(int led_count)
{
    printf("Booted up\n");

    int height = ceil((led_count / (double)width) + (led_count * spacing));

    unsigned char image[height][width][BYTES_PER_PIXEL];

    for (int h = 0; h < height; h++)
    {
        for (int w = 0; w < width; w++)
        {
            image[h][w][2] = (unsigned char)(0); /// r
            image[h][w][1] = (unsigned char)(0); /// g
            image[h][w][0] = (unsigned char)(0); /// b
        }
    }

    int widthInBytes = width * BYTES_PER_PIXEL;

    unsigned char padding[3] = {0, 0, 0};
    int paddingSize = (4 - (widthInBytes) % 4) % 4;
    int stride = (widthInBytes) + paddingSize;

    FILE *imageFile = fopen("local_dev_output.bmp", "wb");
    printf("Opened file\n");

    int fileSize = FILE_HEADER_SIZE + INFO_HEADER_SIZE + (stride * height);
    unsigned char *fileHeader = createBitmapFileHeader(fileSize);
    fwrite(fileHeader, 1, FILE_HEADER_SIZE, imageFile);
    printf("Wrote file header\n");

    unsigned char *infoHeader = createBitmapInfoHeader(height, width);
    fwrite(infoHeader, 1, INFO_HEADER_SIZE, imageFile);
    printf("Wrote info header\n");

    for (int h = 0; h < height; h++)
    {
        for (int w = 0; w < width; w++)
        {
            for (int b = 0; b < BYTES_PER_PIXEL; b++)
            {
                fwrite(&image[h][w][b], 1, 1, imageFile);
            }
        }

        fwrite(padding, 1, paddingSize, imageFile);
    }

    fclose(imageFile);
}

void picolight_post_boot()
{
}

int getPermanentInt(int i)
{
    return 0;
}

void setPermanentInt(int i, int v)
{
    // TODO: Do something?
}

void *keyboard_action_listener()
{
    while (1)
    {
        // TODO: Read action from file, which was activated from JavaScript somehow?

        sleep(1);
    }
}

void register_action_listeners()
{
    pthread_t thread;
    pthread_create(&thread, NULL, keyboard_action_listener, NULL);
}