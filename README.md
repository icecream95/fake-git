# fake-git

A fake git server for reproducing hangs on Snapdragon X Elite SoCs.

## Usage

```
sync; ./git-server.py & sleep 1; while :; do git clone http://localhost:8000/crash-me-please.git; done
```

Each clone should fail successfully with code 418.

If you are running this on an affected chip, and without mitigations,
then soon (less than ten or so clone attempts) the system should
freeze, and then after a few seconds automatically reboot. It seems
that sometimes the system stays up for a couple of seconds after the
core running `git` deadlocks.

The crash is more likely to happen on an idle system, especially if a
desktop environment is not running.

## Notes

A mitigation is to remove `SCTLR_EL1_DZE` from `INIT_SCTLR_EL1_MMU_ON`
in `arch/arm64/include/asm/sysreg.h`, in the Linux kernel source code,
in order to disable the `dc zva` instruction. This might also have to
be done for VMs. Deleting the `dc zva` instructions from
`arch/arm64/lib/memset.S` and `arch/arm64/lib/clear_page.S`, so that
they cannot be speculatively executed, may also help.

Windows uses a different mitigation, which is currently unknown.

It is currently unknown whether `dc zva` is the only instruction that
can cause this reset.

The list of tags and branches comes from
https://chromium.googlesource.com/chromiumos/third_party/kernel
