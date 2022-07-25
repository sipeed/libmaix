
#include "gs831_uvai.hpp"
#include "find_apriltag.hpp"

extern "C"
{
    gs831_uv _gs831_, *gs831 = &_gs831_;

    // static struct timeval old, now;

    static void cap_set()
    {
        return;
        // gettimeofday(&old, NULL);
    }

    static void cap_get(const char *tips)
    {
        return;
        // gettimeofday(&now, NULL);
        // if (now.tv_usec > old.tv_usec)
        //     printf("%20s - %5ld ms\r\n", tips, (now.tv_usec - old.tv_usec) / 1000);
    }

    unsigned char *g2d_allocMem(unsigned int size);
    int g2d_freeMem(void *vir_ptr);
    unsigned int g2d_getPhyAddrByVirAddr(void *vir_ptr);

    void gs831_signal(int signal)
    {
        gs831->signal = signal;
        switch (signal)
        {
        case SIGINT:
        case SIGABRT:
        case SIGSEGV:
        {
            gs831->exit = 0;
            break;
        }
        default:
            break;
        }
    }

    void gs831_vi_open()
    {
        gs831->vi = libmaix_cam_create(0, gs831->vi_w, gs831->vi_h, 0, 0);
        if (NULL == gs831->vi)
            return;
        gs831->vi->start_capture(gs831->vi);
    }

    void gs831_vi_stop()
    {
        if (NULL != gs831->vi)
            libmaix_cam_destroy(&gs831->vi);
    }

    void gs831_vi_load()
    {
        LIBMAIX_DEBUG_PRINTF("gs831_vi_load");
        libmaix_camera_module_init();
        libmaix_image_module_init();
        libmaix_nn_module_init();

        gs831->vi_w = gs831_vi_w, gs831->vi_h = gs831_vi_h;
        gs831->ui_w = gs831_ui_w, gs831->ui_h = gs831_ui_h;

        gs831_vi_open();

        gs831->vo = libmaix_vo_create(gs831->ui_w, gs831->ui_h, 0, 30, gs831->ui_w, gs831->ui_h);
        if (NULL == gs831->vo)
            return;

        gs831->vi_th_usec = 30 * 1000; // 30ms 33fps for vi & hw
    }

    void gs831_vi_exit()
    {
        gs831_vi_stop();

        if (NULL != gs831->vo)
            libmaix_vo_destroy(&gs831->vo), gs831->vo = NULL;

        // if (gs831->ui_rgba)
        //     libmaix_image_destroy(&gs831->ui_rgba);

        libmaix_nn_module_deinit();
        libmaix_image_module_deinit();
        libmaix_camera_module_deinit();
        LIBMAIX_DEBUG_PRINTF("gs831_vi_exit");
    }

    void gs831_vi_loop()
    {
        CALC_FPS("gs831_vi_loop");
        // LIBMAIX_INFO_PRINTF("gs831_vi_loop");

        cap_set();
        void *frame = gs831->vo->get_frame(gs831->vo, 0);
        if (frame != NULL)
        {
            uint32_t *phy = NULL, *vir = NULL;
            gs831->vo->frame_addr(gs831->vo, frame, &vir, &phy);
            if (gs831->vi && LIBMAIX_ERR_NONE == gs831->vi->capture(gs831->vi, (unsigned char *)vir[0]))
            {
                find_apriltag_app_loop(&find_apriltag_app, (unsigned char *)vir[0], gs831->vi_w, gs831->vi_h);
                gs831->vo->set_frame(gs831->vo, frame, 0);
            }
        }
        cap_get("gs831->vi");

        // cap_set();
        // pthread_mutex_lock(&gs831->vi_mutex);
        // if (LIBMAIX_ERR_NONE == gs831->ai->capture_image(gs831->ai, &gs831->ai_rgb))
        // {
        //     // cv::Mat cv_src(gs831->ai_rgb->height, gs831->ai_rgb->width, CV_8UC3, gs831->ai_rgb->data);
        //     // cv::rectangle(cv_src, cv::Point(24, 24), cv::Point(200, 200), cv::Scalar(255, 0, 0), 5);
        //     // void *tmp = gs831->vo->get_frame(gs831->vo, 9);
        //     // if (tmp != NULL)
        //     // {
        //     //     uint32_t *phy = NULL, *vir = NULL;
        //     //     gs831->vo->frame_addr(gs831->vo, tmp, &vir, &phy);
        //     //     cv::Mat bgra(gs831->ui_h, gs831->ui_w, CV_8UC4, (unsigned char *)vir[0]);
        //     //     cv::rectangle(bgra, cv::Point(20, 20), cv::Point(220, 220), cv::Scalar(0, 0, 255, 128), 20);
        //     //     // cv::Mat cv_dst;
        //     //     // cv::resize(cv_src, cv_dst, cv::Size(gs831->ui_w, gs831->ui_h));
        //     //     // cv::cvtColor(cv_dst, bgra, cv::COLOR_RGB2BGRA);
        //     //     gs831->vo->set_frame(gs831->vo, tmp, 9);
        //     // }
        // }
        // pthread_mutex_unlock(&gs831->vi_mutex);
        // cap_get("gs831->ai");
    }

    void gs831_write_string_to_file(std::string path, std::string txt)
    {
        // FILE *fp = NULL;
        // fp = fopen(path.c_str(), "w+");
        // if (fp) {
        //     fprintf(fp, "%s", txt.c_str());
        //     fclose(fp);
        // }
        // printf("fp %p\r\n", fp);
        std::ofstream outfile(path);
        outfile << txt;
        outfile.flush();
        outfile.close();
    }

    std::string gs831_read_file_to_string(const std::string &path)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            return "";
        }
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

    void gs831_save_json_conf(const std::string &path, json5pp::value &cfg)
    {
        auto tmp = cfg.stringify();
        LIBMAIX_INFO_PRINTF("save_json_conf %s\n", tmp.c_str());
        gs831_write_string_to_file(path, tmp);
        system("sync");
    }

    void gs831_load_json_conf(const std::string &path, json5pp::value &cfg, json5pp::value old)
    {
        std::string conf = gs831_read_file_to_string(path);
        try
        {
            cfg = json5pp::parse(conf);
            // {
            //     cfg["last_select"] = cfg["last_select"] + 1;
            //     cfg["language"] = "zh-kz";
            //     std::cout << cfg["last_select"] << std::endl;
            //     std::cout << cfg["language"] << std::endl;
            // }
        }
        catch (json5pp::syntax_error e)
        {
            LIBMAIX_INFO_PRINTF("load_json_conf %s : %s", conf.c_str(), e.what());
            system(string_format("rm -rf %s && sync", path.c_str()).c_str());
            cfg = old;
            gs831_save_json_conf(path, cfg);
        }
        LIBMAIX_INFO_PRINTF("load_json_conf %s\n", conf.c_str());
    }

    void maix_gs831_main(int argc, char *argv[])
    {
        void gs831_ctrl_load();
        void gs831_home_load();
        void gs831_ctrl_loop();
        void gs831_home_loop();
        void gs831_ctrl_exit();
        void gs831_home_exit();

        pthread_mutex_init(&gs831->vi_mutex, NULL);
        pthread_mutex_init(&gs831->ai_mutex, NULL);
        pthread_mutex_init(&gs831->ui_mutex, NULL);

        gs831->exit = 0;
        signal(SIGINT, gs831_signal);
        signal(SIGABRT, gs831_signal);
        signal(SIGSEGV, gs831_signal);
        find_apriltag_app_load(&find_apriltag_app);
        gs831_vi_load();
        gs831_ctrl_load();

        while (gs831->exit == 0)
        {
            // usleep(gs831->vi_th_usec); // 30ms 33fps for vi & hw
            gs831_vi_loop();
            gs831_ctrl_loop();
        }

        find_apriltag_app_exit(&find_apriltag_app);
        gs831_ctrl_exit();
        gs831_vi_exit();
        gs831->exit = 1;
    }
}
