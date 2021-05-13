# On Screen Navigation
PRODUCT_PROPERTY_OVERRIDES += \
	qemu.hw.mainkeys=0

# OpenGLES
PRODUCT_PROPERTY_OVERRIDES += \
    ro.opengles.version=196608

# Radio
PRODUCT_PROPERTY_OVERRIDES += \
	persist.radio.snapshot_enabled=1 \
	persist.radio.snapshot_timer=22 \
	persist.radio.lte_vrte_ltd=1

# Volume
PRODUCT_PROPERTY_OVERRIDES += \
	ro.config.vc_call_vol_steps=15 \
	ro.config.media_vol_steps=30
