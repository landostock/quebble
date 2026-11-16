"""Validate and assemble a local upload kit. Never contacts a publishing API."""
import hashlib
import json
import shutil
import zipfile
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
SCREENS = ["emery_01_title.png", "emery_02_gameplay.png", "emery_03_territory.png",
           "emery_04_guide.png", "emery_05_level2.png"]
GALLERY = ["emery_01_title.png", "emery_02_guide.png", "emery_03_settings.png",
           "emery_04_gameplay.png", "emery_05_territory.png", "emery_06_hit.png",
           "emery_07_game_over.png", "emery_08_level_2.png"]
ART = {"banner-720x320.png":(720,320), "icon-80.png":(80,80),
       "icon-144.png":(144,144), "icon-512.png":(512,512)}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def check_image(path, size):
    with Image.open(path) as image:
        assert image.size == size, (path, image.size, size)
        assert image.format == "PNG", path
        image.verify()
    assert path.stat().st_size <= 4_400_000, path


def main():
    package = json.loads((ROOT/"package.json").read_text())
    version = package["version"]
    pbw = ROOT/"build/quebble.pbw"
    with zipfile.ZipFile(pbw) as bundle:
        metadata = json.loads(bundle.read("appinfo.json"))
        assert metadata["versionLabel"] == version
        assert metadata["uuid"] == package["pebble"]["uuid"]
        assert metadata["targetPlatforms"] == ["emery"]
        assert "emery/pebble-app.bin" in bundle.namelist()
    assert pbw.stat().st_size <= 4_400_000
    description = (ROOT/"store/copy/description.txt").read_text()
    assert len(description) <= 1600
    assert not any(name in description.lower() for name in ("qix", "nintendo", "game boy"))
    for name,size in ART.items():
        check_image(ROOT/"resources/store"/name, size)
    with Image.open(ROOT/"resources/store/banner-720x320.png") as image:
        image = image.convert("RGB")
        for x in (0,719):
            assert sum(max(image.getpixel((x,y)))>128 for y in range(320))>=32, "Empty banner side band"
    for name in SCREENS:
        check_image(ROOT/"store/screenshots"/name,(200,228))
    for name in GALLERY:
        check_image(ROOT/"store/gallery"/name,(200,228))
    check_image(ROOT/"resources/images/menu_icon.png",(25,25))

    output = ROOT/"publish"/f"Quebble-{version}-Publish-Kit"
    output.mkdir(parents=True, exist_ok=True)
    for directory in ("images","screenshots","gallery","copy","legal","source-art","qa"):
        (output/directory).mkdir(exist_ok=True)
    shutil.copy2(pbw,output/f"quebble-{version}.pbw")
    shutil.copy2(ROOT/"store/START-HERE.md",output/"START-HERE.md")
    for path in (ROOT/"store/copy").iterdir():
        if path.is_file(): shutil.copy2(path,output/"copy"/path.name)
    for name in [*ART,"logo-transparent.png","logo-black.png"]:
        shutil.copy2(ROOT/"resources/store"/name,output/"images"/name)
    shutil.copy2(ROOT/"resources/images/menu_icon.png",output/"images/menu-icon-25.png")
    for name in SCREENS:
        shutil.copy2(ROOT/"store/screenshots"/name,output/"screenshots"/name)
    for old in ("emery_04_touch.png","emery_05_difficulty.png"):
        stale=output/"screenshots"/old
        if stale.exists(): stale.unlink()
    for name in GALLERY:
        shutil.copy2(ROOT/"store/gallery"/name,output/"gallery"/name)
    shutil.copy2(ROOT/"store/gallery/README.md",output/"gallery/README.md")
    for name in ("Apache-2.0.txt","CC0-1.0.txt"):
        shutil.copy2(ROOT/"resources/store"/name,output/"legal"/name)
    shutil.copy2(ROOT/"store/legal/THIRD-PARTY-NOTICES.txt",output/"legal/THIRD-PARTY-NOTICES.txt")
    shutil.copy2(ROOT/"resources/audio/SOURCES.md",output/"legal/audio-sources.md")
    shutil.copy2(ROOT/"docs/store-readiness.md",output/"legal/store-readiness.md")
    shutil.copy2(ROOT/"resources/store/banner-master.png",output/"source-art/banner-master.png")
    shutil.copy2(ROOT/"store/artwork-provenance.md",output/"source-art/artwork-provenance.md")
    shutil.copy2(ROOT/"docs/verification/store-17.md",output/"qa/verification.md")
    manifest = {"name":"Quebble", "version":version, "uuid":metadata["uuid"],
                "targetPlatforms":["emery"], "published":False, "files":[]}
    for path in sorted(output.rglob("*")):
        if not path.is_file() or path.name=="manifest.json": continue
        entry = {"path":str(path.relative_to(output)),"bytes":path.stat().st_size,"sha256":digest(path)}
        if path.suffix==".png":
            with Image.open(path) as image: entry["dimensions"]=list(image.size)
        manifest["files"].append(entry)
    (output/"manifest.json").write_text(json.dumps(manifest,indent=2)+"\n")
    archive = output.parent/(output.name+".zip")
    with zipfile.ZipFile(archive,"w",zipfile.ZIP_DEFLATED) as bundle:
        for path in sorted(output.rglob("*")):
            if path.is_file(): bundle.write(path,Path(output.name)/path.relative_to(output))
    with zipfile.ZipFile(archive) as bundle: assert bundle.testzip() is None
    (archive.parent/(archive.name+".sha256")).write_text(digest(archive)+"  "+archive.name+"\n")
    gallery_zip=output.parent/f"Quebble-{version}-Screenshots.zip"
    with zipfile.ZipFile(gallery_zip,"w",zipfile.ZIP_DEFLATED) as bundle:
        for name in [*GALLERY,"README.md"]:
            bundle.write(ROOT/"store/gallery"/name,Path("Quebble-Screenshots")/name)
    with zipfile.ZipFile(gallery_zip) as bundle: assert bundle.testzip() is None
    print(f"Validated {len(manifest['files'])} files; description {len(description)} characters")
    print(archive)
    print(gallery_zip)
    print(digest(archive))


if __name__ == "__main__":
    main()
