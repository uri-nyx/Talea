# TPSTOOLS

A simple package of tools to manage disk images for the Taleä system.

## Tools

- tps_wrap: a python script/library that allows you to wrap a disk image with the tps header needed by the emulator.
- hcs_wrap: a python script/library that allows you to wrap a disk image with the hcs header needed by the emulator.
- tfat: a bash script that implements a extfs for Midnight Commander (mc). It allows you to browse, copy, extract and edit files inside the disk images. To use it, copy it to `~/.local/share/mc/extfs.d/`, and add this to `~/.config/mc/mc.ext.ini`:

```ini
[Talea]
Regex=\.(tps|tpc|hcs)$
RegexIgnoreCase=true
Open=%cd %p/tfat://
View=%view{ascii} head -c 512 %f | xxd
```

> Note: the View option currently only shows an hexdump of the header
