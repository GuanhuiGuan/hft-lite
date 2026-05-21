#include "macros.hpp"

int main()
{
    ASSERT(true, "right");
    // ASSERT(false, "wrong");
    FATAL("dead");

    return 0;
}