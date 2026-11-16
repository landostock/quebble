"""Check the distributable ZIP independently of its packager."""
import hashlib
import io
import json
import zipfile
from pathlib import Path
from PIL import Image

root = Path(__file__).resolve().parents[1]
version = json.loads((root/"package.json").read_text())["version"]
archive = root/"publish"/f"Quebble-{version}-Publish-Kit.zip"
prefix = f"Quebble-{version}-Publish-Kit/"
with zipfile.ZipFile(archive) as bundle:
    assert bundle.testzip() is None
    manifest = json.loads(bundle.read(prefix+"manifest.json"))
    assert manifest["published"] is False
    assert manifest["version"] == version
    screenshots = []
    gallery = []
    for entry in manifest["files"]:
        data = bundle.read(prefix+entry["path"])
        assert len(data) == entry["bytes"], entry["path"]
        assert hashlib.sha256(data).hexdigest() == entry["sha256"], entry["path"]
        if "dimensions" in entry:
            with Image.open(io.BytesIO(data)) as image:
                assert list(image.size) == entry["dimensions"]
                if entry["path"]=="images/banner-720x320.png":
                    rgb=image.convert("RGB")
                    for x in (0,719):
                        assert sum(max(rgb.getpixel((x,y)))>128 for y in range(320))>=32
                if entry["path"].startswith("screenshots/"):
                    assert image.size == (200,228)
                    screenshots.append(entry["path"])
                if entry["path"].startswith("gallery/"):
                    assert image.size == (200,228)
                    gallery.append(entry["path"])
        if entry["path"].endswith(".pbw"):
            assert data == (root/"build/quebble.pbw").read_bytes()
            with zipfile.ZipFile(io.BytesIO(data)) as pbw:
                metadata = json.loads(pbw.read("appinfo.json"))
                assert metadata["versionLabel"] == version
                assert metadata["targetPlatforms"] == ["emery"]
    assert len(screenshots) == 5
    assert len(gallery) == 8
    assert len({bundle.read(prefix+name) for name in gallery}) == 8
    assert set(bundle.namelist()) == {
        prefix+entry["path"] for entry in manifest["files"]
    } | {prefix+"manifest.json"}
with zipfile.ZipFile(root/"publish"/f"Quebble-{version}-Screenshots.zip") as bundle:
    assert bundle.testzip() is None
    images=[name for name in bundle.namelist() if name.endswith(".png")]
    assert len(images)==8
    for name in images:
        assert bundle.read(name)==(root/"store/gallery"/Path(name).name).read_bytes()
print("store kit ok: ZIP integrity, all hashes, five upload images, eight gallery images, exact PBW")
