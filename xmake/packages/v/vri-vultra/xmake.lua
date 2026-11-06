-- VRI as libvultra builds it: VRI 0.2.0 from merged commit d0c78964 plus the desktop patches below
-- (copied from libvultra's external/vri, MIT, see LICENSE). Consumers use it as `vri`.
--   calibrated_timestamps: with several Vulkan ICDs (e.g. Intel + NVIDIA) device creation looked up the
--                          time-domain query through another ICD's trampoline and crashed
--   validation_extent:     the validation layer did not forward GetSwapChainExtent (null in Debug)
--   cube_array:            enable the queried imageCubeArray feature
--   texture_copy_region:   texture copies honour offsets, extents, mips and layers
-- Kept in-tree until it is published to SnowLeopardEngine/xmake-repo.
package("vri-vultra")
    set_base("vri")
    set_urls("https://github.com/zzxzzk115/VRI/archive/d0c78964cc50788eb49d4b7daf967ef3222d2530.tar.gz")
    add_versions("v0.2.0", "b236bd95ad4a9f4f04ffbbf61cbff02934cdcfeac740963e8dd8f2c1025a62ad")
    add_patches("v0.2.0", path.join(os.scriptdir(), "validation_extent.patch"),
                "defa9f682a859c8d443bd59861adb0a78a7e0d43d183b7b5d28491ecf039e584")
    add_patches("v0.2.0", path.join(os.scriptdir(), "cube_array.patch"),
                "1feddcfbeedccc258857d3144524d85d444b96f2c1e26cd2ecfe70ea4d56e7f9")
    add_patches("v0.2.0", path.join(os.scriptdir(), "texture_copy_region.patch"),
                "3978dea6962f488ac162b44e23b8b5a99f766d7e3c69b8502a994e661b1a8b94")
    add_patches("v0.2.0", path.join(os.scriptdir(), "calibrated_timestamps.patch"),
                "d2f06cd8c0157944c0088e83f0bcd52c2fb7ff3d4a874be393454a216bf9da11")
package_end()
