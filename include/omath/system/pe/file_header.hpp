//
// Created by Vlad on 10/7/2025.
//

#pragma once
#include <cstdint>

namespace omath::system::pe
{
    enum class MachineId : std::uint16_t
    {
        UNKNOWN = 0x0000,
        TARGET_HOST = 0x0001, // Useful for indicating we want to interact with the host and not a WoW guest.
        I386 = 0x014C, // Intel 386.
        R3000 = 0x0162, // MIPS little-endian, 0x160 big-endian
        R4000 = 0x0166, // MIPS little-endian
        R10000 = 0x0168, // MIPS little-endian
        WCEMIPSV2 = 0x0169, // MIPS little-endian WCE v2
        ALPHA = 0x0184, // Alpha_AXP
        SH3 = 0x01A2, // SH3 little-endian
        SH3DSP = 0x01A3,
        SH3E = 0x01A4, // SH3E little-endian
        SH4 = 0x01A6, // SH4 little-endian
        SH5 = 0x01A8, // SH5
        ARM = 0x01C0, // ARM Little-Endian
        THUMB = 0x01C2, // ARM Thumb/Thumb-2 Little-Endian
        ARMNT = 0x01C4, // ARM Thumb-2 Little-Endian
        AM33 = 0x01D3,
        POWERPC = 0x01F0, // IBM PowerPC Little-Endian
        POWERPCP = 0x01F1,
        IA64 = 0x0200, // Intel 64
        MIPS16 = 0x0266, // MIPS
        ALPHA64 = 0x0284, // ALPHA64
        MIPSFPU = 0x0366, // MIPS
        MIPSFPU16 = 0x0466, // MIPS
        AXP64 = 0x0284,
        TRICORE = 0x0520, // Infineon
        CEF = 0x0CEF,
        EBC = 0x0EBC, // EFI Byte Code
        AMD64 = 0x8664, // AMD64 (K8)
        M32R = 0x9041, // M32R little-endian
        ARM64 = 0xAA64, // ARM64 Little-Endian
        CEE = 0xC0EE,
    };

    struct FileHeader final
    {
        MachineId machine;
        uint16_t num_sections;
        uint32_t timedate_stamp;
        uint32_t ptr_symbols;
        uint32_t num_symbols;
        uint16_t size_optional_header;
        std::uint16_t characteristics;
    };
}