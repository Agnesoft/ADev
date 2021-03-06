module;

#pragma warning(push)
#pragma warning(disable : 5105)
#pragma warning(disable : 5106)
#pragma warning(disable : 4005)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

module acore : process_windows;

import : acore_common;
#pragma warning(pop)

namespace acore
{
class SecurityAttributes
{
public:
    SecurityAttributes() :
        mSecurityAttributes{.nLength = sizeof(mSecurityAttributes),
                            .lpSecurityDescriptor = nullptr,
                            .bInheritHandle = TRUE}
    {
    }

    [[nodiscard]] constexpr auto get() noexcept -> SECURITY_ATTRIBUTES &
    {
        return mSecurityAttributes;
    }

private:
    SECURITY_ATTRIBUTES mSecurityAttributes;
};

class Handle
{
public:
    Handle() = default;

    Handle(const Handle &other) = delete;
    Handle(Handle &&other) noexcept = default;

    ~Handle()
    {
        close();
    }

    auto close() -> void
    {
        if (mHandle)
        {
            CloseHandle(mHandle);
            mHandle = nullptr;
        }
    }

    [[nodiscard]] constexpr auto get() noexcept -> HANDLE &
    {
        return mHandle;
    }

    auto operator=(const Handle &other) -> Handle & = delete;
    auto operator=(Handle &&other) noexcept -> Handle & = default;

private:
    HANDLE mHandle = nullptr;
};

class Pipe
{
public:
    Pipe()
    {
        if (CreatePipe(&mReadHandle.get(),
                       &mWriteHandle.get(),
                       &mSecurityAttributes.get(),
                       0)
            == FALSE)
        {
            throw std::runtime_error{"Failed to create Windows pipe: " + std::to_string(GetLastError())};
        }
    }

    auto close() -> void
    {
        mWriteHandle.close();
        mReadHandle.close();
    }

    [[nodiscard]] constexpr auto readHandle() noexcept -> HANDLE &
    {
        return mReadHandle.get();
    }

    [[nodiscard]] constexpr auto writeHandle() noexcept -> HANDLE &
    {
        return mWriteHandle.get();
    }

private:
    SecurityAttributes mSecurityAttributes;
    Handle mReadHandle;
    Handle mWriteHandle;
};

class StartupInfo
{
public:
    explicit StartupInfo(Pipe *pipe) :
        mStartupInfo{
            .cb = sizeof(mStartupInfo),
            .dwFlags = STARTF_USESTDHANDLES,
            .hStdOutput = pipe->writeHandle(),
            .hStdError = pipe->writeHandle()}
    {
    }

    [[nodiscard]] constexpr auto get() noexcept -> STARTUPINFO &
    {
        return mStartupInfo;
    }

private:
    STARTUPINFO mStartupInfo;
};

class ProcessInfo
{
public:
    ProcessInfo() :
        mProcessInfo{}
    {
    }

    ProcessInfo(const ProcessInfo &other) = delete;
    ProcessInfo(ProcessInfo &&other) noexcept = default;

    [[nodiscard]] constexpr auto get() noexcept -> PROCESS_INFORMATION &
    {
        return mProcessInfo;
    }

    auto operator=(const ProcessInfo &other) -> ProcessInfo & = delete;
    auto operator=(ProcessInfo &&other) noexcept -> ProcessInfo & = default;

private:
    PROCESS_INFORMATION mProcessInfo;
};

class AsyncReader
{
public:
    AsyncReader(std::string *output, HANDLE readHandle) :
        mThread{[output, readHandle] {
            constexpr size_t BUFFER_SIZE = 65536;
            static char buffer[BUFFER_SIZE] = {};
            DWORD bytesRead = 0;

            while (ReadFile(readHandle,
                            static_cast<LPVOID>(buffer),
                            static_cast<DWORD>(BUFFER_SIZE),
                            &bytesRead,
                            nullptr)
                       == TRUE
                   && bytesRead != 0)
            {
                output->append(buffer, bytesRead);
            }
        }}
    {
    }

    AsyncReader(const AsyncReader &other) = delete;
    AsyncReader(AsyncReader &&other) noexcept = default;

    ~AsyncReader()
    {
        if (mThread.joinable())
        {
            mThread.join();
        }
    }

    auto operator=(const AsyncReader &other) -> AsyncReader & = delete;
    auto operator=(AsyncReader &&other) noexcept -> AsyncReader & = default;

private:
    std::thread mThread;
};

class WindowsProcess
{
public:
    WindowsProcess(const std::string &command, const std::vector<std::string> &arguments, const std::string &workingDirectory)
    {
        Pipe pipe;
        StartupInfo startupInfo{&pipe};
        std::string commandLine = createCommandLine(command, arguments);

        if (CreateProcess(nullptr,
                          &commandLine.front(),
                          nullptr,
                          nullptr,
                          TRUE,
                          0,
                          nullptr,
                          workingDirectory.c_str(),
                          &startupInfo.get(),
                          &mProcessInfo.get())
            == FALSE)
        {
            throw std::runtime_error{"Failed to create WindowsProcess: " + std::to_string(GetLastError())};
        }

        AsyncReader reader{&mOutput, pipe.readHandle()};
        WaitForSingleObject(mProcessInfo.get().hProcess, INFINITE);
        pipe.close();
    }

    ~WindowsProcess()
    {
        CloseHandle(mProcessInfo.get().hProcess);
        CloseHandle(mProcessInfo.get().hThread);
    }

    WindowsProcess(const WindowsProcess &other) = delete;
    WindowsProcess(WindowsProcess &&other) noexcept = default;

    [[nodiscard]] auto exitCode() -> int
    {
        int exitCode = 0;

        if (GetExitCodeProcess(mProcessInfo.get().hProcess,
                               static_cast<LPDWORD>(static_cast<void *>(&exitCode)))
            == FALSE)
        {
            throw std::runtime_error{"Failed to get exit code from WindowsProcess: " + std::to_string(GetLastError())};
        }

        return exitCode;
    }

    [[nodiscard]] auto output() -> std::string
    {
        return mOutput;
    }

    auto operator=(const WindowsProcess &other) = delete;
    auto operator=(WindowsProcess &&other) noexcept = delete;

private:
    [[nodiscard]] static auto createCommandLine(const std::string &executable, const std::vector<std::string> &arguments) -> std::string
    {
        std::string command{executable};

        for (const auto &arg : arguments)
        {
            command += ' ';
            command += arg;
        }

        return command;
    }

    ProcessInfo mProcessInfo;
    std::string mOutput;
};
}
