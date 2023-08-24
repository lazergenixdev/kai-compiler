project 'kai'
    kind 'StaticLib'
    language 'C++'
	cppdialect 'c++20'

	defines { "KAI_BUILD" }
    targetdir ("%{wks.location}/bin/" .. OutputDir)
	objdir ("%{wks.location}/bin-int/" .. OutputDir .. "/%{prj.name}")

    files {
        "%{prj.location}/src/*",
        "%{wks.location}/include/kai/**.h" -- public API
    }
	
	if (KAI_NO_DEBUG == nil) then
		files "%{prj.location}/src/debug/*"
	end
	
	includedirs {
		'%{wks.location}/include',
	}
	
	filter "system:windows"
        files "%{prj.location}/src/platform/windows.cpp"
	
    filter "configurations:Debug"
        defines { "KAI_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "KAI_RELEASE" }
        optimize "On"

    filter "configurations:Dist"
        defines { "KAI_DIST" }
        optimize "Speed"
		