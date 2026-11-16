"""Export the existing Quebble pixel identity and store-sized artwork."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageOps

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "resources/store"
WHITE, CYAN, MAGENTA = "#ffffff", "#00ffff", "#ff00ff"
LETTERS = [
    [14,17,17,17,21,18,13], [17,17,17,17,17,17,14],
    [31,16,16,30,16,16,31], [30,17,17,30,17,17,30],
    [30,17,17,30,17,17,30], [16,16,16,16,16,16,31],
    [31,16,16,30,16,16,31],
]


def wordmark():
    image = Image.new("RGBA", (184, 42))
    draw = ImageDraw.Draw(image)
    for letter, rows in enumerate(LETTERS):
        for row, bits in enumerate(rows):
            for col in range(5):
                if not bits & (16 >> col):
                    continue
                x, y = 4 + letter * 24 + col * 4 + 6 - row, 4 + row * 4
                draw.rectangle((x+2,y+3,x+5,y+6), fill=MAGENTA)
                draw.rectangle((x,y,x+3,y+3), fill=WHITE if row<3 else CYAN)
    return image.crop(image.getbbox())


def icon():
    image = Image.new("RGB", (144,144), "black")
    draw = ImageDraw.Draw(image)
    land = [(0,103),(22,103),(22,118),(63,118),(63,144),(0,144)]
    draw.polygon(land, fill=CYAN)
    draw.line(land[:-1], fill=WHITE, width=3)
    for row, bits in enumerate(LETTERS[0]):
        for col in range(5):
            if bits & (16 >> col):
                x, y = 38+col*10+(6-row)*2, 23+row*10
                draw.rectangle((x+3,y+4,x+12,y+13), fill=MAGENTA)
                draw.rectangle((x,y,x+9,y+9), fill=WHITE if row<3 else CYAN)
    draw.line([(69,119),(109,119),(109,104)], fill="#ffff00", width=3)
    draw.polygon([(109,96),(117,104),(109,112),(101,104)], fill=WHITE)
    draw.polygon([(109,100),(113,104),(109,108),(105,104)], fill=CYAN)
    return image


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    logo = wordmark().resize((wordmark().width*6,wordmark().height*6), Image.Resampling.NEAREST)
    logo.save(OUT / "logo-transparent.png")
    black = Image.new("RGB", logo.size, "black")
    black.paste(logo, mask=logo.getchannel("A"))
    black.save(OUT / "logo-black.png")
    for size in (80,144,512):
        icon().resize((size,size),Image.Resampling.NEAREST).save(OUT / f"icon-{size}.png")
    menu = Image.new("1",(25,25))
    draw = ImageDraw.Draw(menu)
    for row,bits in enumerate(LETTERS[0]):
        for col in range(5):
            if bits & (16>>col):
                x,y=3+col*3+(6-row)//2,2+row*3
                draw.rectangle((x,y,x+2,y+2),fill=1)
    menu.save(ROOT / "resources/images/menu_icon.png")
    # Proportional full-bleed export; trim surplus height, never add side padding.
    master = Image.open(OUT / "banner-master.png").convert("RGB")
    banner = ImageOps.fit(master,(720,320),Image.Resampling.LANCZOS,centering=(0.5,0.4))
    for x in (0,719):
        assert sum(max(banner.getpixel((x,y)))>128 for y in range(320))>=32, "Banner has an empty side band"
    banner.save(OUT / "banner-720x320.png")
    print("Exported Quebble store art")


if __name__ == "__main__":
    main()
