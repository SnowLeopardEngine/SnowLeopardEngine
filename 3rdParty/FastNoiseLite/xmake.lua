-- define target
target("FastNoiseLite")
    set_kind("headeronly")
    add_headerfiles("*.h")
    add_rules("utils.install.cmake_importfiles")
    add_rules("utils.install.pkgconfig_importfiles")
    add_includedirs(".", {public = true}) -- public: let other targets to auto include