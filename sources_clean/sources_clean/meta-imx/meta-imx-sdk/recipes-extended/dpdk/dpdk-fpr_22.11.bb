DESCRIPTION = "DPDK based Fast Path Routing"
HOMEPAGE = "http://dpdk.org"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=0f00d99239d922ffd13cabef83b33444"

DEPENDS = "dpdk"

SRC_URI = "${DPDK_FPR_SRC};branch=${SRCBRANCH}"
DPDK_FPR_SRC ?= "git://github.com/nxp/dpdk-fpr;protocol=https"

SRCBRANCH = "main"
SRCREV = "0d192200e738dbb82b5f45bccaa49fb6af79ca0d"

S = "${WORKDIR}/git"

inherit pkgconfig

do_install() {
    install -d ${D}${sysconfdir}/dpdk-fpr
    install -d ${D}${bindir}
    install -m 0644 ${S}/nxp/*.txt ${D}${sysconfdir}/dpdk-fpr/
    install -m 0644 ${S}/nxp/fpr*.conf ${D}${sysconfdir}/dpdk-fpr/
    install -m 0755 ${S}/nxp/up ${D}${sysconfdir}/dpdk-fpr/
    install -m 0755 ${S}/app/dpdk-fpr ${D}${bindir}/
}

RDEPENDS:${PN} += " socat kernel-module-dpdk-extras"

COMPATIBLE_MACHINE = "(mx93-nxp-bsp|mx95-nxp-bsp)"
