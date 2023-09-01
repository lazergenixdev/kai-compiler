project 'kai'
    kind 'StaticLib'
    language 'C++'
	cppdialect 'c++20'

	defines { "KAI_BUILD" }
    targetdir ("%{wks.location}/bin/" .. OutputDir)
	objdir ("%{wks.location}/bin-int/" .. OutputDir .. "/%{prj.name}")

    files {
        "%{prj.location}/src/*",
        "%{wks.location}/include/kai/**.h", -- public API
        "%{prj.location}/src/backend/%{cfg.architecture}.cpp"
    }
	
	if (KAI_NO_DEBUG == nil) then
		files "%{prj.location}/src/debug/*"
	end
	
	includedirs {
		'%{wks.location}/include',
		'%{prj.location}/vendor',
	}
	
    -- AsmJit
    defines {
        "ASMJIT_EMBED",
        "ASMJIT_STATIC",
        "ASMJIT_NO_FOREIGN",
        "ASMJIT_NO_DEPRECATED", -- Disables deprecated API at compile time so it won't be available and the
    --  "ASMJIT_NO_BUILDER",    -- Disables \ref asmjit_builder functionality completely. This implies \ref ASMJIT_NO_COMPILER
    --  "ASMJIT_NO_COMPILER",   -- Disables \ref asmjit_compiler functionality completely.
    --  "ASMJIT_NO_INTROSPECTION",
        "ASMJIT_NO_JIT",        -- Disables JIT memory management and \ref JitRuntime.
        "ASMJIT_NO_LOGGING",    -- Disables \ref Logger and \ref Formatter.
    --  "ASMJIT_NO_TEXT",       -- Disables everything that contains string representation of AsmJit constants
        "ASMJIT_NO_VALIDATION", -- Disables validation API.
    }

    files "%{prj.location}/vendor/asmjit/core/*"

	filter "system:windows"
        files "%{prj.location}/src/platform/windows.cpp"
    
	filter "architecture:x86_64"
        files "%{prj.location}/vendor/asmjit/x86/*"
	
    filter "configurations:Debug"
        defines { "KAI_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "KAI_RELEASE" }
        optimize "On"

    filter "configurations:Dist"
        defines { "KAI_DIST" }
        optimize "Speed"
		