```
BrushEngine
|-- 3rdparty
|   |-- imgui
|   |   |-- .github
|   |   |   |-- ISSUE_TEMPLATE
|   |   |   |   |-- config.yml
|   |   |   |   -- issue_template.yml
|   |   |   |-- workflows
|   |   |   |   |-- build.yml
|   |   |   |   |-- build_windows_vulkan_libs.ps1
|   |   |   |   |-- manual.yml
|   |   |   |   |-- scheduled.yml
|   |   |   |   -- static-analysis.yml
|   |   |   |-- FUNDING.yml
|   |   |   -- pull_request_template.md
|   |   |-- backends
|   |   |   |-- sdlgpu3
|   |   |   |   |-- build_instructions.txt
|   |   |   |   |-- shader.frag
|   |   |   |   -- shader.vert
|   |   |   |-- vulkan
|   |   |   |   |-- build_instructions.txt
|   |   |   |   |-- generate_spv.sh
|   |   |   |   |-- glsl_shader.frag
|   |   |   |   -- glsl_shader.vert
|   |   |   |-- imgui_impl_allegro5.cpp
|   |   |   |-- imgui_impl_allegro5.h
|   |   |   |-- imgui_impl_android.cpp
|   |   |   |-- imgui_impl_android.h
|   |   |   |-- imgui_impl_dx10.cpp
|   |   |   |-- imgui_impl_dx10.h
|   |   |   |-- imgui_impl_dx11.cpp
|   |   |   |-- imgui_impl_dx11.h
|   |   |   |-- imgui_impl_dx12.cpp
|   |   |   |-- imgui_impl_dx12.h
|   |   |   |-- imgui_impl_dx9.cpp
|   |   |   |-- imgui_impl_dx9.h
|   |   |   |-- imgui_impl_glfw.cpp
|   |   |   |-- imgui_impl_glfw.h
|   |   |   |-- imgui_impl_glut.cpp
|   |   |   |-- imgui_impl_glut.h
|   |   |   |-- imgui_impl_metal.h
|   |   |   |-- imgui_impl_metal.mm
|   |   |   |-- imgui_impl_null.cpp
|   |   |   |-- imgui_impl_null.h
|   |   |   |-- imgui_impl_opengl2.cpp
|   |   |   |-- imgui_impl_opengl2.h
|   |   |   |-- imgui_impl_opengl3.cpp
|   |   |   |-- imgui_impl_opengl3.h
|   |   |   |-- imgui_impl_opengl3_loader.h
|   |   |   |-- imgui_impl_osx.h
|   |   |   |-- imgui_impl_osx.mm
|   |   |   |-- imgui_impl_sdl2.cpp
|   |   |   |-- imgui_impl_sdl2.h
|   |   |   |-- imgui_impl_sdl3.cpp
|   |   |   |-- imgui_impl_sdl3.h
|   |   |   |-- imgui_impl_sdlgpu3.cpp
|   |   |   |-- imgui_impl_sdlgpu3.h
|   |   |   |-- imgui_impl_sdlgpu3_shaders.h
|   |   |   |-- imgui_impl_sdlrenderer2.cpp
|   |   |   |-- imgui_impl_sdlrenderer2.h
|   |   |   |-- imgui_impl_sdlrenderer3.cpp
|   |   |   |-- imgui_impl_sdlrenderer3.h
|   |   |   |-- imgui_impl_vulkan.cpp
|   |   |   |-- imgui_impl_vulkan.h
|   |   |   |-- imgui_impl_wgpu.cpp
|   |   |   |-- imgui_impl_wgpu.h
|   |   |   |-- imgui_impl_win32.cpp
|   |   |   -- imgui_impl_win32.h
|   |   |-- docs
|   |   |   |-- BACKENDS.md
|   |   |   |-- CHANGELOG.txt
|   |   |   |-- CONTRIBUTING.md
|   |   |   |-- EXAMPLES.md
|   |   |   |-- FAQ.md
|   |   |   |-- FONTS.md
|   |   |   |-- README.md
|   |   |   |-- SECURITY.md
|   |   |   -- TODO.txt
|   |   |-- examples
|   |   |   |-- example_allegro5
|   |   |   |   |-- imconfig_allegro5.h
|   |   |   |   |-- main.cpp
|   |   |   |   -- README.md
|   |   |   |-- example_android_opengl3
|   |   |   |   |-- android
|   |   |   |   |   |-- app
|   |   |   |   |   |   |-- src
|   |   |   |   |   |   |   -- main
|   |   |   |   |   |   |       |-- java
|   |   |   |   |   |   |       |   -- MainActivity.kt
|   |   |   |   |   |   |       -- AndroidManifest.xml
|   |   |   |   |   |   -- build.gradle
|   |   |   |   |   |-- gradle
|   |   |   |   |   |   -- libs.versions.toml
|   |   |   |   |   |-- .gitignore
|   |   |   |   |   |-- build.gradle
|   |   |   |   |   -- settings.gradle
|   |   |   |   |-- CMakeLists.txt
|   |   |   |   -- main.cpp
|   |   |   |-- example_apple_metal
|   |   |   |   |-- example_apple_metal.xcodeproj
|   |   |   |   |   -- project.pbxproj
|   |   |   |   |-- iOS
|   |   |   |   |   |-- Info-iOS.plist
|   |   |   |   |   -- LaunchScreen.storyboard
|   |   |   |   |-- macOS
|   |   |   |   |   |-- Info-macOS.plist
|   |   |   |   |   -- MainMenu.storyboard
|   |   |   |   |-- main.mm
|   |   |   |   |-- Makefile
|   |   |   |   -- README.md
|   |   |   |-- example_apple_opengl2
|   |   |   |   |-- example_apple_opengl2.xcodeproj
|   |   |   |   |   -- project.pbxproj
|   |   |   |   |-- main.mm
|   |   |   |   -- Makefile
|   |   |   |-- example_glfw_metal
|   |   |   |   |-- main.mm
|   |   |   |   -- Makefile
|   |   |   |-- example_glfw_opengl2
|   |   |   |   |-- build_win32.bat
|   |   |   |   |-- main.cpp
|   |   |   |   -- Makefile
|   |   |   |-- example_glfw_opengl3
|   |   |   |   |-- build_win32.bat
|   |   |   |   |-- main.cpp
|   |   |   |   |-- Makefile
|   |   |   |   -- Makefile.emscripten
|   |   |   |-- example_glfw_vulkan
|   |   |   |   |-- build_win32.bat
|   |   |   |   |-- build_win64.bat
|   |   |   |   |-- CMakeLists.txt
|   |   |   |   |-- main.cpp
|   |   |   |   -- Makefile
|   |   |   |-- example_glfw_wgpu
|   |   |   |   |-- CMakeLists.txt
|   |   |   |   |-- main.cpp
|   |   |   |   |-- Makefile.emscripten
|   |   |   |   -- README.md
|   |   |   |-- example_glut_opengl2
|   |   |   |   |-- main.cpp
|   |   |   |   -- Makefile
|   |   |   |-- example_null
|   |   |   |   |-- build_win32.bat
|   |   |   |   |-- main.cpp
|   |   |   |   -- Makefile
|   |   |   |-- example_sdl2_directx11
|   |   |   |   |-- build_win32.bat
|   |   |   |   -- main.cpp
|   |   |   |-- example_sdl2_metal
|   |   |   |   |-- main.mm
|   |   |   |   -- Makefile
|   |   |   |-- example_sdl2_opengl2
|   |   |   |   |-- build_win32.bat
|   |   |   |   |-- main.cpp
|   |   |   |   |-- Makefile
|   |   |   |   -- README.md
|   |   |   |-- example_sdl2_opengl3
|   |   |   |   |-- build_win32.bat
|   |   |   |   |-- main.cpp
|   |   |   |   |-- Makefile
|   |   |   |   |-- Makefile.emscripten
|   |   |   |   -- README.md
|   |   |   |-- example_sdl2_sdlrenderer2
|   |   |   |   |-- build_win32.bat
|   |   |   |   |-- main.cpp
|   |   |   |   |-- Makefile
|   |   |   |   -- README.md
|   |   |   |-- example_sdl2_vulkan
|   |   |   |   |-- build_win32.bat
|   |   |   |   |-- build_win64.bat
|   |   |   |   |-- main.cpp
|   |   |   |   -- Makefile
|   |   |   |-- example_sdl2_wgpu
|   |   |   |   |-- CMakeLists.txt
|   |   |   |   |-- main.cpp
|   |   |   |   |-- Makefile.emscripten
|   |   |   |   -- README.md
|   |   |   |-- example_sdl3_directx11
|   |   |   |   |-- build_win32.bat
|   |   |   |   -- main.cpp
|   |   |   |-- example_sdl3_metal
|   |   |   |   |-- main.mm
|   |   |   |   -- Makefile
|   |   |   |-- example_sdl3_opengl3
|   |   |   |   |-- build_win32.bat
|   |   |   |   |-- build_win64.bat
|   |   |   |   |-- main.cpp
|   |   |   |   |-- Makefile
|   |   |   |   |-- Makefile.emscripten
|   |   |   |   -- README.md
|   |   |   |-- example_sdl3_sdlgpu3
|   |   |   |   |-- build_win64.bat
|   |   |   |   |-- main.cpp
|   |   |   |   -- Makefile
|   |   |   |-- example_sdl3_sdlrenderer3
|   |   |   |   |-- build_win32.bat
|   |   |   |   |-- main.cpp
|   |   |   |   -- Makefile
|   |   |   |-- example_sdl3_vulkan
|   |   |   |   |-- build_win32.bat
|   |   |   |   |-- build_win64.bat
|   |   |   |   |-- main.cpp
|   |   |   |   -- Makefile
|   |   |   |-- example_sdl3_wgpu
|   |   |   |   |-- CMakeLists.txt
|   |   |   |   |-- main.cpp
|   |   |   |   |-- Makefile.emscripten
|   |   |   |   -- README.md
|   |   |   |-- example_win32_directx10
|   |   |   |   |-- build_win32.bat
|   |   |   |   -- main.cpp
|   |   |   |-- example_win32_directx11
|   |   |   |   |-- build_win32.bat
|   |   |   |   -- main.cpp
|   |   |   |-- example_win32_directx12
|   |   |   |   |-- build_win32.bat
|   |   |   |   -- main.cpp
|   |   |   |-- example_win32_directx9
|   |   |   |   |-- build_win32.bat
|   |   |   |   -- main.cpp
|   |   |   |-- example_win32_opengl3
|   |   |   |   |-- build_mingw.bat
|   |   |   |   |-- build_win32.bat
|   |   |   |   -- main.cpp
|   |   |   |-- example_win32_vulkan
|   |   |   |   |-- build_win32.bat
|   |   |   |   |-- build_win64.bat
|   |   |   |   -- main.cpp
|   |   |   |-- libs
|   |   |   |   |-- emscripten
|   |   |   |   |   |-- emscripten_mainloop_stub.h
|   |   |   |   |   -- shell_minimal.html
|   |   |   |   |-- glfw
|   |   |   |   |   |-- include
|   |   |   |   |   |   -- GLFW
|   |   |   |   |   |       |-- glfw3.h
|   |   |   |   |   |       -- glfw3native.h
|   |   |   |   |   |-- lib-vc2010-32
|   |   |   |   |   |-- lib-vc2010-64
|   |   |   |   |   -- COPYING.txt
|   |   |   |   -- usynergy
|   |   |   |       |-- README.txt
|   |   |   |       |-- uSynergy.c
|   |   |   |       -- uSynergy.h
|   |   |   -- README.txt
|   |   |-- misc
|   |   |   |-- cpp
|   |   |   |   |-- imgui_stdlib.cpp
|   |   |   |   |-- imgui_stdlib.h
|   |   |   |   -- README.txt
|   |   |   |-- debuggers
|   |   |   |   |-- imgui.gdb
|   |   |   |   |-- imgui.natstepfilter
|   |   |   |   |-- imgui.natvis
|   |   |   |   |-- imgui_lldb.py
|   |   |   |   -- README.txt
|   |   |   |-- fonts
|   |   |   |   |-- binary_to_compressed_c.cpp
|   |   |   |   |-- Cousine-Regular.ttf
|   |   |   |   |-- DroidSans.ttf
|   |   |   |   |-- Karla-Regular.ttf
|   |   |   |   |-- ProggyClean.ttf
|   |   |   |   |-- ProggyTiny.ttf
|   |   |   |   -- Roboto-Medium.ttf
|   |   |   |-- freetype
|   |   |   |   |-- imgui_freetype.cpp
|   |   |   |   |-- imgui_freetype.h
|   |   |   |   -- README.md
|   |   |   |-- single_file
|   |   |   |   -- imgui_single_file.h
|   |   |   -- README.txt
|   |   |-- .editorconfig
|   |   |-- .gitattributes
|   |   |-- .gitignore
|   |   |-- imconfig.h
|   |   |-- imgui.cpp
|   |   |-- imgui.h
|   |   |-- imgui_demo.cpp
|   |   |-- imgui_draw.cpp
|   |   |-- imgui_internal.h
|   |   |-- imgui_tables.cpp
|   |   |-- imgui_widgets.cpp
|   |   |-- imstb_rectpack.h
|   |   |-- imstb_textedit.h
|   |   |-- imstb_truetype.h
|   |   -- LICENSE.txt
|   |-- spdlog
|   |   |-- .github
|   |   |   -- workflows
|   |   |       |-- linux.yml
|   |   |       |-- macos.yml
|   |   |       -- windows.yml
|   |   |-- bench
|   |   |   |-- async_bench.cpp
|   |   |   |-- bench.cpp
|   |   |   |-- CMakeLists.txt
|   |   |   |-- formatter-bench.cpp
|   |   |   |-- latency.cpp
|   |   |   -- utils.h
|   |   |-- cmake
|   |   |   |-- ide.cmake
|   |   |   |-- pch.h.in
|   |   |   |-- spdlog.pc.in
|   |   |   |-- spdlog_header_only.pc.in
|   |   |   |-- spdlogConfig.cmake.in
|   |   |   |-- spdlogCPack.cmake
|   |   |   |-- utils.cmake
|   |   |   -- version.rc.in
|   |   |-- example
|   |   |   |-- CMakeLists.txt
|   |   |   -- example.cpp
|   |   |-- include
|   |   |   -- spdlog
|   |   |       |-- cfg
|   |   |       |   |-- argv.h
|   |   |       |   |-- env.h
|   |   |       |   |-- helpers.h
|   |   |       |   -- helpers-inl.h
|   |   |       |-- details
|   |   |       |   |-- backtracer.h
|   |   |       |   |-- backtracer-inl.h
|   |   |       |   |-- circular_q.h
|   |   |       |   |-- console_globals.h
|   |   |       |   |-- file_helper.h
|   |   |       |   |-- file_helper-inl.h
|   |   |       |   |-- fmt_helper.h
|   |   |       |   |-- log_msg.h
|   |   |       |   |-- log_msg_buffer.h
|   |   |       |   |-- log_msg_buffer-inl.h
|   |   |       |   |-- log_msg-inl.h
|   |   |       |   |-- mpmc_blocking_q.h
|   |   |       |   |-- null_mutex.h
|   |   |       |   |-- os.h
|   |   |       |   |-- os-inl.h
|   |   |       |   |-- periodic_worker.h
|   |   |       |   |-- periodic_worker-inl.h
|   |   |       |   |-- registry.h
|   |   |       |   |-- registry-inl.h
|   |   |       |   |-- synchronous_factory.h
|   |   |       |   |-- tcp_client.h
|   |   |       |   |-- tcp_client-windows.h
|   |   |       |   |-- thread_pool.h
|   |   |       |   |-- thread_pool-inl.h
|   |   |       |   |-- udp_client.h
|   |   |       |   |-- udp_client-windows.h
|   |   |       |   -- windows_include.h
|   |   |       |-- fmt
|   |   |       |   |-- bundled
|   |   |       |   |   |-- args.h
|   |   |       |   |   |-- base.h
|   |   |       |   |   |-- chrono.h
|   |   |       |   |   |-- color.h
|   |   |       |   |   |-- compile.h
|   |   |       |   |   |-- core.h
|   |   |       |   |   |-- fmt.license.rst
|   |   |       |   |   |-- format.h
|   |   |       |   |   |-- format-inl.h
|   |   |       |   |   |-- os.h
|   |   |       |   |   |-- ostream.h
|   |   |       |   |   |-- printf.h
|   |   |       |   |   |-- ranges.h
|   |   |       |   |   |-- std.h
|   |   |       |   |   -- xchar.h
|   |   |       |   |-- bin_to_hex.h
|   |   |       |   |-- chrono.h
|   |   |       |   |-- compile.h
|   |   |       |   |-- fmt.h
|   |   |       |   |-- ostr.h
|   |   |       |   |-- ranges.h
|   |   |       |   |-- std.h
|   |   |       |   -- xchar.h
|   |   |       |-- sinks
|   |   |       |   |-- android_sink.h
|   |   |       |   |-- ansicolor_sink.h
|   |   |       |   |-- ansicolor_sink-inl.h
|   |   |       |   |-- base_sink.h
|   |   |       |   |-- base_sink-inl.h
|   |   |       |   |-- basic_file_sink.h
|   |   |       |   |-- basic_file_sink-inl.h
|   |   |       |   |-- callback_sink.h
|   |   |       |   |-- daily_file_sink.h
|   |   |       |   |-- dist_sink.h
|   |   |       |   |-- dup_filter_sink.h
|   |   |       |   |-- hourly_file_sink.h
|   |   |       |   |-- kafka_sink.h
|   |   |       |   |-- loki_sink.h
|   |   |       |   |-- mongo_sink.h
|   |   |       |   |-- msvc_sink.h
|   |   |       |   |-- null_sink.h
|   |   |       |   |-- ostream_sink.h
|   |   |       |   |-- qt_sinks.h
|   |   |       |   |-- ringbuffer_sink.h
|   |   |       |   |-- rotating_file_sink.h
|   |   |       |   |-- rotating_file_sink-inl.h
|   |   |       |   |-- sink.h
|   |   |       |   |-- sink-inl.h
|   |   |       |   |-- stdout_color_sinks.h
|   |   |       |   |-- stdout_color_sinks-inl.h
|   |   |       |   |-- stdout_sinks.h
|   |   |       |   |-- stdout_sinks-inl.h
|   |   |       |   |-- syslog_sink.h
|   |   |       |   |-- systemd_namespace_sink.h
|   |   |       |   |-- systemd_sink.h
|   |   |       |   |-- tcp_sink.h
|   |   |       |   |-- udp_sink.h
|   |   |       |   |-- win_eventlog_sink.h
|   |   |       |   |-- wincolor_sink.h
|   |   |       |   -- wincolor_sink-inl.h
|   |   |       |-- async.h
|   |   |       |-- async_logger.h
|   |   |       |-- async_logger-inl.h
|   |   |       |-- common.h
|   |   |       |-- common-inl.h
|   |   |       |-- formatter.h
|   |   |       |-- fwd.h
|   |   |       |-- logger.h
|   |   |       |-- logger-inl.h
|   |   |       |-- mdc.h
|   |   |       |-- namespace.h
|   |   |       |-- pattern_formatter.h
|   |   |       |-- pattern_formatter-inl.h
|   |   |       |-- spdlog.h
|   |   |       |-- spdlog-inl.h
|   |   |       |-- stopwatch.h
|   |   |       |-- tweakme.h
|   |   |       -- version.h
|   |   |-- logos
|   |   |   |-- jetbrains-variant-4.svg
|   |   |   -- spdlog.png
|   |   |-- scripts
|   |   |   |-- ci_setup_clang.sh
|   |   |   |-- extract_version.py
|   |   |   -- format.sh
|   |   |-- src
|   |   |   |-- async.cpp
|   |   |   |-- bundled_fmtlib_format.cpp
|   |   |   |-- cfg.cpp
|   |   |   |-- color_sinks.cpp
|   |   |   |-- file_sinks.cpp
|   |   |   |-- spdlog.cpp
|   |   |   -- stdout_sinks.cpp
|   |   |-- tests
|   |   |   |-- CMakeLists.txt
|   |   |   |-- includes.h
|   |   |   |-- main.cpp
|   |   |   |-- test_async.cpp
|   |   |   |-- test_backtrace.cpp
|   |   |   |-- test_bin_to_hex.cpp
|   |   |   |-- test_cfg.cpp
|   |   |   |-- test_circular_q.cpp
|   |   |   |-- test_create_dir.cpp
|   |   |   |-- test_custom_callbacks.cpp
|   |   |   |-- test_daily_logger.cpp
|   |   |   |-- test_dup_filter.cpp
|   |   |   |-- test_errors.cpp
|   |   |   |-- test_eventlog.cpp
|   |   |   |-- test_file_helper.cpp
|   |   |   |-- test_file_logging.cpp
|   |   |   |-- test_fmt_helper.cpp
|   |   |   |-- test_macros.cpp
|   |   |   |-- test_misc.cpp
|   |   |   |-- test_mpmc_q.cpp
|   |   |   |-- test_pattern_formatter.cpp
|   |   |   |-- test_registry.cpp
|   |   |   |-- test_ringbuffer.cpp
|   |   |   |-- test_sink.h
|   |   |   |-- test_stdout_api.cpp
|   |   |   |-- test_stopwatch.cpp
|   |   |   |-- test_systemd.cpp
|   |   |   |-- test_systemd_namespace.cpp
|   |   |   |-- test_time_point.cpp
|   |   |   |-- test_timezone.cpp
|   |   |   |-- utils.cpp
|   |   |   -- utils.h
|   |   |-- .clang-format
|   |   |-- .clang-tidy
|   |   |-- .gitattributes
|   |   |-- .git-blame-ignore-revs
|   |   |-- .gitignore
|   |   |-- appveyor.yml
|   |   |-- CMakeLists.txt
|   |   |-- INSTALL
|   |   |-- LICENSE
|   |   -- README.md
|   -- CMakeLists.txt
|-- LICENSES
|   |-- Apache-2.0.txt
|   |-- BSD-3-Clause.txt
|   -- MIT.txt
|-- src
|   |-- base
|   |   |-- base_math.h
|   |   |-- base_types.h
|   |   |-- CMakeLists.txt
|   |   |-- color_light.cpp
|   |   |-- gaussRand.h
|   |   |-- geometry.cpp
|   |   |-- image_process.cpp
|   |   |-- interpolation.cpp
|   |   |-- log.cpp
|   |   |-- log.h
|   |   |-- random.cpp
|   |   -- stroke.cpp
|   |-- engine
|   |   |-- CMakeLists.txt
|   |   |-- engine_init.cpp
|   |   |-- engine_init.h
|   |   |-- engine_state.cpp
|   |   |-- engine_state.h
|   |   |-- render_loop.cpp
|   |   |-- render_loop.h
|   |   |-- tool_switcher.cpp
|   |   |-- tool_switcher.h
|   |   |-- window_proc.cpp
|   |   -- window_proc.h
|   |-- render
|   |   |-- blend_mode.cpp
|   |   |-- blend_mode.h
|   |   |-- brush_renderer.cpp
|   |   |-- brush_renderer.h
|   |   |-- canvas.cpp
|   |   |-- canvas.h
|   |   |-- CMakeLists.txt
|   |   |-- gpu_brush_renderer.cpp
|   |   |-- gpu_brush_renderer.h
|   |   |-- gpu_canvas.cpp
|   |   |-- gpu_canvas.h
|   |   |-- layer.cpp
|   |   |-- layer.h
|   |   |-- render_target.cpp
|   |   |-- render_target.h
|   |   |-- texture.cpp
|   |   |-- texture.h
|   |   |-- ui_panel.cpp
|   |   -- ui_panel.h
|   |-- sys
|   |   |-- brush.cpp
|   |   |-- brush.h
|   |   |-- brush_circle.cpp
|   |   |-- brush_circle.h
|   |   |-- brush_marker.cpp
|   |   |-- brush_marker.h
|   |   |-- brush_pencil.cpp
|   |   |-- brush_pencil.h
|   |   |-- brush_watercolor.cpp
|   |   |-- brush_watercolor.h
|   |   |-- CMakeLists.txt
|   |   |-- input_sample.cpp
|   |   |-- input_sample.h
|   |   |-- shortcut.cpp
|   |   |-- shortcut.h
|   |   |-- tool_manager.cpp
|   |   |-- tool_manager.h
|   |   |-- tool_type.h
|   |   |-- ui_manager.cpp
|   |   -- ui_manager.h
|   |-- tool
|   |   |-- CMakeLists.txt
|   |   |-- tool_base.cpp
|   |   |-- tool_base.h
|   |   |-- tool_crop.cpp
|   |   |-- tool_crop.h
|   |   |-- tool_ellipse_select.cpp
|   |   |-- tool_ellipse_select.h
|   |   |-- tool_eraser.cpp
|   |   |-- tool_eraser.h
|   |   |-- tool_eyedropper.cpp
|   |   |-- tool_eyedropper.h
|   |   |-- tool_lasso.cpp
|   |   |-- tool_lasso.h
|   |   |-- tool_magic_wand.cpp
|   |   |-- tool_magic_wand.h
|   |   |-- tool_move.cpp
|   |   |-- tool_move.h
|   |   |-- tool_pen.cpp
|   |   |-- tool_pen.h
|   |   |-- tool_poly_lasso.cpp
|   |   |-- tool_poly_lasso.h
|   |   |-- tool_rect_select.cpp
|   |   |-- tool_rect_select.h
|   |   |-- tool_rotate_canvas.cpp
|   |   |-- tool_rotate_canvas.h
|   |   |-- tool_text.cpp
|   |   |-- tool_text.h
|   |   |-- tool_zoom.cpp
|   |   -- tool_zoom.h
|   |-- vulkan
|   |   |-- brush_dab.frag
|   |   |-- brush_dab.vert
|   |   |-- CMakeLists.txt
|   |   |-- composite.frag
|   |   |-- composite.vert
|   |   |-- vk_brush_pipeline.cpp
|   |   |-- vk_brush_pipeline.h
|   |   |-- vk_buffer.cpp
|   |   |-- vk_buffer.h
|   |   |-- vk_canvas_tex.cpp
|   |   |-- vk_canvas_tex.h
|   |   |-- vk_commands.cpp
|   |   |-- vk_commands.h
|   |   |-- vk_composite_pipeline.cpp
|   |   |-- vk_composite_pipeline.h
|   |   |-- vk_core.cpp
|   |   |-- vk_core.h
|   |   |-- vk_device.cpp
|   |   |-- vk_device.h
|   |   |-- vk_imageview.cpp
|   |   |-- vk_imageview.h
|   |   |-- vk_instance.cpp
|   |   |-- vk_instance.h
|   |   |-- vk_pipeline.cpp
|   |   |-- vk_pipeline.h
|   |   |-- vk_renderpass.cpp
|   |   |-- vk_renderpass.h
|   |   |-- vk_surface.cpp
|   |   |-- vk_surface.h
|   |   |-- vk_swapchain.cpp
|   |   |-- vk_swapchain.h
|   |   |-- vk_sync.cpp
|   |   |-- vk_sync.h
|   |   |-- vk_utils.cpp
|   |   -- vk_utils.h
|   |-- cmake-modules.json
|   |-- main.cpp
|   -- Update-CMakeModules.ps1
|-- .gitignore
|-- .gitmodules
|-- build.bat
|-- CMakeLists.txt
-- README.md
```

语言：C++ (100%)

文件数：117

有效代码行：9,471
