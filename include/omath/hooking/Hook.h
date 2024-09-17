//
// Created by Vlad on 17.09.2024.
//

#pragma once
#include <any>
#include <cstdint>


namespace omath::hooking
{
    enum opcodes : uint8_t
    {

    };
    class Hook
    {
    public:
        Hook(void* source, void* detour);

    };
}