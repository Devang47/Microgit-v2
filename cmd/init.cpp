#include "init.hpp"
#include "../utils/main.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace cmd
{
    Command *initCmd = nullptr;

    int Init(const std::vector<std::string> &args)
    {
        std::string repoDir = utils::DEFAULT_PATH;
        std::string objectsDir = utils::DEFAULT_PATH + "/objects";
        std::string refsDir = utils::DEFAULT_PATH + "/refs";

        if (fs::exists(repoDir))
        {
            std::cout << "\nRepository already initialized." << std::endl;
            return 0;
        }

        try
        {
            fs::create_directory(repoDir);
            fs::permissions(repoDir, fs::perms::owner_all | fs::perms::group_read | fs::perms::group_exec | fs::perms::others_read | fs::perms::others_exec);

            fs::create_directory(objectsDir);
            fs::create_directory(refsDir);
            fs::create_directory(utils::DEFAULT_PATH + "/staging");

            // Create HEAD file pointing to nothing initially
            std::ofstream headFile(utils::DEFAULT_PATH + "/HEAD");
            headFile << "";
            headFile.close();

            // Create LATEST file pointing to nothing initially
            std::ofstream latestFile(utils::DEFAULT_PATH + "/LATEST");
            latestFile << "";
            latestFile.close();

            // Create empty index file
            std::ofstream indexFile(utils::DEFAULT_PATH + "/index");
            indexFile << "{}";
            indexFile.close();

            std::cout << "Initialized empty MicroGit repository in " << fs::absolute(repoDir) << std::endl;
            return 0;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error initializing repository: " << e.what() << std::endl;
            return 1;
        }
    }

    void InitInitCommand()
    {
        initCmd = new Command(
            "init",
            "Initialize a new MicroGit repository",
            "Initialize a new MicroGit repository in the current directory.\n"
            "This creates the necessary directory structure and files for version control.\n"
            "The repository will be initialized in a .microgit directory.");

        initCmd->SetRunFunc([](const std::vector<std::string> &args)
                            { Init(args); });
    }
} // namespace cmd
