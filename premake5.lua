newoption({
	trigger = "glfw_use_wayland",
	description = "Should glfw use wayland for linux build",
	value = "boolean",
	allowed = {
		{ "N", "No" },
		{ "Y", "Yes" }
	},
	default = "N"
})

local workspaceName = "VulkanTemplate"
local programName = "VulkanProgram"

local vulkanSDKPath = os.getenv("VULKAN_SDK")

workspace(workspaceName)
	configurations({ "Debug", "Release", "Dist" })
	platforms({ "x64" })

	cppdialect("C++latest")
	rtti("On")
	exceptionhandling("On")
	flags("MultiProcessorCompile")
	defines({ "PREMAKE_SYSTEM_%{cfg.system}" })

	filter("configurations:Debug")
		defines({ "_DEBUG" })
		optimize("Off")
		symbols("On")

	filter("configurations:Release")
		defines({ "_RELEASE" })
		optimize("Full")
		symbols("On")

	filter("configurations:Dist")
		defines({ "_RELEASE" })
		optimize("Full")
		symbols("Off")

	filter("system:windows")
		toolset("msc")
		defines({ "NOMINMAX", "WIN32_LEAN_AND_MEAN", "_CRT_SECURE_NO_WARNINGS" })

	filter({})
	
	startproject(programName)
	
	group("Dependencies")
	project("GLFW")
		location("Deps/GLFW/")
		kind("StaticLib")
		targetdir("%{wks.location}/Int/%{cfg.system}-%{cfg.platform}-%{cfg.buildcfg}/%{prj.name}/")
		objdir("%{wks.location}/Int/%{cfg.system}-%{cfg.platform}-%{cfg.buildcfg}/%{prj.name}/")

		includedirs({ "%{prj.location}/include/" })

		files({
			"%{prj.location}/include/**",
			"%{prj.location}/src/glfw_config.h",
			"%{prj.location}/src/context.c",
			"%{prj.location}/src/init.c",
			"%{prj.location}/src/input.c",
			"%{prj.location}/src/monitor.c",
			"%{prj.location}/src/vulkan.c",
			"%{prj.location}/src/window.c"
		})

		filter("system:windows")
			files({
				"%{prj.location}/src/win32_init.c",
				"%{prj.location}/src/win32_monitor.c",
				"%{prj.location}/src/win32_window.c",
				"%{prj.location}/src/win32_joystick.c",
				"%{prj.location}/src/win32_time.c",
				"%{prj.location}/src/win32_thread.c",
				"%{prj.location}/src/wgl_context.c",
				"%{prj.location}/src/egl_context.c",
				"%{prj.location}/src/osmesa_context.c"
			})

			defines({ "_GLFW_WIN32" })

		filter("system:linux")
if _OPTIONS["glfw_use_wayland"] == "Y" then
			files({
				"%{prj.location}/src/wl_init.c",
				"%{prj.location}/src/wl_monitor.c",
				"%{prj.location}/src/wl_window.c",
				"%{prj.location}/src/linux_joystick.c",
				"%{prj.location}/src/posix_time.c",
				"%{prj.location}/src/xkb_unicode.c",
				"%{prj.location}/src/glx_context.c",
				"%{prj.location}/src/egl_context.c",
				"%{prj.location}/src/osmesa_context.c"
			})

			defines({ "_GLFW_WAYLAND" })
else
			files({
				"%{prj.location}/src/x11_init.c",
				"%{prj.location}/src/x11_monitor.c",
				"%{prj.location}/src/x11_window.c",
				"%{prj.location}/src/linux_joystick.c",
				"%{prj.location}/src/posix_time.c",
				"%{prj.location}/src/xkb_unicode.c",
				"%{prj.location}/src/glx_context.c",
				"%{prj.location}/src/egl_context.c",
				"%{prj.location}/src/osmesa_context.c"
			})

			defines({ "_GLFW_X11" })
