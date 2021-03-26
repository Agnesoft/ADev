export module atest;

import<cstdlib>;
import<iostream>;
import<stdexcept>;
export import<sstream>;
import<string>;
import<vector>;

namespace atest
{
struct Test
{
    const char *description = "";
    auto (*testBody)() -> void = nullptr;
};

struct TestSuite
{
    const char *name = "";
    std::vector<Test> tests;
};

class TestFailedException : public std::exception
{
public:
    explicit TestFailedException(std::string description) noexcept :
        mDescription{std::move(description)}
    {
    }

    [[nodiscard]] auto what() const noexcept -> const char * override
    {
        return mDescription.c_str();
    }

private:
    std::string mDescription;
};

class TestThrewException : public TestFailedException
{
public:
    using TestFailedException::TestFailedException;
};

class TestRunner
{
public:
    ~TestRunner()
    {
        try
        {
            runTestSuites();
        }
        catch (...)
        {
            mFailed = true;
            std::cout << "Unexpected exception when running tests.\n";
        }

        std::exit(mFailed ? EXIT_FAILURE : EXIT_SUCCESS);
    }

    auto addTest(const char *name, auto (*testBody)()->void) -> void
    {
        mCurrentSuite->tests.emplace_back(Test{name, testBody});
    }

    auto beginSuite(const char *name) -> void
    {
        mCurrentSuite = &mTestSuites.emplace_back(TestSuite{name, {}});
    }

    auto endSuite() -> void
    {
        mCurrentSuite = &mTestSuites.front();
    }

private:
    auto runTest(Test *test) -> void
    {
        std::cout << "  [" << test->description << "]... ";

        try
        {
            test->testBody();
            std::cout << "SUCCESS\n";
        }
        catch (std::exception &e)
        {
            mFailed = true;
            std::cout << "FAILED\n    " << e.what() << '\n';
        }
        catch (...)
        {
            mFailed = true;
            std::cout << "FAILED\n    Unknown exception\n";
        }
    }

    auto runTests(TestSuite *testSuite) -> void
    {
        for (Test &test : testSuite->tests)
        {
            runTest(&test);
        }
    }

    auto runTestSuite(TestSuite *testSuite) -> void
    {
        std::cout << "Running tests from '" << testSuite->name << "':\n";
        runTests(testSuite);
        std::cout << '\n';
    }

    auto runTestSuites() -> void
    {
        for (TestSuite &testSuite : mTestSuites)
        {
            runTestSuite(&testSuite);
        }
    }

    std::vector<TestSuite> mTestSuites = std::vector<TestSuite>{{"Global", {}}};
    TestSuite *mCurrentSuite = &mTestSuites.front();
    bool mFailed = false;
};

TestRunner RUNNER = TestRunner{};

export auto test(const char *description, auto (*testBody)()->void) -> void
{
    RUNNER.addTest(description, testBody);
}

export auto suite(const char *name, auto (*suiteBody)()->void) -> int
{
    RUNNER.beginSuite(name);
    suiteBody();
    RUNNER.endSuite();
    return 0;
}

export template<typename T>
class Expect
{
public:
    Expect(const T &expression) noexcept :
        mExpression{expression}
    {
    }

    template<typename V>
    auto toBe(const V &value) -> void
    {
        const auto left = expressionValue<V>();

        if (!(left == value))
        {
            std::stringstream stream;
            stream << left << " != " << value;
            throw TestFailedException{stream.str()};
        }
    }

private:
    template<typename V>
    [[nodiscard]] auto expressionValue() const -> V
    {
        if constexpr (std::is_invocable<T>::value)
        {
            try
            {
                return mExpression();
            }
            catch (std::exception &e)
            {
                throw TestThrewException{e.what()};
            }
            catch (...)
            {
                throw TestThrewException{"Unknown exception"};
            }
        }
        else
        {
            return mExpression;
        }
    }

    const T &mExpression;
};

export template<typename T>
[[nodiscard]] auto expect(const T &value) noexcept -> Expect<T> &&
{
    return std::move(Expect<T>{value});
}
}
