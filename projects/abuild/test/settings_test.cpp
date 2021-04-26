import atest;
import abuild;

using atest::expect;
using atest::suite;
using atest::test;

static const auto testSuite = suite("abuild::Settings", [] {
    test("type traits", [] {

    });

    test("set", [] {
        abuild::BuildCache cache;
        abuild::Settings{cache};
    });
});
