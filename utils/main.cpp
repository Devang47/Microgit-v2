#include "main.hpp"
#include <iomanip>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <openssl/sha.h>

namespace fs = std::filesystem;

namespace utils
{

  std::string HashContent(const std::vector<uint8_t> &content)
  {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, content.data(), content.size());
    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
      ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
  }

  bool WriteObject(const std::string &hash, const std::vector<uint8_t> &content)
  {
    fs::path objectPath = fs::path(DEFAULT_PATH) / "objects" / hash;

    try
    {
      // Create directories if they don't exist
      fs::create_directories(objectPath.parent_path());

      std::ofstream file(objectPath, std::ios::binary);
      if (!file)
      {
        return false;
      }

      file.write(reinterpret_cast<const char *>(content.data()), content.size());
      return !file.fail();
    }
    catch (const std::exception &)
    {
      return false;
    }
  }

} // namespace utils
