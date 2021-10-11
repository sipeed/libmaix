
class libimage
{
private:
  /* data */
public:
  libimage(/* args */);
  ~libimage();
};

libimage::libimage(/* args */)
{
}

libimage::~libimage(){};

extern "C" {
  
  #include "stdio.h"

  void libimage_cpp()
  {
    auto t = libimage();

    puts("libimage_cpp");
  }

}
