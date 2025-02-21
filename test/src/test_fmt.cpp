#include "lccl/fmt.h"
#include "test_fmt.h"

void TestFmt(int argc, char **argv)
{
    fmt::printf("%d\n", 123456);
    fmt::println(fmt::format("1:{} 2:{} 3:{}", 1, 2LL, "3"));
    fmt::print(fmt::fg(fmt::terminal_color::bright_yellow), fmt::format("123\n"));
}
