# Type D — contribution notes

Notes toward folding **Type D** support into the single A/B/C/D codebase. This is reverse-engineering
+ patch documentation, independently derived and largely confirmed on Type D hardware. Where a fix 
lands in the hand-maintained base `.rbn` (whose stub map isn't in the repo), it's written as a 
description to act on rather than a binary diff.

Reel type is read from `0x80340000`; **Type D == 4** (matches `res.c`/`hist.c`/`manwb.c`).

---

## 0. Base equivalence — addresses port 1:1

The committed `fw/FWDV280-D.rbn` is **byte-identical** to videodoctor's decompressed D base except
**2 bytes** in the container header (offsets 111–112). So the D memory map is shared, and every file
offset / VMA below applies unchanged to `fw/FWDV280-D.rbn`.

---

## 1. Boot freeze on D — root cause + proposed fix

**Symptom:** the committed D base freezes at boot when entering the main menu — motor init hangs (no
motor nudge on frozen boots). Reproduced by building the repo as-is for D (crop+hist+manwb spliced
into `fw/FWDV280-D.rbn`, packed, flashed).

**Confirmed it's not the repo modules:** adding `if(*reelType==4) return;` to the top of all three
(`crop_mod` / `calc_histogram` / `select_wb`) and re-flashing still freezes → the culprit is in the
source-less arena code, not `crop`/`hist`/`manwb`.

**Root cause:** entry/inline `jal` hooks in the `0x2b…/0x2c…` motor/preview region stall motor
timing on D (the same "entry hooks break motor timing" effect videodoctor reported — hooks must fire
*inside* functions, not at motor-critical entry points). The boot-critical hooks call **arena helper
routines whose source is not in the repo** (only `crop`/`hist`/`manwb`/`res` are). Prime suspects
(clean-base `jal` targets into the arena):

| Hook site (VMA) | Calls arena routine | Note |
|---|---|---|
| `0x802b6a44` | `0x8033a690` | AE/motor cluster (`0x2b6a3c` region) — strongest suspect |
| `0x8013b5f8` | `0x80339420` | early |
| `0x801b9354` | `0x80339890` | |
| `0x8023fdf0` | `0x80339580` | |
| `0x8026e010` | `0x8033a560` | |
| `0x802eadf4` / `0x802eb090` | `0x803392c0` / `0x80339180` | |

**Proposed fix:** guard the boot-critical arena routine(s) with the same pattern already used in 
`hist.c`/`manwb.c`:

```c
volatile int *reelType = (int *)0x80340000;
if (*reelType == 4) { /* skip */ }
```

i.e. make the motor-region hook a no-op (or a D-specific safe path) on D while A/B/C keep their
current behavior. Because the `jal` sites are shared across all reel types, the branch must live
*inside* the called routine — which is why this needs the arena source rather than a base byte-patch.

---

## 2. WB `select_wb` caller-save clobber (stability fix)

The WB hook `jal select_wb` (call site `0x802b7c98`, target `0x8033e000`) does not reload the
caller-saved registers `a1`/`a2` after the call, so WB gains get written from clobbered registers →
garbage/corruption. Fix = restore `a1`/`a2` from their source after the `jal` — a **3-instruction
reload placed in the slots at `0x2b7cc0` / `0x2b7cc8` / `0x2b7ccc`** (identified - verify on-device). 
This is a base hook-site fix, not a change to `manwb.c`'s body.

---

## 3. Quality patches (bitrate / denoise / grain)

Three single-word edits on the **decompressed** D image. 

| What | File offset | Original word | Patched word | LE bytes (orig → new) | Effect |
|---|---|---|---|---|---|
| **Bitrate ×4** | `0x240588` | `0x000318c0` (`sll v1,v1,3`) | `0x00031940` (`sll v1,v1,5`) | `c0 18 03 00` → `40 19 03 00` | ~8 → ~31 Mbps. **×2** = `0x00031900` / `00 19 03 00` (~16 Mbps) |
| **3D-NR force ON** | `0x2b9d0c` | `0x1202001b` (`beq s0,v0,..`) | `0x1000001b` (`b ..`) | `1b 00 02 12` → `1b 00 00 10` | temporal denoise always on (subtle) |
| **FilmGrain OFF** | `0x2f1c60` | `0x8c840000` (`lw a0,0(a0)`) | `0x00002021` (`move a0,zero`) | `00 00 84 8c` → `21 20 00 00` | disables synthetic grain enable (no visible change here; harmless) |

Mechanism: the record bitrate is a **soft target**, not a hard buffer cap. Active bits/sec =
`config[24] << 3` at `0x80240588`; the rate-control limits (`RC+156 ≈ 0.8×`, `RC+148 ≈ 6×`) scale
with the target, so raising the target just makes the encoder pick a **lower QP** rather than freeze.
Tested 8 → 15.7 → 31.9 Mbps, all stable.

> Heads-up: the QP config bytes at `0x8026d624/628` (min/init/max, default 30/32/32)
> are a **dead lever** on this build — they belong to a different encoder instance and patching them
> does nothing. The live QP is rate-control-driven, so the bitrate target above is the real lever.

---

## 4. OSD potential corrections

Once D boots, the existing `hist.c` OSD should render correctly.

**Frame counter.** `hist.c` reads the scratch slot `enc_frames @ 0x85bf0014` for its `Frm:` line, but
this was **never verified on D** — the D build freezes at boot before `hist.c` ever runs, so whether
that slot gets populated on D is untested. The counter confirmed working on D (via the videodoctor
base, which boots) is `*(*(u32*)0x80e974dc + 0xD4)` (encoder ctx; resets to 0 at record start, +1 per
encoded frame). Verify how `enc_frames` behaves once D boots.

**ISO vs. Gain (worth distinguishing on the HUD).** These are two different layers:
- **Sensor ISO** (the setting `hist.c`'s OSD prints as `ISO`): on D = `0x80e5590c` (`sensor ISO`),
  exposure time at `+1`. This is the AE sensitivity target.
- **Analog gain** (what our HUD showed as `Gain`): `0x80f82598` (×100, unity = 100). In our tests this
  read **≈ unity**. *Plausible implication:* if gain really sits near unity during capture, the visible  
  grain is more likely sensor read-noise + inherent 8mm film grain than gain/ISO amplification noise.

Other live HUD values confirmed on D: exposure `0x80f82594`, WB gains `0x80F9075C/60/64`, resolution
`0x80ddc718/71c`, QP cur/floor `*(*(0x80e974dc)+0x94)` / `+0x98`.

---

## 5. Geometry (`res` / `crop`) on D — needs a confirming pass

On D, resolution appears to be driven by the static registers `0x80ddc718` (width) / `0x80ddc71c` 
(height) rather than the `(width−440)*4` A–C math (which `crop.c` already gates with `if(*reelType<4)`
and leaves as a `// not for D` TODO). `crop_mod`'s `else` (reelType≥4) branch runs on D but its 
geometry output should be verified on-device.

---

## 6. Build / verify (repo, on this machine)

- Toolchain: WSL `mipsel-linux-gnu-gcc 12.4`; packers `ntkcalcVS.exe -cw`, `bfc4ntkVS.exe -c`.
- `utils/splice.py` re-injects `crop`/`hist`/`manwb` bodies into `fw/FWDV280-D.rbn` (dry-run by
  default; `--write OUT.rbn` to emit). All three fit budget.
- Pack: `ntkcalcVS -cw OUT.rbn` → `bfc4ntkVS -c OUT.rbn OUT.bcl` → `ntkcalcVS -cw OUT.bcl`.
- If custom code is ever needed beyond the arena routines: the D image ends at VMA `0x80e08dac`, and
  there's a zeroed free-space region at file `0x34116c` usable for a placed blob (`.text` + `.rodata`).
  Reel-type fns for reference: `consoleD = 0x80080c60`.
