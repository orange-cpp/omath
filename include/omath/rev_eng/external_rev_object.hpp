//
// Created by Vlad on 10/4/2025.
//

#pragma once
#include <cstddef>
#include <cstdint>

namespace omath::rev_eng
{
    template<class ExternalMemoryManagementTrait>
    class ExternalReverseEngineeredObject
    {
    public:
        explicit ExternalReverseEngineeredObject(const std::uintptr_t addr): m_object_address(addr)
        {
        }
    private:
        std::uintptr_t m_object_address{};

    protected:
        template<class Type>
        [[nodiscard]]
        Type get_by_offset(const std::ptrdiff_t offset) const
        {
            return ExternalMemoryManagementTrait::read_memory(m_object_address+offset);
        }

        template<class Type>
        void set_by_offset(const std::ptrdiff_t offset, const Type& value) const
        {
            return ExternalMemoryManagementTrait::write_memory(m_object_address+offset, value);
        }
    };
} // namespace omath::rev_eng