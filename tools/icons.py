from PIL import Image, ImageDraw, ImageFont
import glob
f=(glob.glob('/usr/share/fonts/**/DejaVuSans-Bold.ttf',recursive=True) or [None])[0]
def mk(txt,col,name):
    S=256; im=Image.new('RGBA',(S,S),(0,0,0,0)); d=ImageDraw.Draw(im)
    d.rounded_rectangle([8,8,S-8,S-8],radius=56,fill=col)
    font=ImageFont.truetype(f,170)
    b=d.textbbox((0,0),txt,font=font)
    d.text(((S-(b[2]-b[0]))/2-b[0],(S-(b[3]-b[1]))/2-b[1]),txt,font=font,fill='white')
    im.save(name,sizes=[(16,16),(20,20),(24,24),(32,32),(48,48),(64,64),(256,256)])
mk('М',(0x1F,0x5F,0xC8,255),'on.ico')
mk('A',(0x77,0x77,0x77,255),'off.ico')
