add_rules("mode.debug", "mode.release")

add_rules("utils.install.cmake_importfiles")
add_rules("utils.install.pkgconfig_importfiles")

set_version("4.6.1", {soname = "4.6"})
set_languages("cxx23")
set_warnings("all")

option("avx2")
    set_default(true)
    set_showmenu(true)
    add_defines("OMATH_ENABLE_AVX2")
    add_vectorexts("avx2")

    after_check(function (option)
        import("core.base.cpu")
        option:enable(cpu.has_feature("avx2"))
    end)
option_end()

option("imgui")
    set_default(true)
    set_showmenu(true)
    add_defines("OMATH_IMGUI_INTEGRATION")
option_end()

option("examples")
    set_default(true)
    set_showmenu(true)
option_end()

if has_config("imgui") then
    add_requires("imgui")
end

if has_config("examples") then
    add_requires("glew", "glfw")
end

target("omath")
    set_kind("static")
    add_files("source/**.cpp")
    add_includedirs("include", {public = true})
    add_headerfiles("include/(**.hpp)", {prefixdir = "omath"})
    if has_config("imgui") then
        add_packages("imgui")
    end
    on_config(function (target)
        if has_config("avx2") then
            cprint("${green} ✔️ AVX2 supported")
        else
            cprint("${red} ❌ AVX2 not supported")
        end
    end)

target("example_projection_matrix_builder")
    set_languages("cxx26")
    set_kind("binary")
    add_files("examples/example_proj_mat_builder.cpp")
    add_deps("omath")
    set_enabled(has_config("examples"))

target("example_signature_scan")
    set_languages("cxx26")
    set_kind("binary")
    add_files("examples/example_signature_scan.cpp")
    add_deps("omath")
    set_enabled(has_config("examples"))

target("example_glfw3")
    set_languages("cxx26")
    set_kind("binary")
    add_files("examples/example_glfw3.cpp")
    add_deps("omath")
    add_packages("glew", "glfw")
    set_enabled(has_config("examples"))
