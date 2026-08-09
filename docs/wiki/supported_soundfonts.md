# Supported SoundFont formats

Little Piggy Tracker loads **SoundFont 2.0 banks** (`.sf2`) through the
E-mu SoundFont parser (`sources/Externals/Soundfont/`). Each **preset** of
a bank becomes one instrument, named after the preset — not the file.

## What works

| Format                     | RIFF form ID | Result                              |
|----------------------------|--------------|-------------------------------------|
| SoundFont 2.x bank (`.sf2`) | `SFBK`       | Full support (presets become instruments) |

## What is rejected

| Format                        | Why                                                     |
|-------------------------------|---------------------------------------------------------|
| `.sf3` (SF2 with Ogg samples) | Samples are compressed — the parser does not decode Ogg |
| `.dls`                        | Downloadable Sounds form, not `SFBK`                    |
| `.gig` / `.sfz` / `.sfark`    | Different container formats, not `SFBK`                 |

The sample-library script (`tools/fix_sample_library.sh`) preserves all of
these extensions, but the tracker only loads `.sf2`.

## Importing a sound font

### Via the import dialog

1. Open the **Import sample** dialog (*File → Import Sample*).
2. `.sf2` files are listed alongside `.wav` and `.mp3`.
3. Select the bank — each preset is added to the instrument bank.

### Manually (the classic way)

1. Copy the `.sf2` file into the project's `samples/` folder
   (the same folder where the `.wav`/`.mp3` files live).
2. Open the project — `SamplePool::Load()` scans `*.sf2` on startup
   and registers every preset of every bank.

## Common gotchas

- **Instruments are named after presets, not files.** A bank called
  `piano.sf2` with presets `Grand Piano`, `Honky Tonk` and `Harpsichord`
  shows up as three separate instruments with those names.
- **The whole bank is loaded into RAM at import time** (every sample is
  read and decompressed on load), so a large bank costs memory
  proportional to its samples. Keep banks lean for embedded platforms.
- **There is no streaming** for sound fonts — unlike WAV/MP3 sources,
  samples are resident in memory for the lifetime of the bank.
- Invalid or truncated files are rejected with
  `Not a valid SF2 sound font` and do not crash the tracker.

## Where the parsing lives

- `SoundFontManager::LoadBank()` — loads the bank file and reads all
  samples into memory (`sources/Application/Instruments/SoundFontManager.cpp`).
- `SamplePool::loadSoundFont()` — creates one `SoundFontPreset` per preset
  (`sources/Application/Instruments/SamplePool.cpp`).
- `sfReadSFBFile()` — the E-mu SF2 parser
  (`sources/Externals/Soundfont/ENAB.CPP`).
