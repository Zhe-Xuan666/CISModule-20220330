// #include "b.h"
#include "get_image.h"

typedef struct VideoBuffer
{
    void *start;
    size_t length;
} VideoBuffer;

VideoBuffer video_buffer_ptr[BUFFER_COUNT];

int cam_re_init(struct v4l2_format *format, int *cam_fd)
{
    int i;
    int ret;
    int cam_fd_re_build;
    int sel = 0;

    // struct v4l2_format format;
    struct v4l2_buffer video_buffer[BUFFER_COUNT];

    // Init_log is moved to main.c
    // init_log();

    // Open Camera
    cam_fd_re_build = open(VIDEO_DEVICE, O_RDWR); // connect camera

    if (cam_fd_re_build < 0)
    {
        write_log("[ error ] : Camera Open");
        goto err;
    }

    ret = ioctl(cam_fd_re_build, VIDIOC_S_INPUT, &sel); // setting video input

    if (ret < 0)
    {
        write_log("[ error ] : VIDICS_S_INPUT");
        goto err;
    }
    // These data is passed by previous function
    // format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    // format.fmt.pix.pixelformat = SENSOR_COLORFORMAT; // V4L2_PIX_FMT_SGRBG10;//10bit raw
    // format.fmt.pix.width = IMAGE_WIDTH;              // resolution
    // format.fmt.pix.height = IMAGE_HEIGHT;

    // œT»{¬O§_€ä«ù
    ret = ioctl(cam_fd_re_build, VIDIOC_TRY_FMT, format);

    if (ret != 0)
    {
        write_log("[ error ] : VIDIOC_TRY_FMT");
        goto err;
    }

    // ³]žm¬ÛŸ÷®æŠ¡
    format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(cam_fd_re_build, VIDIOC_S_FMT, format);

    if (ret != 0)
    {
        write_log("[ error ] : VIDIOC_S_FMT");
        goto err;
    }

    struct v4l2_requestbuffers req;

    // buffer žÌŠsªº·Ó€ùŒÆ
    req.count = BUFFER_COUNT;

    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    // ¥ÓœÐœwœÄ°ÏÀxŠs
    ret = ioctl(cam_fd_re_build, VIDIOC_REQBUFS, &req);
    if (ret != 0)
    {
        write_log("[ error ] : VIDIOC_REQBUFS, ¥ÓœÐœwœÄ°Ï¥¢±Ñ");
        goto err;
    }

    if (req.count < BUFFER_COUNT)
    {
        write_log("[error] : VIDIOC_REQBUFS, ¥ÓœÐœwœÄ°ÏŒÆ€£š¬");
        goto err;
    }

    // ¶}±ÒŒÆŸÚ¬y
    int buffer_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(cam_fd_re_build, VIDIOC_STREAMON, &buffer_type);
    if (ret != 0)
    {
        write_log("[ error ] : VIDIOC_STEAMON");
        goto err;
    }

    struct v4l2_buffer buffer;
    memset(&buffer, 0, sizeof(buffer));
    buffer.type = req.type;
    buffer.memory = V4L2_MEMORY_MMAP;
    for (i = 0; i < req.count; i++)
    {
        buffer.index = i;
        // ¬d§äœwŠs°Ï°T®§
        ret = ioctl(cam_fd_re_build, VIDIOC_QUERYBUF, &buffer);
        if (ret != 0)
        {
            write_log("[ error ] : VIDIOC_QUERYBUF");
            goto err;
        }

        // map kernel cache to user process
        video_buffer_ptr[i].length = buffer.length;
        video_buffer_ptr[i].start = (char *)mmap(NULL, buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, cam_fd_re_build, buffer.m.offset);
        if (video_buffer_ptr[i].start == MAP_FAILED)
        {
            write_log("[ error ] : Re-map failed");
            goto err;
        }
    }

    sleep(1);
    *cam_fd = cam_fd_re_build;
    return 0;

err:
    write_log("[ error ] : Re-init");
    close(cam_fd_re_build);
    *cam_fd = -1;
    return -1;
}

int change_raw_fmt(int pixel_bit, int *cam_fd)
{
    struct v4l2_format format;
    int ret;

    memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(*cam_fd, VIDIOC_G_FMT, &format);
    if (ret != 0)
    {
        write_log("[ error ] : change_raw_fmt: Fail to get fmt");
        return ret;
    }

    switch (pixel_bit)
    {
    case 6:
    case 7:
    case 14:
    case 20:
        write_log("[ error ] : change_raw_fmt: Unsupport fmt");
        return -1;

    case 8:
        format.fmt.pix.pixelformat = V4L2_PIX_FMT_SRGGB8;
        break;
    case 10:
        format.fmt.pix.pixelformat = V4L2_PIX_FMT_SRGGB10;
        break;
    case 12:
        format.fmt.pix.pixelformat = V4L2_PIX_FMT_SRGGB12;
        break;
    case 16:
        format.fmt.pix.pixelformat = V4L2_PIX_FMT_SRGGB16;
        break;

    default:
        write_log("[ error ] : change_raw_fmt: Invalid fmt");
        return -1;
    }

    // Close and re_init cam_fd
    ret = cam_close(*cam_fd);

    ret = cam_re_init(&format, cam_fd);

    return ret;
}
/*--------------------Extern functions---------------------*/

