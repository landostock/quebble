"""Create watch-sized signed PCM from the documented CC0 source downloads."""
import argparse
from array import array
from pathlib import Path
import subprocess
import sys

parser = argparse.ArgumentParser()
parser.add_argument("source_dir", type=Path)
args = parser.parse_args()
root = Path(__file__).resolve().parents[1]
output = root / "resources/audio"
output.mkdir(parents=True, exist_ok=True)
clips = {
    "music": ("Juhani Junkala [Retro Game Music Pack] Title Screen.wav", 0, 11.294127),
    "tick": ("pepSound3.mp3", 0, 0.13),
    "start": ("powerUp2.mp3", 0, 0.30),
    "claim": ("powerUp7.mp3", 0, 0.38),
    "hit": ("lowDown.mp3", 0, 0.45),
    "clear": ("highUp.mp3", 0, 0.50),
    "over": ("lowDown.mp3", 0, 0.65),
    "ui": ("pepSound3.mp3", 0, 0.06),
}
for name, (source, start, duration) in clips.items():
    fade_out = min(0.040, duration / 2)
    filters = ["asetpts=N/SR/TB", "highpass=f=160", "lowpass=f=3200"]
    if name == "over":
        filters.insert(1, "asetrate=35280,aresample=44100")
    if name == "music":
        filters = ["asetpts=N/SR/TB", "highpass=f=420:poles=2",
                   "equalizer=f=1200:t=q:w=0.8:g=4", "lowpass=f=3400"]
    raw = subprocess.check_output(["ffmpeg", "-v", "error", "-ss", str(start), "-i",
                    str(args.source_dir / source), "-t", str(duration), "-af", ",".join(filters),
                    "-ac", "1", "-ar", "8000", "-c:a", "pcm_s16le", "-f", "s16le", "-"])
    samples = array("h", raw)
    if sys.byteorder != "little":
        samples.byteswap()
    peak = max(abs(sample) for sample in samples)
    if not peak:
        raise ValueError("Silent source: " + source)
    target_peak = 25600 if name == "music" else 28160
    # Apply envelopes after resampling so interpolation cannot leave a nonzero endpoint.
    attack = round((0.002 if name == "music" else 0.006) * 8000)
    release = round((0.002 if name == "music" else fade_out) * 8000)
    pcm = array("h")
    for i, sample in enumerate(samples):
        gain = min(1.0, i / (attack - 1), (len(samples) - 1 - i) / (release - 1))
        gain = gain * gain * (3 - 2 * gain)
        pcm.append(max(-32768, min(32767, round(sample * target_peak / peak * gain))))
    if name != "music":
        pcm = array("h", [0] * 32) + pcm + array("h", [0] * 256)
    if sys.byteorder != "little":
        pcm.byteswap()
    (output / (name + ".pcm")).write_bytes(pcm.tobytes())
    print(name, (output / (name + ".pcm")).stat().st_size)
