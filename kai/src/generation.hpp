#pragma once
#include <kai/generation.h>
#include <string_view>
#include <unordered_map>

struct Typed_Identifier {
    kai_str name;
    kai_Type type;
};

using Type_Table = std::unordered_map<std::string_view, kai_Type_Info*>;
template <typename T> using Hash_Table = std::unordered_map<std::string_view, T>;
template <typename T> using Stack = std::vector<T>;

template <typename T>
           struct type_info_map { static constexpr kai_u32 ID = -1; };
template<> struct type_info_map <kai_Type_Info_Pointer> { static constexpr kai_u32 ID = kai_Type_Pointer; };

enum Byte_Code_Instrution_ID {
    Instr_Load_Argument,

    Instr_Add,
    Instr_Subtract,
    Instr_Multiply,
    Instr_Divide,

    Instr_Return,

    Instr_Convert_Float32_To_Int,
    Instr_Convert_Int_To_Float32,
};

#define VirtualReg_Immediate -2

struct VirtualReg {
    int index;
    kai_u64 value;
};

struct ByteCodeInstruction {
    kai_u32 intruction;
    VirtualReg left, right;
};

struct Type_Checker_Context {
	kai_Memory memory;
    Type_Table table; // this is terrible

    Stack<Typed_Identifier> ident_stack;
    Stack<int> scope_ident_count_stack;

    Hash_Table<int> ident_index_table;
    std::vector<ByteCodeInstruction> intruction_stream;

    template <typename TypeInfo>
    inline TypeInfo* alloc_type_info() {
        static_assert(type_info_map<TypeInfo>::ID != -1, "Must be a Type Info Structure");
        auto info = reinterpret_cast<TypeInfo*>(memory.alloc(memory.user, sizeof(TypeInfo)));
        info->type = type_info_map<TypeInfo>::ID;
        return info;
    }
};

