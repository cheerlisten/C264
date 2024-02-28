#pragma once
#include <c264/config.h>
#include <stdlib.h>

enum LogLevel
{
    LL_Verbose,
    LL_Debug,
    LL_Info,
    LL_Warn,
    LL_Error,
    LL_Panic
};

void SetLogLevel(LogLevel ll);
void Log(LogLevel ll, const char* fmt, ...);

#if !defined(CONFIG_FORCE_LOG_POSITION)
    #define LOG(...) Log(__VA_ARGS__)
#else
    #define LOG LOGP
#endif
#define LOGF(ll, fmt, ...) Log(ll, fmt " at %s", __VA_ARGS__, __func__)
#define LOGP(ll, fmt, ...) Log(ll, fmt " at %s:%d", __VA_ARGS__, __FILE__, __LINE__)
#define LOGPF(ll, fmt, ...) Log(ll, fmt " at %s %s:%d", __VA_ARGS__, __func__, __FILE__, __LINE__)

#define __CHOOSE_20th(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, ...)  \
    _20
// clang-format off
#define __MAP(_1, _2, _3, more, ...) __CHOOSE_20th( \
__VA_ARGS__, more, more, more, more, more, more, more, more, more, \
       more, more, more, more, more, more, more,   _3,   _2,   _1)
// clang-format on
#define __ASSERT_detail(condi, msg, act, ...)                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        bool b = condi;                                                                                                \
        if (b)                                                                                                         \
            continue;                                                                                                  \
        LOGPF(LL_Error, "ASSERT(" #condi ") failed " msg, ##__VA_ARGS__);                                              \
        act;                                                                                                           \
    } while (0)

#define __ASSERT_1(condi) __ASSERT_detail(condi, "", exit(-1))
#define __ASSERT_2(condi, msg) __ASSERT_detail(condi, msg, exit(-1))
#define __ASSERT_3(condi, msg, act) __ASSERT_detail(condi, msg, act)
#define AASSERT(...) __MAP(__ASSERT_1, __ASSERT_2, __ASSERT_3, __ASSERT_detail, __VA_ARGS__)(__VA_ARGS__)

// clang-format off
#define __Choose30th(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10,\
                     _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,\
                     _21, _22, _23, _24, _25, _26, _27, _28, _29, _30,\
                    ...) _30
