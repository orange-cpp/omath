//
// Created by Vlad on 11/8/2025.
//

#include <iostream>
#include <omath/utility/pe_pattern_scan.hpp>
#include <print>

int main()
{
    std::println("OMATH Signature Scanner");

    std::print("Enter path to PE file: ");
    std::string file_path;
    std::getline(std::cin, file_path); // allows spaces

    std::print("Enter target section: ");
    std::string section;
    std::getline(std::cin, section);

    std::print("Enter signature: ");
    std::string signature;
    std::getline(std::cin, signature);

    std::println("[LOG] Performing scan....");

    const auto result = omath::PePatternScanner::scan_for_pattern_in_file(file_path, signature, section);

    if (!result)
    {
        std::println("[ERROR] Scan failed or signature was not found");
        return -1;
    }

    std::println("Found at virtual 0x{:x} , raw 0x{:x}", result->virtual_base_addr + result->target_offset,
               result->raw_base_addr + result->target_offset);

    return 0;
}