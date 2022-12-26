#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "logging/test.h"

void cpp_fun() {
    for (int i = 0; i < 10; i++){
        debug("ala");
    }
}

#ifdef __cplusplus
}
#endif