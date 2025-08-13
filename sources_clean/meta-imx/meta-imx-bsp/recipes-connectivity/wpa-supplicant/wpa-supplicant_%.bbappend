FILESEXTRAPATHS:prepend:imx-nxp-bsp := "${THISDIR}/${PN}:"

DEPENDS += "readline"

SRC_URI += "file://defconfig"

PACKAGECONFIG ??= "${PACKAGECONFIG_NXP}"
PACKAGECONFIG_NXP             = ""
PACKAGECONFIG_NXP:imx-nxp-bsp = "nxp"

do_configure:prepend() {
    if [ "${@bb.utils.filter('PACKAGECONFIG', 'nxp', d)}" ]; then
        # Overwrite defconfig with NXP Wi-Fi version
        install -m 0755 ${WORKDIR}/defconfig wpa_supplicant/defconfig
    fi
}
