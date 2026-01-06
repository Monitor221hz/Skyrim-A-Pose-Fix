-- =========================================================
-- serde_hkx : Rust FFI static library (self-contained)
--
-- Requirements
-- git, cargo(Rust v1.85.0)
--
-- lib/serdehkx/
-- ├─ include/
-- │  └─ serde_hkx_ffi.h
-- ├─ lib/
-- │  └─ serde_hkx_ffi.lib
-- ├─ serde-hkx <- git clone
---└─ xmake.lua
-- =========================================================

option("serdehkx_build")
    set_default(true)
    set_showmenu(true)
    set_description("Build serde_hkx_ffi via cargo if library does not exist")
option_end()

target("serdehkx")
    set_kind("static")

    -- public include / link
    add_includedirs("include", {public = true})
    add_linkdirs("lib", {public = true})
    add_links("serde_hkx_ffi", {public = true})

    -- auto build rust ffi
    on_load(function (target)
        local base = os.scriptdir()

        local libfile = path.join(base, "lib", "serde_hkx_ffi.lib")
        local incdir  = path.join(base, "include")

        if os.isfile(libfile) and os.isdir(incdir) then
            return
        end

        if not has_config("serdehkx_build") then
            return
        end

        local repo = path.join(base, "serde-hkx")
        if not os.isdir(repo) then
            raise("serde-hkx submodule not found. Did you run git submodule update --init?")
        end

        os.mkdir(path.join(base, "lib"))
        os.mkdir(path.join(base, "include"))

        local old = os.cd(repo)
        print("Building serde_hkx_ffi (cargo)...")
        os.exec("cargo build -p serde_hkx_ffi --release")
        os.cd(old)

        os.cp(
            path.join(repo, "target", "release", "serde_hkx_ffi.lib"),
            libfile
        )

        os.cp(
            path.join(repo, "crates", "serde_hkx_ffi", "test", "serde_hkx_ffi.h"),
            incdir
        )
    end)
