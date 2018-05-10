#ifdef _WIN32
    #define ZPL_IMPLEMENTATION
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-parameter"
    #pragma GCC diagnostic ignored "-Wuninitialized"
    #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    #pragma GCC diagnostic ignored "-Wunused-function"
    #pragma GCC diagnostic ignored "-Wunused-variable"
    #pragma GCC diagnostic ignored "-Wformat="
    #pragma GCC diagnostic ignored "-Wformat-extra-args"
    #pragma GCC diagnostic ignored "-Wmisleading-indentation"
    #include "../header/zpl.h"
    #pragma GCC diagnostic pop
#endif