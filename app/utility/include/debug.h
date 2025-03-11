#ifdef DEBUG
    #define DEBUG_PRINT(fmt, ...) \
        do { printf(fmt, ##__VA_ARGS__); } while (0)
#else
    #define DEBUG_PRINT(fmt, ...) \
        do { } while (0)
#endif