#define ID(_1, ...) _1
#define IDx(...) __VA_ARGS__
#define Callx(f, ...) ID(f(__VA_ARGS__))
#define CallxPPS(f, _1) ID(f _1)
#define FOREACH0(it_operator, pps, ...)
#define FOREACH1(it_operator, pps, _1, ...)  IDx(it_operator(pps, _1) FOREACH0 (it_operator, pps, __VA_ARGS__))
#define FOREACH2(it_operator, pps, _1, ...)  IDx(it_operator(pps, _1) FOREACH1 (it_operator, pps, __VA_ARGS__))
#define FOREACH3(it_operator, pps, _1, ...)  IDx(it_operator(pps, _1) FOREACH2 (it_operator, pps, __VA_ARGS__))
#define FOREACH4(it_operator, pps, _1, ...)  IDx(it_operator(pps, _1) FOREACH3 (it_operator, pps, __VA_ARGS__))
#define FOREACH5(it_operator, pps, _1, ...)  IDx(it_operator(pps, _1) FOREACH4 (it_operator, pps, __VA_ARGS__))
#define FOREACH6(it_operator, pps, _1, ...)  IDx(it_operator(pps, _1) FOREACH5 (it_operator, pps, __VA_ARGS__))
#define FOREACH7(it_operator, pps, _1, ...)  IDx(it_operator(pps, _1) FOREACH6 (it_operator, pps, __VA_ARGS__))
#define FOREACH8(it_operator, pps, _1, ...)  IDx(it_operator(pps, _1) FOREACH7 (it_operator, pps, __VA_ARGS__))
#define FOREACH9(it_operator, pps, _1, ...)  IDx(it_operator(pps, _1) FOREACH8 (it_operator, pps, __VA_ARGS__))
#define FOREACH10(it_operator, pps, _1, ...) IDx(it_operator(pps, _1) FOREACH9 (it_operator, pps, __VA_ARGS__))
#define FOREACH11(it_operator, pps, _1, ...) IDx(it_operator(pps, _1) FOREACH10(it_operator, pps, __VA_ARGS__))
#define FOREACH12(it_operator, pps, _1, ...) IDx(it_operator(pps, _1) FOREACH11(it_operator, pps, __VA_ARGS__))
#define FOREACH13(it_operator, pps, _1, ...) IDx(it_operator(pps, _1) FOREACH12(it_operator, pps, __VA_ARGS__))
#define FOREACH14(it_operator, pps, _1, ...) IDx(it_operator(pps, _1) FOREACH13(it_operator, pps, __VA_ARGS__))
#define FOREACH15(it_operator, pps, _1, ...) IDx(it_operator(pps, _1) FOREACH14(it_operator, pps, __VA_ARGS__))
#define FOREACH16(it_operator, pps, _1, ...) IDx(it_operator(pps, _1) FOREACH15(it_operator, pps, __VA_ARGS__))
#define FOREACH17(it_operator, pps, _1, ...) IDx(it_operator(pps, _1) FOREACH16(it_operator, pps, __VA_ARGS__))
#define FOREACH18(it_operator, pps, _1, ...) IDx(it_operator(pps, _1) FOREACH17(it_operator, pps, __VA_ARGS__))
#define FOREACH19(it_operator, pps, _1, ...) IDx(it_operator(pps, _1) FOREACH18(it_operator, pps, __VA_ARGS__))
#define FOREACH20(it_operator, pps, _1, ...) IDx(it_operator(pps, _1) FOREACH19(it_operator, pps, __VA_ARGS__))
#define FOREACH21(it_operator, pps, _1, ...) IDx(it_operator(pps, _1) FOREACH20(it_operator, pps, __VA_ARGS__))
#define FOREACH22(it_operator, pps, _1, ...) IDx(it_operator(pps, _1) FOREACH21(it_operator, pps, __VA_ARGS__))
#define FOREACH23(it_operator, pps, _1, ...) IDx(it_operator(pps, _1) FOREACH22(it_operator, pps, __VA_ARGS__))
#define FOREACH24(it_operator, pps, _1, ...) IDx(it_operator(pps, _1) FOREACH23(it_operator, pps, __VA_ARGS__))
#define FOREACH25(it_operator, pps, _1, ...) IDx(it_operator(pps, _1) FOREACH24(it_operator, pps, __VA_ARGS__))
#define FOREACH26(it_operator, pps, _1, ...) IDx(it_operator(pps, _1) FOREACH25(it_operator, pps, __VA_ARGS__))
#define FOREACH27(it_operator, pps, _1, ...) IDx(it_operator(pps, _1) FOREACH26(it_operator, pps, __VA_ARGS__))
#define FOREACH28(it_operator, pps, _1, ...) IDx(it_operator(pps, _1) FOREACH27(it_operator, pps, __VA_ARGS__))
// clang-format on
#define FOREACH(pps, ...)                                                                                              \
    ID(__Choose30th(__VA_ARGS__,                                                                                       \
                    FOREACH28,                                                                                         \
                    FOREACH27,                                                                                         \
                    FOREACH26,                                                                                         \
                    FOREACH25,                                                                                         \
                    FOREACH24,                                                                                         \
                    FOREACH23,                                                                                         \
                    FOREACH22,                                                                                         \
                    FOREACH21,                                                                                         \
                    FOREACH20,                                                                                         \
                    FOREACH19,                                                                                         \
                    FOREACH18,                                                                                         \
                    FOREACH17,                                                                                         \
                    FOREACH16,                                                                                         \
                    FOREACH15,                                                                                         \
                    FOREACH14,                                                                                         \
                    FOREACH13,                                                                                         \
                    FOREACH12,                                                                                         \
                    FOREACH11,                                                                                         \
                    FOREACH10,                                                                                         \
                    FOREACH9,                                                                                          \
                    FOREACH8,                                                                                          \
                    FOREACH7,                                                                                          \
                    FOREACH6,                                                                                          \
                    FOREACH5,                                                                                          \
                    FOREACH4,                                                                                          \
                    FOREACH3,                                                                                          \
                    FOREACH2,                                                                                          \
                    FOREACH1,                                                                                          \
                    FOREACH0))                                                                                         \
    IDx((pps, __VA_ARGS__))

#define __EqualToPlain(x, y) || (x == y)
#define __EqualToPacket(px, y) CallxPPS(__EqualToPlain, (IDx px, y))
#define EqualToAnyOf(val, ...) (0 FOREACH(__EqualToPacket, (val), __VA_ARGS__))