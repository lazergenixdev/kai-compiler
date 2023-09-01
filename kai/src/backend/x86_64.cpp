#include "../program.hpp"
#include <asmjit/asmjit.h>
#include "../bytecode_stream.hpp"
#include <iostream>
#include <vector>
#include "../config.hpp"

using namespace asmjit;

/*
auto in = compiler.newInt32();
auto addr = compiler.newGpq();

compiler.mov(in, 69);
compiler.mov(addr, print);

InvokeNode* invokeNode;      // Function invocation:
compiler.invoke(&invokeNode, // - InvokeNode (output).
	addr,                    // - Function address or Label.
	FuncSignatureT<void, int>(CallConvId::kCDecl));

invokeNode->setArg(0, in);
*/
void print(int i) {
	printf("%i\n", i);
}

TypeId prim_type_id(Primitive_Type pt) {
	switch (pt)
	{
	case Prim_Type_u8:      return TypeId::kUInt8;
	case Prim_Type_u16:     return TypeId::kUInt16;
	case Prim_Type_u32:     return TypeId::kUInt32;
	case Prim_Type_u64:     return TypeId::kUInt64;
	case Prim_Type_s8:      return TypeId::kInt8;
	case Prim_Type_s16:     return TypeId::kInt16;
	case Prim_Type_s32:     return TypeId::kInt32;
	case Prim_Type_s64:     return TypeId::kInt64;
	case Prim_Type_f32:     return TypeId::kFloat32;
	case Prim_Type_f64:     return TypeId::kFloat64;
	case Prim_Type_Pointer: return TypeId::kUIntPtr;
	default:                return TypeId::kVoid;
	}
}

struct Code_Generation_Context {
	x86::Compiler compiler;
	std::vector<x86::Reg> registers;

	FuncNode* func;
	std::vector<x86::Reg> args;

	x86::Reg get_register(u32 reg_index, TypeId type_id) {
		// procedure input
		if (reg_index > MAX_PROC_ARG_REGISTER)
			return args[PROC_ARG(reg_index)];

		while (reg_index >= registers.size()) {
			registers.emplace_back(compiler.newReg(type_id));
		}
		return registers[reg_index];
	}

	void init_procedure(FuncSignature signature) {
		func = compiler.addFunc(signature);
		registers.clear();
		args.clear();

		for range(signature.argCount()) {
			auto arg_reg = compiler.newReg(signature.arg(i));
			func->setArg(i, arg_reg);
			args.emplace_back(std::move(arg_reg));
		}
	}

	template <typename T>
	T _load_and_inc(u8* bytecode, u64& index) {
		auto ptr_value = reinterpret_cast<T*>(bytecode + index);
		index += sizeof(T);
		return *ptr_value;
	}

	void generate_procedure(u8* bytecode, u64 size) {
		u64 index = 0;

		while (index < size) {
			auto instr = bytecode[index++];
			switch (instr)
			{
			default:
				panic_with_message("undefined operation");
			break; case Operation_Add: {
				u8 prim_type = bytecode[index++];
				u32 dst_reg_index = _load_and_inc<u32>(bytecode, index);
				u32 left_reg_index = _load_and_inc<u32>(bytecode, index);
				u8 is_imm = bytecode[index++]; // assume false
				u32 right_reg_index = _load_and_inc<u32>(bytecode, index);
				
				auto dst_reg = get_register(dst_reg_index, TypeId::kInt64);
				auto left_reg = get_register(left_reg_index, TypeId::kInt64);
				auto right_reg = get_register(right_reg_index, TypeId::kInt64);

				if (dst_reg.isGp()) {
					auto dst   = dst_reg  .as<x86::Gp>();
					auto left  = left_reg .as<x86::Gp>();
					auto right = right_reg.as<x86::Gp>();

					compiler.mov(dst, left);
					compiler.add(dst, right);
				}
			}
			break; case Operation_Mul: {
				u8 prim_type = bytecode[index++];
				u32 dst_reg_index = _load_and_inc<u32>(bytecode, index);
				u32 left_reg_index = _load_and_inc<u32>(bytecode, index);
				u8 is_imm = bytecode[index++]; // assume false
				u32 right_reg_index = _load_and_inc<u32>(bytecode, index);
				
				auto dst_reg = get_register(dst_reg_index, TypeId::kInt64);
				auto left_reg = get_register(left_reg_index, TypeId::kInt64);
				auto right_reg = get_register(right_reg_index, TypeId::kInt64);

				if (dst_reg.isGp()) {
					auto dst   = dst_reg  .as<x86::Gp>();
					auto left  = left_reg .as<x86::Gp>();
					auto right = right_reg.as<x86::Gp>();

					compiler.mov(dst, left);
					compiler.imul(dst, right);
				}
			}
			break; case Operation_Return: {
				u8 ret_count = bytecode[index++];
				u32 value_reg_index = _load_and_inc<u32>(bytecode, index);
				
				auto value_reg = get_register(value_reg_index, TypeId::kVoid);

				compiler.ret(value_reg);
			}
			}
		}

		compiler.endFunc();
	}
};

// TODO: need to set the base address properly, -> create program here
Machine_Code generate_machine_code(void* bytecode, kai_u64 bytecode_size) {
	// Setup
	CodeHolder holder;
	auto env = Environment::host();
	env.setObjectFormat(ObjectFormat::kJIT);
	holder.init(env, CpuInfo::host().features());

	Code_Generation_Context ctx;
	holder.attach(&ctx.compiler);

	FuncSignature sig;
	TypeId args[2];
	args[0] = TypeId::kInt64;
	args[1] = TypeId::kInt64;
	sig.init(CallConvId::kCDecl, FuncSignature::kNoVarArgs,
		TypeId::kInt64, args, 2
	);

	ctx.init_procedure(sig);
	ctx.generate_procedure((u8*)bytecode, bytecode_size);

	ctx.compiler.finalize();

	auto size = holder.codeSize();
	auto data = malloc(size);

	auto r = holder.copyFlattenedData(data, size);

	std::cout << "\ntranslation result = ";
	std::cout << DebugUtils::errorAsString(r) << '\n';

	if (r) panic();

	return { data, size };
}
