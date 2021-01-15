#include "stdio.h"
#include <stdint.h>
#include <stdbool.h>

#include "global_config.h"
#include "global_build_info_time.h"
#include "global_build_info_version.h"

#include "libmaix_disp.h"
#include "libmaix_image.h"
#include "image.h"
#include <sys/time.h>
#include <unistd.h>


void draw_test(struct libmaix_disp* disp, void* buff, int w, int h)
{
    int count = 5;

    printf("--image module init\n");
    libmaix_image_module_init();

    libmaix_image_t* img = libmaix_image_create(w, h, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, buff, false);
    if(!img)
    {
        printf("create yuv image fail\n");
        goto end;
    }

    libmaix_image_color_t color = {
        .rgb888.r = 255,
        .rgb888.g = 255,
        .rgb888.b = 255
    };
    libmaix_image_color_t bg = {
        .rgb888.r = 241,
        .rgb888.g = 102,
        .rgb888.b = 102
    };
    while(count--)
    {
        img->draw_string(img, "hello", 20, 146, 16, color, NULL);
        img->draw_string(img, "libmaix", 20 + 50, 146, 16, color, &bg);
        img->draw_rectangle(img, 16, 142, 120, 24, color, false, 4);
        img->draw_rectangle(img, 0, 240 - 24, 240, 24, color, true, 0);
        img->draw_rectangle(img, 200, 142, 120, 24, color, false, 4);
        disp->draw(disp, img->data, (disp->width - img->width) / 2,(disp->height - img->height) / 2, img->width, img->height, 0);
        disp->flush(disp);
        sleep(2);
    }
end:
    printf("--image module deinit\n");
    libmaix_image_module_deinit();
}

int main(int argc, char* argv[])
{
    struct libmaix_disp* disp = libmaix_disp_creat();
    if(disp == NULL) {
        printf("creat disp object fail\n");
        return -1;
    }

    int w = 240, h = 240;
    struct timeval start, end;
    int64_t interval_s;
    int count = 10;

    disp->swap_rb = 1;

    while(count --)
    {
        printf("display now\n");
        gettimeofday( &start, NULL );
        disp->draw(disp, image_logo.pixel_data, (disp->width - w) / 2,(disp->height - h) / 2, w, h, 0);
        gettimeofday( &end, NULL );
        interval_s  =(int64_t)(end.tv_sec - start.tv_sec)*1000000ll;
        printf("use time: %lld us\n", interval_s + end.tv_usec - start.tv_usec);

        printf("flush\n");
        gettimeofday( &start, NULL );
        disp->flush(disp);
        gettimeofday( &end, NULL );
        interval_s  =(int64_t)(end.tv_sec - start.tv_sec)*1000000ll;
        printf("use time: %lld us\n", interval_s + end.tv_usec - start.tv_usec);
        printf("flush end\n");
    }

    sleep(1);

    printf("draw test\n");
    draw_test(disp, image_logo.pixel_data, image_logo.width, image_logo.height);
    printf("display end\n");

    libmaix_disp_destroy(&disp);
    return 0;
}