int cam_close(int cam_fd)
{
    for (int i = 0; i < BUFFER_COUNT; i++)
    {
        munmap(video_buffer_ptr[i].start, video_buffer_ptr[i].length);
    }
    int ret = close(cam_fd); // disconnect camera
    return 0;
}

int cam_init(int IMAGE_WIDTH, int IMAGE_HEIGHT)
{
    int i;
    int ret;
    int cam_fd;
    int sel = 0;
    char log_msg[5000];

    struct v4l2_format format;
    struct v4l2_buffer video_buffer[BUFFER_COUNT];

    // Init_log is moved to main.c
    // init_log();

    // Open Camera
    cam_fd = open(VIDEO_DEVICE, O_RDWR); // connect camera

    if (cam_fd < 0)
    {
        write_log("[ error ] : Camera Open");
        return -1;
    }

    ret = ioctl(cam_fd, VIDIOC_S_INPUT, &sel); // setting video input

    if (ret < 0)
    {
        write_log("[ error ] : VIDICS_S_INPUT");
        return -1;
    }

    memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.pixelformat = SENSOR_COLORFORMAT; // V4L2_PIX_FMT_SGRBG10;//10bit raw
    format.fmt.pix.width = IMAGE_WIDTH;              // resolution
    format.fmt.pix.height = IMAGE_HEIGHT;

    // œT»{¬O§_€ä«ù
    ret = ioctl(cam_fd, VIDIOC_TRY_FMT, &format);

    if (ret != 0)
    {
        write_log("[ error ] : VIDIOC_TRY_FMT");
        return ret;
    }

    // ³]žm¬ÛŸ÷®æŠ¡
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(cam_fd, VIDIOC_S_FMT, &format);

    if (ret != 0)
    {
        write_log("[ error ] : VIDIOC_S_FMT");
        return ret;
    }

    struct v4l2_requestbuffers req;

    // buffer žÌŠsªº·Ó€ùŒÆ
    req.count = BUFFER_COUNT;

    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    // ¥ÓœÐœwœÄ°ÏÀxŠs
    ret = ioctl(cam_fd, VIDIOC_REQBUFS, &req);
    if (ret != 0)
    {
        write_log("[ error ] : VIDIOC_REQBUFS, ¥ÓœÐœwœÄ°Ï¥¢±Ñ");
        return ret;
    }

    if (req.count < BUFFER_COUNT)
    {
        write_log("[error] : VIDIOC_REQBUFS, ¥ÓœÐœwœÄ°ÏŒÆ€£š¬");
        return ret;
    }

    // ¶}±ÒŒÆŸÚ¬y
    int buffer_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(cam_fd, VIDIOC_STREAMON, &buffer_type);
    if (ret != 0)
    {
        write_log("[ error ] : VIDIOC_STEAMON");
        return ret;
    }

    struct v4l2_buffer buffer;
    memset(&buffer, 0, sizeof(buffer));
    buffer.type = req.type;
    buffer.memory = V4L2_MEMORY_MMAP;
    for (i = 0; i < req.count; i++)
    {
        buffer.index = i;
        // ¬d§äœwŠs°Ï°T®§
        ret = ioctl(cam_fd, VIDIOC_QUERYBUF, &buffer);
        if (ret != 0)
        {
            write_log("[ error ] : VIDIOC_QUERYBUF");
            return ret;
        }

        // map kernel cache to user process
        video_buffer_ptr[i].length = buffer.length;
        video_buffer_ptr[i].start = (char *)mmap(NULL, buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, cam_fd, buffer.m.offset);
        if (video_buffer_ptr[i].start == MAP_FAILED)
            return -1;
    }

    sleep(1);

    return cam_fd;
}

