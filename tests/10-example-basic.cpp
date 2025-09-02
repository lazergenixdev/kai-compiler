#define KAI_IMPLEMENTATION
#include "../kai.h"
#include <string>

#define Make_Procedure_Type(Name, Ty) \
    struct Name { \
        static constexpr const char* Repr = #Ty; \
        using Proc = Ty; \
    }

Make_Procedure_Type(on_jump, void(Kai_Context* context, Kai_u32 objid, Kai_vector2_f32 pos));

void print(Kai_int);

// This will be part of header eventually
namespace Kai {
    using u8 = Kai_u8;
    using u32 = Kai_u32;
    using Type = Kai_Type;
    using Source = Kai_Source;
    using Writer = Kai_Writer;

    struct String: public Kai_string {
        String() = default;
        String(const char* cstr): Kai_string(kai_string_from_c(cstr)) {}
        String(std::string_view view): Kai_string({
            .count = static_cast<Kai_u32>(view.size()),
            .data = reinterpret_cast<Kai_u8*>(const_cast<char*>(view.data())),
        }) {}
    };

    static auto StdOut() -> Writer* {
        static Writer writer;
        writer = kai_writer_stdout();
        return &writer;
    }

    struct Error: public Kai_Error {
        Error() = default;
        operator bool() const {
            return result != KAI_SUCCESS;
        }
        void write(Writer* writer = StdOut()) const {
            auto error = const_cast<Error*>(this);
            kai_write_error(writer, error);
        }
    };

    struct Program: public Kai_Program {
        using CreateInfo = Kai_Program_Create_Info;

        Program() = default;
        
        auto create(CreateInfo info) -> Error {
            Error error {};
            info.error = &error;
            kai_create_program(&info, this);
            return error;
        }

        template <typename Proc>
        auto find_procedure(String name, Type opt_type) -> Proc {
            void* proc = kai_find_procedure(this, name, opt_type);
            return reinterpret_cast<Proc>(proc);
        }
    };
}

#define BIND_SCRIPT_PROC(name) name = program.find_procedure<decltype(name)>(#name, nullptr)

struct Script {
    Kai::Program program = {};
    on_jump::Proc* on_jump = {};

    Script(Kai::Source source) {
        auto err = program.create({.sources = {.count = 1, .data = &source}});
        if (err) {
            err.write();
            return;
        }
        BIND_SCRIPT_PROC(on_jump);
    }
};

int main()
{
    Kai::Program program;
    auto hello_world = program.find_procedure<Kai_int(*)()>("hello_world", nullptr);
    Kai_int n = hello_world();

    // #nocontext

    Script script = Script({});
    Kai_Context context;// = kai_default_context();
    script.on_jump(&context, 23, {.x = 2, .y = 4});
}