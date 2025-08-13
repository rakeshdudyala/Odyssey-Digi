#!/bin/sh
#===============================================================================
#
#  hdmi_hotplug.sh
#
#  Copyright (C) 2017 by Digi International Inc.
#  All rights reserved.
#
#  This program is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License version 2 as published by
#  the Free Software Foundation.
#
#
#  !Description: HDMI hotplug script called from udev on HDMI plug/unplug event
#
#===============================================================================

# Find HDMI card number
for card in /sys/class/sound/card*; do
	if readlink "${card}/device" | grep -qs hdmi; then
		HDMI_CARD="${card##/sys/class/sound/card}"
		break
	fi
done

# On HDMI plugin event, if the sink has not been loaded yet, load the
# HDMI audio sink from ALSA
if [ "${EVENT}" = "plugin" ]; then
	if ! pactl list sinks | grep -qs "imx-hdmi-soc"; then
		card-detect "${HDMI_CARD}"

		# Set HDMI as default sink
		pactl set-default-sink "$(pactl list sinks | grep -i 'Name:.*hdmi' | cut -d ' ' -f2)"
	fi
fi

