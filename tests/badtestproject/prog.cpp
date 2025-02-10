template <bool B>
struct CompileFail {
    static_assert(B, "This will fail at compile time.");
};

CompileFail<false> test;


int main(int argc, char const *argv[])
{
    int a = 1 + 2;
    return 0;
}
