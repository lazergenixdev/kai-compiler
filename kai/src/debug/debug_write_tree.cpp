#include <kai/debug.h>
#include <cstdio>
#include "tree_traverser.hpp"

static constexpr char8_t const* branches[4] = {
	u8"│   ",
	u8"├───",
	u8"    ",
	u8"└───",
};

struct Write_Tree_Traverser : public Syntax_Tree_Traverser {
    kai_Debug_String_Writer* writer;
	std::vector<u8> stack;
    const char* prefix;
	char temp[32];
	
	Write_Tree_Traverser(kai_Debug_String_Writer* writer)
	: writer(writer), prefix(nullptr), temp()
	{
		stack.reserve(128);
	}

	char const* binary_operator_name(kai_u32 op) {
		switch (op)
		{
		case token_2("->"):return "cast";
		case token_2("&&"):return "and";
		case token_2("||"):return "or";
		case '+':return "add";
		case '-':return "subtract";
		case '*':return "multiply";
		case '/':return "divide";
		case '.':return "member access";
		default:
			snprintf(temp, sizeof(temp), "undefined(%u)", op);
			return temp;
		}
	}
	char const* unary_operator_name(kai_u32 op) {
		switch (op)
		{
		case '-':return "negate";
		case '*':return "pointer to";
		default:
			snprintf(temp, sizeof(temp), "undefined(%u)", op);
			return temp;
		}
	}

	virtual bool visit_begin(kai_Expr node, u8 is_last) override {
		stack.push_back(is_last);

		_set_color(kai_debug_color_secondary);

		auto const last = stack.size() - 1;
		for_n(stack.size()) {
			u8 index = (stack[i] << 1) | u8(i == last);
			_write((char*)branches[index]);
		}

		if (prefix) {
			_set_color(kai_debug_color_important_2);
			_write(prefix);
			_write_char(' ');
			prefix = nullptr;
		}

		_set_color(kai_debug_color_primary);

		if (node == nullptr) {
			_write("null\n");
			return false;
		}
		return true;
	}

	virtual void visit_end() override {
		stack.pop_back();
	}

	virtual void visit_unknown(kai_Expr node) override {
        _write("unknown (id = ");
        _write_format("%i", node->id);
        _write_char(')');
        _write_char('\n');
	}

	virtual void visit_identifier(kai_Expr_Identifier* node) override {
		_write("identifier \"");
		_set_color(kai_debug_color_important);
		_write_string(node->source_code);
		_set_color(kai_debug_color_primary);
		_write("\"\n");
	}

	virtual void visit_number(kai_Expr_Number* node) override {
		_write("number \"");
		_set_color(kai_debug_color_important);
		_write_string(node->source_code);
		_set_color(kai_debug_color_primary);
		_write("\"\n");
	}

	virtual void visit_string(kai_Expr_String* node) override {
		_write("string \"");
		_set_color(kai_debug_color_important);
		_write_string(node->source_code);
		_set_color(kai_debug_color_primary);
		_write("\"\n");
	}

	virtual void visit_binary(kai_Expr_Binary* node) override {
		_write("binary (op = ");
		_set_color(kai_debug_color_important);
		_write(binary_operator_name(node->op));
		_set_color(kai_debug_color_primary);
		_write(")\n");

		visit(node->left);
		visit(node->right, true);
	}

	virtual void visit_unary(kai_Expr_Unary* node) override {
		_write("unary (op = ");
		_set_color(kai_debug_color_important);
		_write(unary_operator_name(node->op));
		_set_color(kai_debug_color_primary);
		_write(")\n");

		visit(node->expr, true);
	}

	virtual void visit_procedure_type(kai_Expr_Procedure_Type* node) override {
		_write("procedure type (");
		_write_format("%u", node->param_count);
		_write(" in, ");
		_write_format("%u", node->ret_count);
		_write(" out)\n");

		auto end = node->param_count + node->ret_count - 1;

		for_n(node->param_count) {
			prefix = "in";
			visit(node->input_output[i], i == end);
		}

		for_n(node->ret_count) {
			prefix = "out";
			auto idx = i + node->param_count;
			visit(node->input_output[idx], idx == end);
		}
	}

	virtual void visit_procedure_call(kai_Expr_Procedure_Call* node) override {
		_write("procedure call\n");

		prefix = "proc";
		visit(node->proc, node->arg_count == 0);

		for_n(node->arg_count)
			visit(node->arguments[i], i == node->arg_count - 1);
	}

