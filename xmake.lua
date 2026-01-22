add_rules("mode.debug", "mode.release")

add_rules("utils.install.cmake_importfiles")
add_rules("utils.install.pkgconfig_importfiles")

set_version("4.6.1", {soname = "4.6"})
set_languages("cxx23")
set_warnings("all")

option("avx2")
    set_default(true)
    set_showmenu(true)

    after_check(function (option)
        import("core.base.cpu")
        option:enable(cpu.has_feature("avx2"))
    end)
option_end()

option("imgui")
    set_default(true)
    set_showmenu(true)
option_end()

option("examples")
    set_default(true)
    set_showmenu(true)
option_end()

option("tests")
    set_default(true)
    set_showmenu(true)
option_end()

if has_config("avx2") then
    add_defines("OMATH_ENABLE_AVX2")
    add_vectorexts("avx2")
end

if has_config("imgui") then
    add_defines("OMATH_IMGUI_INTEGRATION")
    add_requires("imgui")
    add_packages("imgui")
end

if has_config("examples") then
    add_requires("glew", "glfw")
end

if has_config("tests") then
    add_requires("gtest")
end

target("omath")
    set_kind("shared")
    add_files("source/**.cpp")
    add_includedirs("include", {public = true})
    add_headerfiles("include/(**.hpp)", {prefixdir = "omath"})
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

for _, file in ipairs(os.files("tests/**.cpp")) do
    local name = path.basename(file)
    target(name)
        set_languages("cxx23")
        set_kind("binary")
        add_files(file, "tests/main.cpp")
        add_deps("omath")
        add_packages("gtest")
        add_defines("OMATH_BUILD_TESTS")
        add_tests("default")
        set_default(false)
        set_enabled(has_config("tests"))
end

task("check")
    on_run(function ()
        import("core.project.task")
        for _, file in ipairs(os.files("tests/**.cpp")) do
            local name = path.basename(file)
            task.run("run", {target = name})
        end
    end)
    set_menu {
        usage = "xmake check", description = "Run tests !", options = {}
    }
