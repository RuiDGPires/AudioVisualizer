#include "util.h"
#include "err.h"

void file_read(void *d, usize size, FILE *fd) {
    ERR_ASSERT_CLN(fread(d, 1, size, fd) == size, fclose(fd), "Error reading file");
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

double mapf(double val, double f_min, double f_max, double t_min, double t_max) {
    // May devide by zero!
    return t_min + (val - f_min) * ((t_max - t_min) / (f_max - f_min));
}
