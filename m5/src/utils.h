#ifndef LAMP_UTILH_H
#define LAMP_UTILS_H

#include <stdint.h>
#include <IPAddress.h>

void turnOffLcd();

template<typename T>
size_t print(T &&t) {
#if !defined(NO_OUTPUT)
    return Serial.print(t);
#endif
}

inline constexpr size_t print() { return 0; }

template<typename T, typename ...R>
size_t print(T &&t, R&&... r) {
    return print(std::forward<T>(t)) + print(std::forward<R>(r)...);
}

inline size_t print(const IPAddress &addr) {
    return print(addr[0], '.', addr[1], '.', addr[2], '.', addr[3]);
}

template<typename ...Ts>
size_t println(Ts&& ...ts) {
    return print(std::forward<Ts>(ts)...) + print("\r\n");
}

#endif