end

		filter("system:macosx")
			files({
				"%{prj.location}/src/cocoa_init.c",
				"%{prj.location}/src/cocoa_monitor.c",
				"%{prj.location}/src/cocoa_window.c",
				"%{prj.location}/src/cocoa_joystick.c",
				"%{prj.location}/src/cocoa_time.c"
			})

			defines({ "_GLFW_COCOA" })

			links({
				"Cocoa.framework",
				"IOKit.framework",
				"CoreFoundation.framework"
			})

		filter({})

	project("Vulkan")
		location("Deps/Vulkan/")
		kind("StaticLib")
		--targetdir("%{wks.location}/Int/%{cfg.system}-%{cfg.platform}-%{cfg.buildcfg}/%{prj.name}/")
		--objdir("%{wks.location}/Int/%{cfg.system}-%{cfg.platform}-%{cfg.buildcfg}/%{prj.name}/")

		includedirs({ "%{prj.location}/include/" })

		files({
			"%{prj.location}/vulkan/**"
		})

	project("VMA")
		location("Deps/VMA/")
		kind("StaticLib")
		targetdir("%{wks.location}/Int/%{cfg.system}-%{cfg.platform}-%{cfg.buildcfg}/%{prj.name}/")
		objdir("%{wks.location}/Int/%{cfg.system}-%{cfg.platform}-%{cfg.buildcfg}/%{prj.name}/")
		removedefines({ "NOMINMAX", "WIN32_LEAN_AND_MEAN" })

		includedirs({ "%{prj.location}/include/" })
		sysincludedirs({
			"%{wks.location}/Deps/Vulkan/Vulkan-Headers/include/"
		})

		files({
			"%{prj.location}/include/**",
			"%{prj.location}/src/VmaUsage.h",
			"%{prj.location}/src/VmaUsage.cpp"
		})

	project("ImGUI")
		location("Deps/ImGUI/")
		kind("StaticLib")
		targetdir("%{wks.location}/Int/%{cfg.system}-%{cfg.platform}-%{cfg.buildcfg}/%{prj.name}/")
		objdir("%{wks.location}/Int/%{cfg.system}-%{cfg.platform}-%{cfg.buildcfg}/%{prj.name}/")

		includedirs({ "%{prj.location}/" })

		files({
			"%{prj.location}/imgui.h",
			"%{prj.location}/imgui.cpp",
			"%{prj.location}/imgui_draw.cpp",
			"%{prj.location}/imgui_internal.h",
			"%{prj.location}/imgui_tables.cpp",
			"%{prj.location}/imgui_widgets.cpp",
			"%{prj.location}/imstb_rectpack.h",
			"%{prj.location}/imstb_textedit.h",
			"%{prj.location}/imstb_truetype.h"
		})

	project("STB")
		location("Deps/STB/")
		kind("StaticLib")
		targetdir("%{wks.location}/Int/%{cfg.system}-%{cfg.platform}-%{cfg.buildcfg}/%{prj.name}/")
		objdir("%{wks.location}/Int/%{cfg.system}-%{cfg.platform}-%{cfg.buildcfg}/%{prj.name}/")

		includedirs({ "%{prj.location}/include/" })

		files({
			"%{prj.location}/stb_image.h"
		})

	group("Program")
	project(programName)
		location(programName)
		targetdir("%{wks.location}/Bin/%{cfg.system}-%{cfg.platform}-%{cfg.buildcfg}/")
		objdir("%{wks.location}/Int/%{cfg.system}-%{cfg.platform}-%{cfg.buildcfg}/" .. programName .. "/")
		debugdir("%{prj.location}/")

		filter("configurations:Debug")
			kind("ConsoleApp")

		filter("configurations:Release or Dist")
			kind("WindowedApp")

		filter({})

		links({ "GLFW", vulkanSDKPath .. "/Lib/vulkan-1", "VMA", "ImGUI" })
		sysincludedirs({
			"%{wks.location}/Deps/GLFW/include/",
			"%{wks.location}/Deps/Vulkan/Vulkan-Headers/include/",
			"%{wks.location}/Deps/Vulkan/vulkan/",
			"%{wks.location}/Deps/VMA/include/",
			"%{wks.location}/Deps/ImGUI/",
			"%{wks.location}/Deps/STB/"
		})

		includedirs({ "%{prj.location}/src" })

		files({ "%{prj.location}/src/**" })