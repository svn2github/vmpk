#-------------------------------------------------
#
# fluidsynth library
#
#-------------------------------------------------

QT -= core gui
QMAKE_LINK = $$QMAKE_LINK_C
QMAKE_LFLAGS_RPATH =
#QMAKE_CFLAGS_DEBUG += -mfpu=vfpv3
#QMAKE_CFLAGS_RELEASE += -mfpu=vfpv3
#QMAKE_CFLAGS_DEBUG += -mfpu=neon -ffast-math
#QMAKE_CFLAGS_RELEASE += -mfpu=neon -ffast-math -funsafe-math-optimizations

TARGET = fluidsynth
TEMPLATE = lib
VERSION = 1.5.0
CONFIG += link_pkgconfig \
    staticlib \
    stdlib

PKGCONFIG += glib-2.0 \
    gthread-2.0 \
    libpulse-simple

DEFINES += FLUIDSYNTH_DLL_EXPORTS \
    HAVE_CONFIG_H

SOURCES += \
    utils/fluid_conv.c \
    utils/fluid_hash.c \
    utils/fluid_list.c \
    utils/fluid_ringbuffer.c \
    utils/fluid_settings.c \
    utils/fluid_sys.c \
    sfloader/fluid_defsfont.c \
    sfloader/fluid_ramsfont.c \
    rvoice/fluid_adsr_env.c \
    rvoice/fluid_chorus.c \
    rvoice/fluid_iir_filter.c \
    rvoice/fluid_lfo.c \
    rvoice/fluid_rvoice.c \
    rvoice/fluid_rvoice_dsp.c \
    rvoice/fluid_rvoice_event.c \
    rvoice/fluid_rvoice_mixer.c \
    rvoice/fluid_rev.c \
    synth/fluid_chan.c \
    synth/fluid_event.c \
    synth/fluid_gen.c \
    synth/fluid_mod.c \
    synth/fluid_synth.c \
    synth/fluid_tuning.c \
    synth/fluid_voice.c \
    midi/fluid_midi.c \
    midi/fluid_midi_router.c \
    midi/fluid_seqbind.c \
    midi/fluid_seq.c \
    bindings/fluid_cmd.c \
    bindings/fluid_filerenderer.c \
    drivers/fluid_adriver.c \
    drivers/fluid_mdriver.c \
    drivers/fluid_aufile.c \
    drivers/fluid_pulse.c

HEADERS += fluidsynth.h \
    config.h

INCLUDEPATH += bindings \
    drivers \
    midi \
    rvoice \
    sfloader \
    synth \
    utils

#symbian {
#    MMP_RULES += EXPORTUNFROZEN
#    TARGET.UID3 = 0xE312EC73
#    TARGET.CAPABILITY = 
#    TARGET.EPOCALLOWDLLDATA = 1
#    addFiles.sources = fluidsynth.dll
#    addFiles.path = !:/sys/bin
#    DEPLOYMENT += addFiles
#}

#unix:!symbian {
#    maemo5 {
#        target.path = /opt/usr/lib
#    } else {
#        target.path = /usr/lib
#    }
#    INSTALLS += target
#}
