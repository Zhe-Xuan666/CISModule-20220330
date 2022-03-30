#ifndef __GET_IMAGE_H__
#define __GET_IMAGE_H__

/* Std Libs */
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stdint.h"

/* Error Libs*/
#include <errno.h>

/* Linux Libs */

#include <fcntl.h>
#include <unistd.h> // For close

/* V4L2 Libs*/
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#define VIDEO_DEVICE "/dev/video0"
#define SENSOR_COLORFORMAT V4L2_PIX_FMT_SBGGR10 // 12
#define BIT_DEPTH 10  // 10 no data
#define BUFFER_COUNT 5

/* Output buffer format defines*/
#define UINT_8 1
#define UINT_16 2
#define UINT_32 3
#define FLOAT_32 4

#ifdef __cplusplus
extern "C"
{
#endif
    // uint8_t *video_buffer_ptr[BUFFER_COUNT];

    /* Return cam_fd or -1 for error */
    int cam_init(int IMAGE_WIDTH, int IMAGE_HEIGHT);

    /* Convert Data Byte Order */
    void cvt_ByteOrder(uint8_t *new_file, uint8_t *raw_file, int raw_buffer_size, int pixel_bit);
    void cvt_ByteOrder_float(float *new_file, uint8_t *raw_file, int image_size);

    /* Return 0 for success, -1 for error */
    /* Use fmt to decide output buffer format */
    int cam_get_image_ext_fmt(void *out_buffer, int out_buffer_size, int cam_fd, int fmt, int raw_bit);

    /* Close camera before leaving */
    int cam_close(int cam_fd);

    /* reset stream and format to clear buf */
    int change_raw_fmt(int pixel_bit, int *cam_fd);

    void init_log();
    void write_log(char *log_msg);

#ifdef __cplusplus
}
#endif

#endif
