module;

#ifdef _MSC_VER
#    include <typeinfo>
#endif

export module atest;

export import "astl.hpp";

namespace atest
{
template<typename T, typename... Values>
auto stringifyImpl(std::stringstream &stream, const T &value, const Values &...values) -> void
{
    stream << value;

    if constexpr (sizeof...(Values) > 0)
    {
        stringifyImpl(stream, values...);
    }
}

template<typename... T>
[[nodiscard]] auto stringify(const T &...values) -> std::string
{
    std::stringstream stream;
    stringifyImpl(stream, values...);
    return stream.str();
}

struct Failure
{
    std::string what;
    std::string expected;
    std::string actual;
};

struct Test
{
    std::string name;
    auto (*testBody)() -> void = nullptr;
    int expectations = 0;
    std::vector<Failure> failures;
};

struct TestSuite
{
    std::string name;
    std::vector<Test> tests;
};

class Printer
{
public:
    Printer() noexcept :
        mStream{&std::cout}
    {
    }

    explicit Printer(std::ostream *stream) noexcept :
        mStream{stream}
    {
    }

    auto beginTest(const Test *test) -> void
    {
        stream() << indent() << "Running \"" << std::left << std::setw(mTestWidth + 4) << (test->name + "\"...");
        mIndentLevel++;
    }

    auto beginTestSuite(const TestSuite *testSuite) -> void
    {
        stream() << "Running tests from \"" << testSuite->name << "\" test suite:\n";
        mTestWidth = testsWidth(testSuite);
        mIndentLevel++;
    }

    auto endTest(const Test *test) -> void
    {
        if (test->failures.empty())
        {
            stream() << " [SUCCESS]\n";
        }
        else
        {
            stream() << " [FAILED]\n";
            printTestFailures(test);
        }

        mIndentLevel--;
    }

    auto endTestSuite([[maybe_unused]] const TestSuite *testSuite) -> void
    {
        stream() << '\n';
        mIndentLevel--;
    }

    auto print(const std::string &text) -> void
    {
        if (!text.empty())
        {
            stream() << indent() << text << '\n';
        }
    }

private:
    [[nodiscard]] auto indent() const -> std::string
    {
        return std::string(mIndentLevel * 2, ' ');
    }

    [[nodiscard]] auto testWidth() const noexcept -> int
    {
        return mTestWidth;
    }

    [[nodiscard]] auto testsWidth(const TestSuite *testSuite) const -> int
    {
        int width = 0;

        for (const Test &test : testSuite->tests)
        {
            if (width < test.name.size())
            {
                width = test.name.size();
            }
        }

        return width;
    }

    auto printTestFailure(const Failure &failure) -> void
    {
        print(failure.what);
        print("  Expected: " + failure.expected);
        print("  Actual  : " + failure.actual);
    }

    auto printTestFailures(const Test *test) -> void
    {
        for (const Failure &failure : test->failures)
        {
            printTestFailure(failure);
        }
    }

    [[nodiscard]] auto stream() noexcept -> std::ostream &
    {
        return *mStream;
    }

    std::ostream *mStream = nullptr;
    int mIndentLevel = 0;
    int mTestWidth = 0;
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
            mPrinter.print("Unexpected exception when running tests.\n");
        }

        std::exit(mFailed ? EXIT_FAILURE : EXIT_SUCCESS);
    }

    auto addTest(const char *name, auto (*testBody)()->void) -> void
    {
        mCurrentTestSuite->tests.emplace_back(Test{name, testBody});
    }

    auto beginRecordTests(const char *testSuiteName) -> void
    {
        mCurrentTestSuite = &mTestSuites.emplace_back(TestSuite{testSuiteName});
    }

    [[nodiscard]] auto currentTest() const noexcept -> Test *
    {
        return mCurrentTest;
    }

    auto setPrinterStream(std::ostream *stream) -> void
    {
        mPrinter = Printer{stream};
    }

    auto stopRecordTests() -> void
    {
        mCurrentTestSuite = &mTestSuites.front();
    }

private:
    auto runTest(Test *test) -> void
    {
        mCurrentTest = test;
        mPrinter.beginTest(test);
        test->testBody();
        mPrinter.endTest(test);
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
        if (!testSuite->tests.empty())
        {
            mPrinter.beginTestSuite(testSuite);
            runTests(testSuite);
            mPrinter.endTestSuite(testSuite);
        }
    }

    auto runTestSuites() -> void
    {
        for (TestSuite &testSuite : mTestSuites)
        {
            runTestSuite(&testSuite);
        }
    }

    Printer mPrinter;
    std::vector<TestSuite> mTestSuites = std::vector<TestSuite>{{"Global"}};
    TestSuite *mCurrentTestSuite = &mTestSuites.front();
    Test *mCurrentTest = nullptr;
    bool mFailed = false;
};

[[nodiscard]] auto globalTestRunner() -> TestRunner *
{
    static TestRunner runner;
    return &runner;
}

export template<typename T>
class ExpectBase
{
public:
    explicit ExpectBase(const T &expression) noexcept :
        mExpression{expression}
    {
        globalTestRunner()->currentTest()->expectations++;
    }

    auto toFail() noexcept
    {
        mExpectFailure = true;
    }

protected:
    [[nodiscard]] auto expectFailure() const noexcept -> bool
    {
        return mExpectFailure;
    }

    [[nodiscard]] auto expression() const noexcept -> const T &
    {
        return mExpression;
    }

