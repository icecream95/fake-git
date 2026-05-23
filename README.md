# fake-git

A fake git server for reproducing hangs on Snapdragon X Elite SoCs.

**Warning:** May cause filesystem corruption, especially with files
that are currently being edited, though doing a sync beforehand makes
this less likely.

## Standalone program (no git—just native C)

```bash
gcc -O2 crash.c -o crash && sync && ./crash
```

This isn't just a minimal reproducer, but tries to act like git in
order to achieve production-grade stability. Coming soon—a more
feather-light version with just the code that's needed.

## Usage (the old way)

```bash
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
in order to disable the `dc zva` instruction.

Windows uses a different mitigation, which is currently unknown.

It is currently unknown whether `dc zva` is the only instruction that
can cause this reset.

The list of tags and branches comes from
https://chromium.googlesource.com/chromiumos/third_party/kernel
