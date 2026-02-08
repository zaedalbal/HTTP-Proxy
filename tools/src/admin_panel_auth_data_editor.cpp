#include <openssl/evp.h>
#include <openssl/rand.h>
#include <toml++/toml.hpp>
#include <array>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

constexpr std::uint32_t PBKDF2_ITERATIONS = 200'000;
constexpr std::size_t SALT_SIZE = 16;
constexpr std::size_t HASH_SIZE = 32;
constexpr std::string_view ALGORITHM = "PBKDF2-HMAC-SHA256";

std::string to_hex(std::span<const std::uint8_t> data)
{
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (auto b : data)
        oss << std::setw(2) << static_cast<int>(b);
    return oss.str();
}

std::vector<std::uint8_t> generate_salt()
{
    std::vector<std::uint8_t> salt(SALT_SIZE);
    if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1)
        throw std::runtime_error("RAND_bytes failed");
    return salt;
}

std::vector<std::uint8_t> hash_password(
    std::string_view password,
    std::span<const std::uint8_t> salt)
{
    std::vector<std::uint8_t> hash(HASH_SIZE);
    if (PKCS5_PBKDF2_HMAC(
            password.data(),
            static_cast<int>(password.size()),
            salt.data(),
            static_cast<int>(salt.size()),
            PBKDF2_ITERATIONS,
            EVP_sha256(),
            static_cast<int>(hash.size()),
            hash.data()) != 1)
    {
        throw std::runtime_error("PBKDF2 failed");
    }
    return hash;
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage:\n";
        std::cerr << "  add_admin_user add <file.toml> <login> <password>\n";
        std::cerr << "  add_admin_user remove <file.toml> <login>\n";
        return 1;
    }

    std::string command = argv[1];
    std::filesystem::path file_path = argv[2];

    try
    {
        toml::table root;
        if (std::filesystem::exists(file_path))
            root = toml::parse_file(file_path.string());

        if (command == "add")
        {
            if (argc != 5)
            {
                std::cerr << "Usage: add_admin_user add <file.toml> <login> <password>\n";
                return 1;
            }

            std::string login = argv[3];
            std::string password = argv[4];

            auto salt = generate_salt();
            auto hash = hash_password(password, salt);

            toml::table account;
            account.insert("login", login);
            account.insert("algorithm", std::string(ALGORITHM));
            account.insert("iterations", PBKDF2_ITERATIONS);
            account.insert("salt", to_hex(salt));
            account.insert("hash", to_hex(hash));

            root.insert_or_assign(login, std::move(account));

            std::ofstream file(file_path);
            file << root;

            std::cout << "Admin user '" << login << "' added successfully\n";
        }
        else if (command == "remove")
        {
            if (argc != 4)
            {
                std::cerr << "Usage: add_admin_user remove <file.toml> <login>\n";
                return 1;
            }

            std::string login = argv[3];

            if (root.contains(login))
            {
                root.erase(login);
                std::ofstream file(file_path);
                file << root;
                std::cout << "Admin user '" << login << "' removed successfully\n";
            }
            else
            {
                std::cerr << "Login '" << login << "' not found\n";
                return 2;
            }
        }
        else
        {
            std::cerr << "Unknown command: " << command << "\n";
            return 1;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 2;
    }

    return 0;
}