    auto handleFailure(Failure &&failure) -> void
    {
        if (!mExpectFailure)
        {
            fail(std::move(failure));
        }
    }

    auto handleSuccess() -> void
    {
        if (mExpectFailure)
        {
            fail(Failure{"Expected a failure but the test succeeded."});
        }
    }

private:
    auto fail(Failure failure) -> void
    {
        globalTestRunner()->currentTest()->expectations++;
        globalTestRunner()->currentTest()->failures.emplace_back(std::move(failure));
    }

    const T &mExpression;
    bool mExpectFailure = false;
};

export template<typename T, typename V>
class ExpectToBe : public ExpectBase<T>
{
public:
    ExpectToBe(const T &expression, const V &value) :
        ExpectBase<T>{expression},
        mValue{value}
    {
    }

    ~ExpectToBe()
    {
#ifdef _MSC_VER
        using ::type_info;
#endif

        try
        {
            const auto left = evaluateExpression();

            if (left == mValue)
            {
                this->handleSuccess();
            }
            else
            {
                this->handleFailure(Failure{"Values are not equal", stringify(left), stringify(mValue)});
            }
        }
        catch (std::exception &e)
        {
            this->handleFailure(Failure{"Unexpected exception thrown", stringify(mValue), stringify(typeid(e).name(), " '", e.what(), '\'')});
        }
        catch (...)
        {
            this->handleFailure(Failure{"Unexpected exception thrown"});
        }
    }

private:
    [[nodiscard]] auto evaluateExpression() const -> V
    {
        if constexpr (std::is_invocable<T>::value)
        {
            return this->expression()();
        }
        else
        {
            return this->expression();
        }
    }

    const V &mValue;
};

export template<typename T, typename E, bool ValidateText>
requires std::invocable<T> class ExpectToThrow : public ExpectBase<T>
{
public:
    using ExpectBase<T>::ExpectBase;

    ExpectToThrow(const T &expression, std::string exceptionText) :
        ExpectBase<T>{expression},
        mExceptionText{std::move(exceptionText)}
    {
    }

    ~ExpectToThrow()
    {
#ifdef _MSC_VER
        using ::type_info;
#endif

        try
        {
            this->expression()();
            this->handleFailure(Failure{"No exception thrown", stringify(typeid(E).name(), " '", mExceptionText, '\''), ""});
        }
        catch (E &e)
        {
            validateException(e);
        }
        catch (...)
        {
            handleUnknownException();
        }
    }

private:
    [[nodiscard]] auto exceptionTextMatches(E &e) -> bool
    {
        return mExceptionText == e.what();
    }

    auto exceptionTextMismatch(std::string what) -> void
    {
        this->handleFailure(Failure{"Exception text mismatch", mExceptionText, std::move(what)});
    }

    auto exceptionTypeMismatch(std::string recievedExceptionName) -> void
    {
#ifdef _MSC_VER
        using ::type_info;
#endif

        this->handleFailure(Failure{"Exception type mismatch", typeid(E).name(), std::move(recievedExceptionName)});
    }

    auto handleUnknownException()
    {
        try
        {
            throw;
        }
        catch (std::exception &e)
        {
#ifdef _MSC_VER
            using ::type_info;
#endif
            exceptionTypeMismatch(typeid(e).name());
        }
        catch (...)
        {
            exceptionTypeMismatch("unknown exception");
        }
    }

    auto validateException(E &e) -> void
    {
#ifdef _MSC_VER
        using ::type_info;
#endif
        if (typeid(E) == typeid(e))
        {
            validateExceptionText(e);
        }
        else
        {
            exceptionTypeMismatch(typeid(e).name());
        }
    }

    auto validateExceptionText(E &e) -> void
    {
        if constexpr (ValidateText)
        {
            if (exceptionTextMatches(e))
            {
                this->handleSuccess();
            }
            else
            {
                exceptionTextMismatch(e.what());
            }
        }
        else
        {
            this->handleSuccess();
        }
    }

    std::string mExceptionText;
};

export template<typename T>
class Expect
{
public:
    Expect(const T &expression) noexcept :
        mExpression{expression}
    {
    }

    template<typename V>
    auto toBe(const V &value) -> ExpectToBe<T, V>
    {
        return ExpectToBe<T, V>{mExpression, value};
    }

    template<typename E>
    requires std::invocable<T> auto toThrow() -> ExpectToThrow<T, E, false>
    {
        return ExpectToThrow<T, E, false>{mExpression};
    }

    template<typename E>
    requires std::invocable<T> auto toThrow(const std::string &exceptionText) -> ExpectToThrow<T, E, true>
    {
        return ExpectToThrow<T, E, true>{mExpression, exceptionText};
    }

private:
    const T &mExpression;
};

export auto suite(const char *name, auto (*suiteBody)()->void) -> int
{
    globalTestRunner()->beginRecordTests(name);
    suiteBody();
    globalTestRunner()->stopRecordTests();
    return 0;
}

export auto test(const char *name, auto (*testBody)()->void) -> void
{
    globalTestRunner()->addTest(name, testBody);
}

export template<typename T>
[[nodiscard]] auto expect(const T &value) noexcept -> Expect<T>
{
    return Expect<T>{value};
}

export auto setPrinterStream(std::ostream *stream) -> void
{
    globalTestRunner()->setPrinterStream(stream);
}
}
