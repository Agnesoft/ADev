#ifdef _MSC_VER
export module abuild : project_scanner;
export import : build_cache;
import : settings;
#endif

namespace abuild
{
export class ProjectScanner
{
public:
    explicit ProjectScanner(BuildCache &cache) :
        mBuildCache{cache}
    {
        scanDirectory(mBuildCache.projectRoot());
    }

private:
    auto appendProjectName(std::string *projectName, const std::string &directoryName) const -> void
    {
        if (projectName->empty())
        {
            *projectName = directoryName;
        }
        else
        {
            *projectName += mBuildCache.settings().projectNameSeparator() + directoryName;
        }
    }

    [[nodiscard]] auto ensureProjectName(const std::string &projectName) const -> std::string
    {
        if (projectName.empty())
        {
            return mBuildCache.projectRoot().filename().string();
        }
        else
        {
            return projectName;
        }
    }

    [[nodiscard]] auto isIgnoreDirectory(const std::filesystem::path &path) -> bool
    {
        return path.filename().string().front() == '.' || mBuildCache.settings().ignoreDirectories().contains(path.filename().string());
    }

    [[nodiscard]] auto isSkipDirectory(const std::filesystem::path &path) -> bool
    {
        return mBuildCache.settings().skipDirectories().contains(path.filename().string());
    }

    [[nodiscard]] auto isSquashDirectory(const std::filesystem::path &path) -> bool
    {
        return mBuildCache.settings().squashDirectories().contains(path.filename().string());
    }

    [[nodiscard]] auto isTestDirectory(const std::filesystem::path &path) -> bool
    {
        return mBuildCache.settings().testDirectories().contains(path.filename().string());
    }

    [[nodiscard]] auto pathDirectories(std::filesystem::path path) -> std::vector<std::filesystem::path>
    {
        std::vector<std::filesystem::path> directories;

        while (path != mBuildCache.projectRoot())
        {
            directories.push_back(path);
            path = path.parent_path();
        }

        return directories;
    }

    auto processFile(const std::filesystem::path &path, const std::string &projectName) -> void
    {
        if (mBuildCache.settings().cppSourceExtensions().contains(path.extension().string()))
        {
            mBuildCache.addSource(path, projectName);

            if (mBuildCache.settings().executableFilenames().contains(path.stem().string()))
            {
                mBuildCache.project(projectName)->setType(Project::Type::Executable);
            }
        }
        else if (mBuildCache.settings().cppHeaderExtensions().contains(path.extension().string()))
        {
            mBuildCache.addHeader(path, projectName);
        }
    }

    [[nodiscard]] auto projectNameFromDirectories(const std::vector<std::filesystem::path> &directories) -> std::string
    {
        std::string projectName;

        for (auto it = directories.crbegin(); it != directories.crend() && !isSquashDirectory(*it); ++it)
        {
            if (isTestDirectory(*it))
            {
                return ensureProjectName(projectName) + mBuildCache.settings().projectNameSeparator() + it->filename().string();
            }
            else if (!isSkipDirectory(*it))
            {
                appendProjectName(&projectName, it->filename().string());
            }
        }

        return ensureProjectName(projectName);
    }

    [[nodiscard]] auto projectNameFromPath(const std::filesystem::path &path) -> std::string
    {
        return projectNameFromDirectories(pathDirectories(path));
    }

    auto scanDirectoryEntry(const std::filesystem::directory_entry &entry, const std::string &projectName) -> void
    {
        const std::filesystem::path entryPath = entry.path();

        if (std::filesystem::is_regular_file(entryPath))
        {
            processFile(entryPath, projectName);
        }
        else if (std::filesystem::is_directory(entryPath) && !isIgnoreDirectory(entryPath))
        {
            scanDirectory(entryPath);
        }
    }

    auto scanDirectory(const std::filesystem::path &path) -> void
    {
        const std::string projectName = projectNameFromPath(path);

        for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(path))
        {
            scanDirectoryEntry(entry, projectName);
        }

        Project *project = mBuildCache.project(projectName);

        if (isTestDirectory(path) && project)
        {
            project->setType(Project::Type::Executable);
        }
    }

    BuildCache &mBuildCache;
};
}
