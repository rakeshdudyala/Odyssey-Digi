# Copyright (C) 2024, 2025, Digi International Inc.
FILESEXTRAPATHS:prepend := "${THISDIR}/${BPN}:"

SRC_URI += "${@oe.utils.vartrue('TRUSTFENCE_FILE_BASED_ENCRYPT', 'file://tee-supplicant', '', d)}"

EXTRA_OEMAKE += "PKG_CONFIG=pkg-config CFG_TEE_FS_PARENT_PATH='${localstatedir}/lib/tee'"

do_install() {
	oe_runmake DESTDIR=${D} install
	install -D -p -m0644 ${WORKDIR}/tee-supplicant.service ${D}${systemd_system_unitdir}/tee-supplicant.service
	sed -i -e s:@sysconfdir@:${sysconfdir}:g \
		-e s:@sbindir@:${sbindir}:g \
		${D}${systemd_system_unitdir}/tee-supplicant.service

	if ${@oe.utils.vartrue('TRUSTFENCE_FILE_BASED_ENCRYPT', 'true', 'false',d)}; then
		install -d ${D}${sysconfdir}/default/
		install -m 0644 ${WORKDIR}/tee-supplicant ${D}${sysconfdir}/default/tee-supplicant
	fi
}
