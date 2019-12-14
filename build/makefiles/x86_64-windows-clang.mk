CC        = clang
CXX       = clang++

# busybox-w32: https://frippery.org/busybox/
BUSYBOX   = busybox64.exe

MKDIR_P   = $(BUSYBOX) mkdir -p
RM_RF     = $(BUSYBOX) rm -rf
FIND      = $(BUSYBOX) find
