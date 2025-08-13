# Copyright (C) 2015 Freescale Semiconductor
# Copyright 2017-2020,2022,2024 NXP
# Released under the MIT license (see COPYING.MIT for the terms)

SUMMARY = "Set of audio tools for inclusion on images"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

PACKAGE_ARCH = "${TUNE_PKGARCH}"

inherit packagegroup

ALSA_INSTALL = " \
    alsa-utils \
    alsa-tools \
"
ALSA_INSTALL:append:mx8-nxp-bsp = " tinycompress"

PULSEAUDIO_INSTALL = " \
    pulseaudio-server \
    pulseaudio-module-cli \
    pulseaudio-misc \
    pulseaudio-module-device-manager \
    ${@bb.utils.contains('DISTRO_FEATURES',"x11", "pulseaudio-module-x11-xsmp \
                                                   pulseaudio-module-x11-publish \
                                                   pulseaudio-module-x11-cork-request \
                                                   pulseaudio-module-x11-bell \
                                                   consolekit", \
                                           "", d)} \
"

# for synchronous and asynchronous sample rate converter
SRC_INSTALL ?= ""
SRC_INSTALL:mx8-nxp-bsp = "imx-dspc-asrc"
SRC_INSTALL:mx9-nxp-bsp = "nxp-ssrc"

PDM_INSTALL ?= ""
PDM_INSTALL:mx8m-nxp-bsp = "imx-sw-pdm"

NXPAFE_INSTALL ?= ""
NXPAFE_INSTALL:mx8mm-nxp-bsp = "nxp-afe nxp-afe-voiceseeker"
NXPAFE_INSTALL:mx8mp-nxp-bsp = "nxp-afe nxp-afe-voiceseeker"
NXPAFE_INSTALL:mx8ulp-nxp-bsp = "nxp-afe nxp-afe-voiceseeker"
NXPAFE_INSTALL:mx9-nxp-bsp = "nxp-afe nxp-afe-voiceseeker"

SOFTOOLS_INSTALL       ?= ""
SOFTOOLS_INSTALL:mx8qm-nxp-bsp  = "sof-tools"
SOFTOOLS_INSTALL:mx8qxp-nxp-bsp = "sof-tools"
SOFTOOLS_INSTALL:mx8dx-nxp-bsp  = "sof-tools"
SOFTOOLS_INSTALL:mx8mp-nxp-bsp  = "sof-tools"
SOFTOOLS_INSTALL:mx8ulp-nxp-bsp = "sof-tools"

RDEPENDS:${PN} = " \
    ${@bb.utils.contains("DISTRO_FEATURES", "alsa",  "${ALSA_INSTALL}", "", d)} \
    ${@bb.utils.contains("DISTRO_FEATURES", "pulseaudio",  "${PULSEAUDIO_INSTALL}", "", d)} \
    ${SRC_INSTALL} \
    ${PDM_INSTALL} \
    ${NXPAFE_INSTALL} \
    ${SOFTOOLS_INSTALL} \
"
