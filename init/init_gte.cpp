/*
   Copyright (c) 2017, The Linux Foundation. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
	* Redistributions of source code must retain the above copyright
	  notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above
	  copyright notice, this list of conditions and the following
	  disclaimer in the documentation and/or other materials provided
	  with the distribution.
	* Neither the name of The Linux Foundation nor the names of its
	  contributors may be used to endorse or promote products derived
	  from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <android-base/file.h>
#include <android-base/strings.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#define SIMSLOT_FILE "/proc/simslot_count"

#include <android-base/properties.h>
#include <android-base/logging.h>

#include "vendor_init.h"
#include "property_service.h"

#define SERIAL_NUMBER_FILE "/efs/FactoryApp/serial_no"

using android::base::GetProperty;
using android::base::ReadFileToString;
using android::base::Trim;
using android::init::property_set;

void property_override(char const prop[], char const value[])
{
    prop_info *pi;

    pi = (prop_info*) __system_property_find(prop);
    if (pi)
        __system_property_update(pi, value, strlen(value));
    else
        __system_property_add(prop, strlen(prop), value, strlen(value));
}

void property_override_dual(char const system_prop[], char const vendor_prop[], char const value[])
{
    property_override(system_prop, value);
    property_override(vendor_prop, value);
}

/* Read the file at filename and returns the integer
 * value in the file.
 *
 * @prereq: Assumes that integer is non-negative.
 *
 * @return: integer value read if succesful, -1 otherwise. */
int read_integer(const char* filename)
{
	int retval;
	FILE * file;

	/* open the file */
	if (!(file = fopen(filename, "r"))) {
		return -1;
	}
	/* read the value from the file */
	fscanf(file, "%d", &retval);
	fclose(file);

	return retval;
}

void set_fingerprint()
{
	std::string fingerprint = GetProperty("ro.build.fingerprint", "");

	if ((strlen(fingerprint.c_str()) > 1) && (strlen(fingerprint.c_str()) <= PROP_VALUE_MAX))
		return;

	char new_fingerprint[PROP_VALUE_MAX+1];

	std::string build_id = GetProperty("ro.build.id","");
	std::string build_tags = GetProperty("ro.build.tags","");
	std::string build_type = GetProperty("ro.build.type","");
	std::string device = GetProperty("ro.product.device","");
	std::string incremental_version = GetProperty("ro.build.version.incremental","");
	std::string release_version = GetProperty("ro.build.version.release","");

	snprintf(new_fingerprint, PROP_VALUE_MAX, "samsung/%s/%s:%s/%s/%s:%s/%s",
		device.c_str(), device.c_str(), release_version.c_str(), build_id.c_str(),
		incremental_version.c_str(), build_type.c_str(), build_tags.c_str());

	property_override_dual("ro.build.fingerprint", "ro.boot.fingerprint", new_fingerprint);
}

void set_cdma_properties()
{
	android::init::property_set("ro.cdma.home.operator.alpha", "Chameleon");
	android::init::property_set("ro.cdma.home.operator.numeric", "310000");
	android::init::property_set("ro.telephony.default_network", "10");
}

void set_gsm_properties()
{
	android::init::property_set("telephony.lteOnCdmaDevice", "0");
	android::init::property_set("ro.telephony.default_network", "9");
}

void set_wifi_properties()
{
    android::init::property_set("ro.carrier", "wifi-only");
    android::init::property_set("ro.radio.noril", "1");
}

void set_target_properties(const char *device, const char *model)
{
	property_override_dual("ro.product.device", "ro.product.vendor.device", device);
	property_override_dual("ro.product.model", "ro.product.vendor.model", model);

	/* check and/or set fingerprint */
	set_fingerprint();

	char const *serial_number_file = SERIAL_NUMBER_FILE;
	std::string serial_number;

	if (ReadFileToString(serial_number_file, &serial_number)) {
        	serial_number = Trim(serial_number);
        	property_override("ro.serialno", serial_number.c_str());
	}
}

void vendor_load_properties()
{
	char *device = NULL;
	char *model = NULL;

	/* get the bootloader string */
	std::string bootloader = android::base::GetProperty("ro.bootloader", "");

	if (bootloader.find("T377P") == 0) {
		device = (char *)"gtesqltespr";
		model = (char *)"SM-T377P";
		set_cdma_properties();
	}
	else if (bootloader.find("T560NUU") == 0) {
		device = (char *)"gtelwifiue";
		model = (char *)"SM-T560NU";
		set_wifi_properties();
	}
	else if (bootloader.find("T550") == 0) {
		device = (char *)"gt510wifi";
		model = (char *)"SM-T550";
		set_wifi_properties();
	}
	else if (bootloader.find("T350") == 0) {
		device = (char *)"gt58wifi";
		model = (char *)"SM-T350";
		set_wifi_properties();
	}
	else if (bootloader.find("T357T") == 0) {
		device = (char *)"gt58ltetmo";
		model = (char *)"SM-T357T";
		set_gsm_properties();
	}
	else {
		return;
	}

	/* set the properties */
	set_target_properties(device, model);
}