int buffer_size = 0;
int cam_get_image_ext_fmt(void *out_buffer, int image_size, int cam_fd, int fmt, int pixel_bit)
{
    int i, ret;
    static int current_bit_depth = BIT_DEPTH;
    struct v4l2_buffer buffer;
    uint8_t *u8_image_buffer;
    uint16_t *u16_image_buffer;
    uint32_t *u32_image_buffer;

    if (current_bit_depth != pixel_bit)
    // if (1 == 1) // Debug build
    {
        ret = change_raw_fmt(pixel_bit, &cam_fd);
        current_bit_depth = pixel_bit;
        if (ret < 0)
        {
            write_log("[ error ] : change_raw_fmt");
            return -1;
        }
    }

    memset(&buffer, 0, sizeof(buffer));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;

    // put image from buffer
    buffer.index = buffer_size;
    ret = ioctl(cam_fd, VIDIOC_QBUF, &buffer);
    if (ret != 0)
    {
        write_log("[ error ] : VIDIOC_QBUF");
        return ret;
    }
    buffer_size++;

    // get image from buffer
    ret = ioctl(cam_fd, VIDIOC_DQBUF, &buffer);
    if (ret != 0)
    {
        write_log("[ error ] : VIDIOC_DQBUF");
        return ret;
    }
    buffer_size--;

    if (buffer.index < 0 || buffer.index >= BUFFER_COUNT)
        return ret;

    if (fmt == UINT_8 || fmt == FLOAT_32)
    {
        memcpy(out_buffer, video_buffer_ptr[buffer.index].start, image_size);
    }
    else
    {
        u8_image_buffer = (uint8_t *)malloc(image_size * sizeof(uint8_t));
        memcpy(u8_image_buffer, video_buffer_ptr[buffer.index].start, image_size);
    }

    if (fmt == UINT_8 || fmt == FLOAT_32)
        return 0;

    switch (fmt)
    {
    case UINT_16:
        u16_image_buffer = (uint16_t *)malloc(image_size);

        for (i = 0; i < image_size / 2; i++)
        {
            u16_image_buffer[i] = u8_image_buffer[2 * i + 1] << 8 | u8_image_buffer[2 * i];
        }
        memcpy(out_buffer, u16_image_buffer, image_size);
        free(u16_image_buffer);
        break;
    case UINT_32:
        u32_image_buffer = (uint32_t *)malloc(image_size);
        for (i = 0; i < image_size; i++)
        {
            u32_image_buffer[i] = u8_image_buffer[i * 4] |
                                  u8_image_buffer[i * 4 + 1] << 8 |
                                  u8_image_buffer[i * 4 + 2] << 16 |
                                  u8_image_buffer[i * 4 + 3] << 24;
        }
        memcpy(out_buffer, u32_image_buffer, image_size);
        free(u32_image_buffer);
        break;

    default:

        free(u8_image_buffer);
        return -1;
        break;
    }
    free(u8_image_buffer);
    return 0;
}

void init_log()
{
    FILE *f;
    char *log_filename = "./cis/get_image.txt";

    f = fopen(log_filename, "w");
    if (f == NULL)
    {
        printf("Log File cannot open!\n");
    }
    else
    {
        fclose(f);
    }
}

void write_log(char *log_msg)
{

    FILE *f;
    char *log_filename = "./cis/get_image.txt";

    f = fopen(log_filename, "a+");
    if (f != NULL)
    {
        fprintf(f, "%s", log_msg);
        fclose(f);
    }
    else
        printf("Log File cannot open!\n");

    printf("%s\n", log_msg);
}

void cvt_ByteOrder(uint8_t *new_file, uint8_t *raw_file, int raw_buffer_size, int pixel_bit)
{
    switch (pixel_bit)
    { /*
    case 6:
        for (int i = 0; i < raw_buffer_size; i++)
        {
            new_file[i] = 0 | (raw_file[i] >> 2);
        }
        break;
    case 7:
        for (int i = 0; i < raw_buffer_size; i++)
        {
            new_file[i] = 0 | (raw_file[i] >> 1);
        }
        break;*/
    case 8:
    case 16:
        for (int i = 0; i < raw_buffer_size; i++)
        {
            new_file[i] = raw_file[i];
        }
        break;
    case 10:
        for (int i = 0; i < raw_buffer_size; i += 2)
        {
            new_file[i] = ((raw_file[i + 1] & 0x3f) << 2) | (raw_file[i] >> 6);
            new_file[i + 1] = 0 | (raw_file[i + 1] >> 6);
        }
        break;
    case 12:
        for (int i = 0; i < raw_buffer_size; i += 2)
        {
            new_file[i] = ((raw_file[i + 1] & 0x0f) << 4) | (raw_file[i] >> 4);
            new_file[i + 1] = 0 | (raw_file[i + 1] >> 4);
        }
        break;
        /*
    case 14:
        for (int i = 0; i < raw_buffer_size; i += 2)
        {
            new_file[i] = ((raw_file[i + 1] & 0x03) << 6) | (raw_file[i] >> 2);
            new_file[i + 1] = 0 | (raw_file[i + 1] >> 2);
        }
        break;
    case 20:
        for (int i = 0; i < raw_buffer_size; i += 4)
        {
            new_file[i] = ((raw_file[i + 2] & 0x0f) << 4) | (raw_file[i + 1] >> 4);
            new_file[i + 1] = ((raw_file[i + 3] & 0x0f) << 4) | (raw_file[i + 2] >> 4);
            new_file[i + 2] = 0 | (raw_file[i + 3] >> 4);
            new_file[i + 3] = 0;
        }
        break;*/
    default:
        write_log("Error input Parameter, set default(RAW8/RAW16):\r\n");
        for (int i = 0; i < raw_buffer_size; i++)
        {
            new_file[i] = raw_file[i];
        }
        break;
    }
}

void cvt_ByteOrder_float(float *new_file, uint8_t *raw_file, int image_size)
{
    // Combine 2 bytes into 1 float
    // Ex: 4095 (2 bytes) --> 4095.0 (4 bytes)
    int i;
    for (i=0; i < image_size / 2; i++)
    {
        new_file [ i ] = (raw_file[i*2 + 1] << 8 | raw_file[i*2] ) * 1.0;
        
    }
}
