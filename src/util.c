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
