"""Validate the actual PCM resources, including sample-exact silent effect handles."""
from array import array
from pathlib import Path
import sys

folder = Path(__file__).resolve().parents[1] / "resources/audio"
total = 0
for name in ("music", "ui", "tick", "start", "claim", "hit", "clear", "over"):
    raw = (folder / (name + ".pcm")).read_bytes()
    assert len(raw) % 2 == 0, name
    samples = array("h", raw)
    if sys.byteorder != "little":
        samples.byteswap()
    assert samples[0] == samples[-1] == 0, name
    assert 0 < max(abs(sample) for sample in samples) <= 28160, name
    if name != "music":
        assert not any(samples[:33]), name
        assert not any(samples[-257:]), name
    total += len(raw)
assert total + 2048 < 256 * 1024
print("audio assets ok: 16-bit PCM, zero endpoints, silent handles, resource headroom")
