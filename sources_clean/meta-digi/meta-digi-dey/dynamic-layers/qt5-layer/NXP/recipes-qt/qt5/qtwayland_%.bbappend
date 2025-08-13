PACKAGECONFIG:remove:mx6sl-nxp-bsp = "wayland-egl"
PACKAGECONFIG:remove:mx8mm-nxp-bsp = "wayland-vulkan-server-buffer"
PACKAGECONFIG:remove:mx9-nxp-bsp = "xcomposite-egl xcomposite-glx"

do_install:append() {
if ls ${D}${libdir}/pkgconfig/Qt5*.pc >/dev/null 2>&1; then
    sed -i 's,-L${STAGING_DIR_HOST}/usr/lib,,' ${D}${libdir}/pkgconfig/Qt5*.pc
fi
}