	virtual void visit_procedure(kai_Expr_Procedure* node) override {
		_write("procedure (");
		_write_format("%u", node->param_count);
		_write(" in, ");
		_write_format("%u", node->ret_count);
		_write(" out)\n");

		for_n(node->param_count) {
			prefix = "in";
			visit(node->input_output[i].type);
		}

		for_n(node->ret_count) {
			prefix = "out";
			visit(node->input_output[i+node->param_count].type);
		}

		visit(node->body, true);
	}

	virtual void visit_return(kai_Stmt_Return* node) override {
		_write("return\n");
		visit(node->expr, true);
	}

	virtual void visit_declaration(kai_Stmt_Declaration* node) override {
		_write("declaration (name = \"");
		_set_color(kai_debug_color_important);
		_write_string(node->name);
		_set_color(kai_debug_color_primary);
		_write("\") ");

		if (node->flags & kai_Decl_Flag_Const) _write("CONST ");

		_write_char('\n');

		visit(node->expr, true);
	}

	virtual void visit_compound(kai_Stmt_Compound* node) override {
		_write("compound statement\n");

		auto const n = node->count;
		for_n(n) visit(node->statements[i], i == n - 1);
	}
};

struct Write_Type_Traverser : public Type_Tree_Traverser {
	kai_Debug_String_Writer* writer;
	std::vector<u8> stack;
	char temp[32];
	const char* prefix;

	Write_Type_Traverser(kai_Debug_String_Writer* writer)
	: writer(writer), prefix(nullptr), temp()
	{
		stack.reserve(8);
	}

	virtual bool visit_begin(kai_Type node, u8 is_last) {
		stack.push_back(is_last);

		_set_color(kai_debug_color_secondary);

		auto const last = stack.size() - 1;
		for_n(stack.size()) {
			u8 index = (stack[i] << 1) | u8(i == last);
			_write((char*)branches[index]);
		}

		if (prefix) {
			_set_color(kai_debug_color_important_2);
			_write(prefix);
			_write_char(' ');
			prefix = nullptr;
		}

		_set_color(kai_debug_color_primary);

		if (node == nullptr) {
			_write("null\n");
			return false;
		}
		return true;
	}

	virtual void visit_end() {
		stack.pop_back();
	}

	virtual void visit_unknown(kai_Type node) override {
		_write("unknown (type = ");
		_write_format("%i", node->type);
		_write_char(')');
		_write_char('\n');
	}
	virtual void visit_type(kai_Type node) override {
		_set_color(kai_debug_color_important);
		_write("type\n");
		_set_color(kai_debug_color_primary);
	}
	virtual void visit_integer(kai_Type_Info_Integer* node) override {
		_write(node->is_signed ? "signed" : "unsigned");
		_set_color(kai_debug_color_important);
		_write_char(' ');
		_write_format("int%i\n", (int)node->bits);
		_set_color(kai_debug_color_primary);
	}
	virtual void visit_float(kai_Type_Info_Float* node) override {
		_set_color(kai_debug_color_important);
		_write_format("float%i\n", (int)node->bits);
		_set_color(kai_debug_color_primary);
	}
	virtual void visit_pointer(kai_Type_Info_Pointer* node) override {
		_write("pointer to\n");
		visit(node->sub_type, true);
	}
	virtual void visit_procedure(kai_Type_Info_Procedure* node) override {
		_write("procedure (");
		_write_format("%u", node->param_count);
		_write(" in, ");
		_write_format("%u", node->ret_count);
		_write(" out)\n");

		auto end = node->param_count + node->ret_count - 1;

		for_n(node->param_count) {
			prefix = "in";
			visit(node->input_output[i], i == end);
		}

		for_n(node->ret_count) {
			prefix = "out";
			auto idx = i + node->param_count;
			visit(node->input_output[idx], idx == end);
		}
	}

};

void kai_debug_write_syntax_tree(kai_Debug_String_Writer* writer, kai_AST* ast) {
	Write_Tree_Traverser traverser{writer};

	_write("Top Level\n");

	auto count = ast->toplevel_count;
	for_n(count) {
		traverser.visit(ast->toplevel_stmts[i], i == (count - 1));
	}
}

void kai_debug_write_type(kai_Debug_String_Writer* writer, kai_Type type) {
	Write_Type_Traverser traverser{writer};
	traverser.visit(type, true);
}

void kai_debug_write_expression(kai_Debug_String_Writer* writer, kai_Expr expr) {
	Write_Tree_Traverser traverser{writer};
	traverser.visit(expr, true);
}
