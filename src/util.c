#include "util.h"
#include "err.h"

void file_read(void *d, usize size, FILE *fd) {
    ERR_ASSERT_CLN(fread(d, 1, size, fd) == size, fclose(fd), "Error reading file");

   // for (usize i = 0; i < size; i++) {
   //     printf("%x", ((u8*)d)[i]);
   // }
   // printf("\n");
   // printf("-------------------------------");
   // printf("\n");
   // for (usize i = 0; i < size; i++) {
   //     u8 c = ((u8 *)d)[i];
   //     if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '1' && c <= '9') || c == '0')
   //         printf("%c", ((u8*)d)[i]);
   //     else
   //         printf("%c", '.');
   // }
   // printf("\n");
}

i32 sign(i32 v) {
    return v >= 0 ? 1 : -1;
}

i32 abs(i32 v) {
    return v >= 0 ? v : -1 * v;
}

i32 map(i32 val, i32 f_min, i32 f_max, i32 t_min, i32 t_max) {
    // May devide by zero!
    return t_min + (val - f_min) * ((double)(t_max - t_min) / (f_max - f_min));
}
