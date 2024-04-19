#include <iostream>
#include <string>
#include <vector>
#include "lccl/fmt.h"
#include "test_fmt.h"

bool TestFmt(int argc, char **argv)
{
    std::vector<std::string> ans;

    ans.push_back(fmt::format("1:{} 2:{} 3:{}", 1, 2LL, "3"));
    ans.push_back(fmt::format("123"));

    for (size_t i = 0; i < ans.size(); ++i)
    {
        std::cout << i << ": " << ans[i] << std::endl;
    }

    return true;
}
