#!/usr/bin/env bash
#
# fix_sample_library.sh — normalize a sample library for LittleGPTracker.
#
# LittleGPTracker's WavFile only decodes WAVE_FORMAT_PCM (code 1) at 16-bit.
# Everything else (float, ADPCM, mu-law, A-law, WAVE_FORMAT_EXTENSIBLE,
# 24/32-bit, 8-bit) is rejected or corrupted on load. This script:
#
#   1. Converts unsupported .wav files to PCM 16-bit WAV.
#   2. Converts .mp3 files to PCM 16-bit WAV.
#   3. NEVER touches sound font files (.sf2, .sf3, .dls, .gig, .sfz) or any
#      other non-audio file.
#
# Detection mirrors WavFile::Open: it reads the wFormatTag and wBitsPerSample
# fields from the "fmt " chunk directly, so it also catches
# WAVE_FORMAT_EXTENSIBLE containers that ffprobe would report as pcm_s16le.
#
# Usage:
#   fix_sample_library.sh [options] [DIR ...]
#
# Options:
#   -n, --dry-run       show what would change, change nothing
#   -b, --backup        keep the original as <file>.bak before overwriting
#   -d, --delete-mp3    remove the source .mp3 after a successful conversion
#   -q, --quiet         only print files that were changed
#   -h, --help          show this help
#
# Default DIR is the current directory. Directories are scanned recursively.
#
# Requires: ffmpeg + ffprobe (or avconv/avprobe).

set -u

DRY_RUN=0
BACKUP=0
DELETE_MP3=0
QUIET=0

FFMPEG=""
FFPROBE=""

die() { printf 'error: %s\n' "$*" >&2; exit 1; }

find_tools() {
    if command -v ffmpeg >/dev/null 2>&1 && command -v ffprobe >/dev/null 2>&1; then
        FFMPEG=ffmpeg
        FFPROBE=ffprobe
        return 0
    fi
    if command -v avconv >/dev/null 2>&1 && command -v avprobe >/dev/null 2>&1; then
        FFMPEG=avconv
        FFPROBE=avprobe
        return 0
    fi
    return 1
}

# Print wFormatTag (2 bytes, little-endian) of a WAV file, or empty if the
# file is not a RIFF/WAVE file or has no "fmt " chunk.
wav_format_tag() {
    local file="$1" tag
    # "RIFF" magic
    if [ "$(od -An -c -N 4 "$file" 2>/dev/null | tr -d ' \n')" != "RIFF" ]; then
        return 1
    fi
    # Locate the "fmt " chunk (4-byte id + 4-byte size precede its payload).
    local pos
    pos=$(grep -abo 'fmt ' "$file" 2>/dev/null | head -1 | cut -d: -f1)
    [ -n "$pos" ] || return 1
    # wFormatTag sits at chunk_payload + 0 (i.e. file offset pos + 8).
    tag=$(od -An -tu2 -j $((pos + 8)) -N 2 "$file" 2>/dev/null | tr -d ' ')
    # Avoid octal parsing: od -tu2 prints decimal, safe to echo.
    printf '%s' "$tag"
}

# Print wBitsPerSample (2 bytes, little-endian) from the same "fmt " chunk.
wav_bits_per_sample() {
    local file="$1"
    local pos
    pos=$(grep -abo 'fmt ' "$file" 2>/dev/null | head -1 | cut -d: -f1)
    [ -n "$pos" ] || return 1
    od -An -tu2 -j $((pos + 8 + 14)) -N 2 "$file" 2>/dev/null | tr -d ' '
}

# 0 = supported (PCM 16-bit), 1 = needs conversion, 2 = unreadable/invalid.
wav_status() {
    local file="$1" tag bits
    tag=$(wav_format_tag "$file") || return 2
    bits=$(wav_bits_per_sample "$file") || return 2
    [ "$tag" = "1" ] && [ "$bits" = "16" ] && return 0
    return 1
}

convert_to_pcm16() {
    local src="$1" dst="$2"
    "$FFMPEG" -v error -y -i "$src" -c:a pcm_s16le "$dst"
}

report() {
    [ "$QUIET" = "1" ] || printf '%s\n' "$*"
}

fix_wav() {
    local file="$1"
    wav_status "$file"
    case $? in
        0)
            return 0 ;;
        2)
            report "skip  (not a decodable WAV)  $file"
            return 0 ;;
    esac

    local tmp
    tmp="${file}.fix$$.wav"
    if [ "$DRY_RUN" = "1" ]; then
        report "would fix (to PCM 16-bit)     $file"
        return 0
    fi
    if ! convert_to_pcm16 "$file" "$tmp"; then
        report "FAIL  (conversion error)      $file"
        rm -f "$tmp"
        return 1
    fi
    if [ "$BACKUP" = "1" ]; then
        cp -p "$file" "${file}.bak"
    fi
    mv -f "$tmp" "$file"
    report "fixed (to PCM 16-bit)         $file"
}

fix_mp3() {
    local file="$1" out
    out="${file%.mp3}.wav"
    if [ "$DRY_RUN" = "1" ]; then
        report "would convert mp3 -> wav       $file"
        return 0
    fi
    local tmp
    tmp="${out}.fix$$.wav"
    if ! convert_to_pcm16 "$file" "$tmp"; then
        report "FAIL  (conversion error)      $file"
        rm -f "$tmp"
        return 1
    fi
    mv -f "$tmp" "$out"
    report "converted mp3 -> wav           $file"
    if [ "$DELETE_MP3" = "1" ]; then
        rm -f "$file"
        report "deleted source mp3            $file"
    fi
}

SOUNDFONT_EXTS='\.sf2$|\.sf3$|\.dls$|\.gig$|\.sfz$|\.sfark$'

process_dir() {
    local dir="$1"
    local f
    # WAV files (case-insensitive), excluding sound fonts by extension.
    while IFS= read -r -d '' f; do
        case "$f" in
            *.[sS][fF]2|*.[sS][fF]3|*.[dD][lL][sS]|*.[gG][iI][gG]|*.[sS][fF][zZ]|*.[sS][fF][aA][rR][kK])
                continue ;;
        esac
        fix_wav "$f"
    done < <(find "$dir" -type f \( -iname '*.wav' \) -print0)

    # MP3 files (case-insensitive), same sound font exclusion for safety.
    while IFS= read -r -d '' f; do
        fix_mp3 "$f"
    done < <(find "$dir" -type f \( -iname '*.mp3' \) -print0)
}

usage() {
    sed -n '2,24p' "$0" | sed 's/^# \{0,1\}//'
    exit 0
}

main() {
    local dirs=()
    while [ $# -gt 0 ]; do
        case "$1" in
            -n|--dry-run)   DRY_RUN=1 ;;
            -b|--backup)    BACKUP=1 ;;
            -d|--delete-mp3) DELETE_MP3=1 ;;
            -q|--quiet)     QUIET=1 ;;
            -h|--help)      usage ;;
            -*)             die "unknown option: $1" ;;
            *)              dirs+=("$1") ;;
        esac
        shift
    done
    [ ${#dirs[@]} -gt 0 ] || dirs=(".")

    find_tools || die "ffmpeg/ffprobe (or avconv/avprobe) not found"

    local d
    for d in "${dirs[@]}"; do
        [ -d "$d" ] || die "not a directory: $d"
        process_dir "$d"
    done
}

main "$@"
