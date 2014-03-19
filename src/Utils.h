#ifndef UTILS_H_SYOM2IL9
#define UTILS_H_SYOM2IL9

#define FOREACH(I,ITEMS) I=ITEMS[0]; for (int i = 0 ; i < sizeof(ITEMS)/sizeof(ITEMS[0]) ; i++,I=ITEMS[i])

#define LOG(fmt, ...) do {  \
   Logger::log(Logger::LEVEL_INFO, "line %d: ", __LINE__); \
   Logger::log(Logger::LEVEL_INFO, fmt, __VA_ARGS__); \
   Logger::log(Logger::LEVEL_INFO, "\n"); \
} while (0)

inline int mod(int x) { if (x < 0) return -x; else return x; }

#endif /* end of include guard: UTILS_H_SYOM2IL9 */

