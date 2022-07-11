
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include "time.h"

#include "global_config.h"
#include "libmaix_debug.h"
#include "libmaix_err.h"
#include "mud.h"

mud_info * mud_obj = NULL;
char* mud_path = "/root/mud/gender.mud";
libmaix_err_t  err = LIBMAIX_ERR_NONE;
libmaix_nn_t * nn = NULL;

void test_init()
{
  mud_obj =  libmaix_mud_init(mud_path);
}
void test_working()
{

    libmaix_nn_module_init();
    libmaix_nn_model_path_t model_path;
    libmaix_nn_opt_param_t opt_param;
    nn = libmaix_mud_build_model(mud_obj, &model_path, &opt_param);
    if(!nn)
    {
        LIBMAIX_DEBUG_PRINTF("libmaix mud src : build nn object is faild\n");
        return NULL;
    }
}
void test_deinit()
{
  libmaix_mud_deinit_mud(mud_obj);
}

int main(int argc, char *argv[])
{
  // printf("start init\n");
  // test_init();
  // printf("finish init\n");

  // printf("start working  \n");
  // test_working();
  // printf("finish working\n");

  // printf("start deinit \n");
  // test_deinit();
  // printf("finish deinit \n");
  libmaix_nn_module_init();
  nn = libmaix_mud_load_model(mud_path);
}