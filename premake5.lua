
if _KAI_EXTERNAL then
	include 'kai'
else
	workspace 'kai_compiler'
	architecture "x86_64"
	configurations { 'Debug', 'Release', 'Dist' }
	
	flags { 'MultiProcessorCompile', 'MFC' }
--	defines '_CRT_SECURE_NO_WARNINGS'
	
	startproject 'demo'
	
	OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	

	include 'kai'  -- Library (.lib/.so)
	include 'demo' -- Demo program (executable)
end