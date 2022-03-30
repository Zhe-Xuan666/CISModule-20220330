#include "get_image.h"
#include "myFuncts.h"
//time
#include <time.h>
#include <sys/time.h>

/*read_ini*/
char buffIni[40];
char iniFile[20];
int IMAGE_WIDTH;
int IMAGE_HEIGHT;
int IMAGE_SIZE;
/*read_ini*/

/* Output buffer format defines*/
/* #define UINT_8 1   */
/* #define UINT_16 2  */
/* #define UINT_32 3  */
/* #define FLOAT_32 4 */
#define BufferType FLOAT_32

int main()
{
    int cam_fd;
    FILE *f;
    uint8_t *raw_buffer;
    uint8_t *cvt_buffer;
    float *float_buffer;
    uint8_t file_header[16];
    char file_name[64];
    char tmp[64] = {"---\n"};
    int frame_num = 0;
    int bit;
    /*Run Time*/
#define CLOCKS_PER_MSEC 1000
    clock_t S1_Begin, S1_End, S2_Begin, S2_End, S3_Begin, S3_End, S4_Begin, S4_End;
    double S1_duration, S2_duration, S3_duration, S4_duration;

    /*Read ini*/
    printf("This code is only for float\n");
    printf("Start calling <config.ini> to read raw data:\r\n");
    strcpy(iniFile, "config.ini");

    if (!initFuncts(iniFile))
    {
        puts("initFuncts error");
        return EXIT_FAILURE;
    }

    puts("initFuncts OK");
    /*Read ini*/

    /*Set buffer size*/
    // IMAGE_SIZE should be input as IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(type)
    raw_buffer = (uint8_t *)malloc(IMAGE_SIZE /* * sizeof(uint8_t)*/);
    cvt_buffer = (uint8_t *)malloc(IMAGE_SIZE /* * sizeof(uint8_t)*/);
    float_buffer = (float *)malloc(IMAGE_SIZE );

    /* Init log */
    init_log();

    /*init cam*/
    cam_fd = cam_init(IMAGE_WIDTH, IMAGE_HEIGHT);
    if (cam_fd == -1)
    {
        printf("fail to init\n");
        free(raw_buffer);
        free(cvt_buffer);
        free(float_buffer);
        return -1;
    }

    while (1)
    {
        /*test change bit depth*/
        /*printf("frame_num = %d\n", frame_num);
        if(frame_num %2 ==0)
            bit = 10;
        else
            bit = 12;
        printf("Bit = %d\n",bit);*/
        /*test change bit depth*/

        S3_Begin = clock();

        /*test change bit depth*/
        /*Get Data*/
        /*if (cam_get_image_ext_fmt(raw_buffer, IMAGE_SIZE, cam_fd, BufferType, bit) != 0)
        {
            printf("error get image\n");
            goto end;
        }*/
        /*Get Data*/
        /*test change bit depth*/

        /*Get Data*/
        if (cam_get_image_ext_fmt(raw_buffer, IMAGE_SIZE, cam_fd, BufferType, BIT_DEPTH) != 0)
        {
            printf("error get image\n");
            goto end;
        }
        /*Get Data*/
        S3_End = clock();
        S3_duration = (double)(S3_End - S3_Begin) / CLOCKS_PER_MSEC;
        printf("\tRead Image Data run time = %f ms\n", S3_duration);

        // 2022-03-30: Convert Data first, then print out

        /*Convert Data Byte Order*/

        // cvt_ByteOrder(cvt_buffer, raw_buffer, IMAGE_SIZE, BIT_DEPTH);
        cvt_ByteOrder_float(float_buffer, raw_buffer, IMAGE_SIZE);

        /*Convert Data Byte Order*/

        /*Show Data*/

        // for (int i = 0; i < 16; i++)
        //     sprintf(&tmp[strlen(tmp)], "%02x ", raw_buffer[i]);
        // printf("%s\r\n", tmp);

        printf("Float buffer:\n");
        for (int i=0;i<16;i++)
            printf("%f ", float_buffer[i]);
        printf("\n");
        memset(tmp, 0, 16);

        /*Show Data*/

        /*Write raw File*/
        sprintf(file_name, "./Pic/RAW_raw8-test_%03u.raw", (unsigned)frame_num);
        f = fopen(file_name, "wb");
        if (f == NULL)
        {
            printf("Fail to open file\n");
            goto end;
        }
        memset(file_header, 0, 16);
        file_header[8] = IMAGE_WIDTH % 256;
        file_header[9] = (int)IMAGE_WIDTH / 256;
        file_header[12] = IMAGE_HEIGHT % 256;
        file_header[13] = (int)IMAGE_HEIGHT / 256;
        fwrite(file_header, 1, 16, f);
        fseek(f, 16, SEEK_SET);
        fwrite(raw_buffer, 1, IMAGE_SIZE, f);
        fclose(f);
        /*Write raw File*/

        /*Write cvt File*/
        // sprintf(file_name, "./Pic/CVT_RAW_raw8-test_%03u.raw", (unsigned)frame_num);
        // f = fopen(file_name, "wb");
        // if (f == NULL)
        // {
        //     printf("Fail to open file\n");
        //     goto end;
        // }
        // memset(file_header, 0, 16);
        // file_header[8] = IMAGE_WIDTH % 256;
        // file_header[9] = (int)IMAGE_WIDTH / 256;
        // file_header[12] = IMAGE_HEIGHT % 256;
        // file_header[13] = (int)IMAGE_HEIGHT / 256;
        // fwrite(file_header, 1, 16, f);
        // fseek(f, 16, SEEK_SET);
        // fwrite(cvt_buffer, 1, IMAGE_SIZE, f);
        // fclose(f);
        /*Write cvt File*/

        /*Write float File*/
        sprintf(file_name, "./Pic/FLOAT_RAW_raw8-test_%03u.raw", (unsigned)frame_num);
        f = fopen(file_name, "wb");
        if (f == NULL)
        {
            printf("Fail to open file\n");
            goto end;
        }
        memset(file_header, 0, 16);
        file_header[8] = IMAGE_WIDTH % 256;
        file_header[9] = (int)IMAGE_WIDTH / 256;
        file_header[12] = IMAGE_HEIGHT % 256;
        file_header[13] = (int)IMAGE_HEIGHT / 256;
        fwrite(file_header, 1, 16, f);
        fseek(f, 16, SEEK_SET);
        fwrite(float_buffer, 1, IMAGE_SIZE, f);
        fclose(f);
        /*Write float File*/

        // next Frame
        frame_num++;
        printf("Next frame, sleep for 0.5 sec\n");
        usleep(5 * 1000 * 100);

        //reset fmt and stream to clear buf
        change_raw_fmt(BIT_DEPTH, &cam_fd);
    }

end:
    cam_close(cam_fd);
    free(raw_buffer);
    free(cvt_buffer);
    free(float_buffer);
    return 0;
}
