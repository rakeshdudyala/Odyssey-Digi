# Copyright (C) 2013-2022, Digi International Inc.

SUMMARY = "DEY examples: OpenGL-ES test application (based on Freescale GPU SDK)"
SECTION = "examples"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6"

DEPENDS = "virtual/egl virtual/libgles1 virtual/libgles2"

SRC_URI = "file://opengles"

S = "${WORKDIR}/opengles"

EXTRA_OEMAKE = "EGL_FLAVOUR=${@oe.utils.conditional('HAVE_GUI', '1' , 'x11', 'fb', d)}"

do_install () {
	oe_runmake DEST_DIR="${D}" install
}

FILES:${PN} = "/opt/${PN}"
FILES:${PN}-dbg += "/opt/${PN}/.debug"

RDEPENDS:${PN}:ccimx6 = "libopenvg-imx"

PACKAGE_ARCH = "${MACHINE_ARCH}"

COMPATIBLE_MACHINE = "(ccimx6$)"
