# Copyright (C) 2015-2021 Stefano Babic <sbabic@denx.de>
#
# SPDX-License-Identifier: GPLv3
#
# This class is thought to be used in an image recipe.
# It generates a SWU file from the image itself
# User *must* add a sw-descitpion file
#
# To use this class, add the inherit clause of the update image bb file.
# The generated output file is an swu archive ready to be uploaded to a device running
# swupdate.

inherit swupdate-common
inherit image-artifact-names

S = "${WORKDIR}/${PN}"

SRC_URI += "file://sw-description"
SWUPDATE_IMAGES += "${IMAGE_BASENAME}"

python () {
    image = d.getVar('IMAGE_BASENAME')
    t = d.getVarFlag("SWUPDATE_IMAGES_FSTYPES", image)
    suffix = d.getVar("IMAGE_NAME_SUFFIX") or ""
    if t is None:
       flag = d.getVarFlag("SWUPDATE_IMAGES_FSTYPES", d.getVar('IMAGE_BASENAME'))
       if flag:
          d.setVarFlag("SWUPDATE_IMAGES_FSTYPES", image, flag)
       else:
          fstypes = d.getVar('IMAGE_FSTYPES').split()
          if not fstypes:
              bb.fatal("SWUPDATE_IMAGES_FSTYPES[%s] is not set !" % image)
          for t in fstypes:
              bb.warn("SWUPDATE_IMAGES_FSTYPES[%s] not set, setting to %s" % (image, t))
              d.setVarFlag("SWUPDATE_IMAGES_FSTYPES", image, suffix + "." + t)
              break
    else:
       d.setVarFlag("SWUPDATE_IMAGES_FSTYPES", image, suffix + t)
}

python do_swupdate_copy_swdescription() {

    import shutil

    workdir = d.getVar('S')
    filespath = d.getVar('FILESPATH')
    sw_desc_path = bb.utils.which(filespath, "sw-description")
    shutil.copyfile(sw_desc_path, os.path.join(workdir, "sw-description"))
}

addtask swupdate_copy_swdescription before do_image_complete after do_unpack
addtask swuimage after do_swupdate_copy_swdescription do_image_complete before do_build

# define setscene task
python do_swuimage_setscene () {
    sstate_setscene(d)
}
addtask do_swuimage_setscene

# Read all variables from sw-description file and add them to the vardeps of the do_swuimage task. Bitbake
# cannot know that the do_swuimage task which evaluates the templated sw-description file needs to be executed
# if a variable which is refered by the sw-description file but not by the recipe itself.
do_swuimage[vardeps] ?= "${@swupdate_find_bitbake_variables(d)}"
