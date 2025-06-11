#define KAI_IMPLEMENTATION
#include "kai.h"
#include <string.h>
#if defined(KAI__PLATFORM_WINDOWS)
#include <windows.h> // --> EXCEPTION_ACCESS_VIOLATION
#include <excpt.h>
#endif

typedef Kai_s32 Main_Proc(Kai_slice);

void   Timer_Init(void);
void   Timer_Start(void);
double Timer_ElapsedSeconds(void);
double Timer_ElapsedMilliseconds(void);

void set_underline(int enable);
int load_file(const char* file_path, Kai_string* out);
void parse(char const* file, Kai_string source_code, Kai_Error* error, Kai_Allocator* allocator);

#define error(...) printf("\x1b[91mError\x1b[0m: "), printf(__VA_ARGS__), putchar('\n')

Kai_String_Writer* writer = NULL;

int main(int argc, char** argv)
{
    int exit_value = 1;
    struct {
        Kai_bool parse_only;
    } options = {
        .parse_only = KAI_FALSE,
    };
    writer = kai_writer_stdout();
    Timer_Init();
    
	set_underline(1);
    kai__write_string(kai_version_string());
    set_underline(0);
    kai__write_string(KAI_STRING("\n"));

    if (argc < 2) {
		printf("usage: kai [--parse-only] <SOURCE> [args...]\n");
		return 1;
	}

    char const* file = argv[1];

    if (strcmp(argv[1], "--parse-only") == 0) {
        if (argc < 3) {
		    error("no input file provided");
            return 1;
        }
        options.parse_only = KAI_TRUE;
        file = argv[2];
    }

    Kai_string source_code = {0};
    if (load_file(file, &source_code)) {
		error("unable to load file \"%s\"", argv[1]);
		return 1;
	}

    Kai_Error     error     = {0};
	Kai_Allocator allocator = {0};
    Kai_Program   program   = {0};
    
	kai_memory_create(&allocator);

    if (options.parse_only) {
        parse(file, source_code, &error, &allocator);
        goto cleanup;
    }

    Kai_Program_Create_Info info = {
        .allocator = allocator,
        .error = &error,
        .source = { source_code },
    };

    Timer_Start();
    Kai_Result result = kai_create_program_from_code(&info, &program);
    double elapsed_ms = Timer_ElapsedMilliseconds();

    printf("Compilation Took: %.4f ms\n", elapsed_ms);

    if (result != KAI_SUCCESS) {
        Kai_Error* curr = &error;
        int i = 0;
        while (curr && i < 10) {
		    curr->location.file_name = kai_string_from_c(file);
            curr->location.source = source_code.data;
            curr = curr->next;
            i += 1;
        }
        kai_write_error(writer, &error);
        goto cleanup;
    }

    void* main_raw = kai_find_procedure(program, KAI_STRING("main"), NULL);
    Main_Proc* main_proc = (Main_Proc*)main_raw;
    
    if (main_proc == NULL) {
		error("unable to find procedure \"main\"");
        goto cleanup;
    }

    {
        Kai_slice args = {0};
        if (argc > 2) {
            args.count = argc - 2;
            args.data = malloc(sizeof(Kai_string) * args.count);
            Kai_string* command_line = (Kai_string*)args.data;
            for (Kai_u32 i = 0; i < args.count; ++i) {
                command_line[i] = kai_string_from_c(argv[i + 2]);
            }
        }

#if defined(KAI__COMPILER_MSVC)
        __try
		{
			exit_value = main_proc(args);
		}
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
			const char* str = "Uknown";
			int code = GetExceptionCode();
			switch (code) {
				case EXCEPTION_ILLEGAL_INSTRUCTION: str = "EXCEPTION_ILLEGAL_INSTRUCTION";
				case EXCEPTION_ACCESS_VIOLATION:    str = "EXCEPTION_ACCESS_VIOLATION";
			}
			printf("Exception caught! %s\n", str);
		}
#else
        exit_value = main_proc(args);
#endif
    }
	
cleanup:
    kai_destroy_program(program);
    kai_destroy_error(&error, &allocator);
	if (kai_memory_destroy(&allocator) != KAI_SUCCESS) {
		error("Some allocations were not freed! (amount=%i B)", (int)kai_memory_usage(&allocator));
	}
	return exit_value;
}

void parse(char const* file, Kai_string source_code, Kai_Error* error, Kai_Allocator* allocator)\
{
    Kai_Syntax_Tree_Create_Info info = {
        .allocator = *allocator,
        .error = error,
        .source_code = source_code,
    };

    Kai_Syntax_Tree tree = {0};
    Kai_Result result = kai_create_syntax_tree(&info, &tree);
    if (result != KAI_SUCCESS) {
		error->location.file_name = kai_string_from_c(file);
        kai_write_error(writer, error);
    }
    else kai_write_syntax_tree(writer, &tree);
    kai_destroy_syntax_tree(&tree);
}

void set_underline(int enable)
{
    printf("\x1b[%im", enable ? 4 : 24);
}

int load_file(const char* file_path, Kai_string* out)
{
    FILE* file = kai__stdc_file_open(file_path, "rb");
    if (!file) return 1;
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    void* data = malloc(size);
    fseek(file, 0, SEEK_SET);
    fread(data, 1, size, file);
    fclose(file);
    out->count = (Kai_u32)size;
    out->data = data;
    return 0;
}

#if defined(_WIN32)
#include <Windows.h>

static LARGE_INTEGER frequency;
static LARGE_INTEGER start;

void Timer_Init(void)
{
	QueryPerformanceFrequency(&frequency);
}

void Timer_Start(void)
{
	QueryPerformanceCounter(&start);
}

double Timer_ElapsedSeconds(void)
{
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);
	LONGLONG duration = end.QuadPart - start.QuadPart;
	return (double)(duration) / (double)(frequency.QuadPart);
}

double Timer_ElapsedMilliseconds(void)
{
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);
	LONGLONG duration = end.QuadPart - start.QuadPart;
	return (double)(duration * 1000) / (double)(frequency.QuadPart);
}
#elif defined(__linux__) || defined(__APPLE__)
#include <sys/time.h>

void Timer_Init(void) {}

struct timespec start;

void Timer_Start(void)
{
	clock_gettime(CLOCK_MONOTONIC, &start);
}

double Timer_ElapsedSeconds(void)
{
    struct timespec end;
	clock_gettime(CLOCK_MONOTONIC, &end);
	uint64_t duration_ns = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
	return (double)duration_ns / 1e9;
}

double Timer_ElapsedMilliseconds(void)
{
    struct timespec end;
	clock_gettime(CLOCK_MONOTONIC, &end);
	uint64_t duration_ns = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
	return (double)duration_ns / 1e6;
}
#endif
