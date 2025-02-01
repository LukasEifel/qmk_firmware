// fontconvert -f~/repos/Noto_Sans_Symbols_2/NotoSansSymbols2-Regular.ttf -s20 -r50 -v _ClipCut_ 0x2702 0x2702 
// Visualize your font via https://tchapi.github.io/Adafruit-GFX-Font-Customiser

const uint8_t NotoSansSymbols2_Regular_ClipCut_20pt16bBitmaps[] PROGMEM = {
  0x3E, 0x00, 0x00, 0x00, 0x01, 0x8E, 0x00, 0x00, 0x00, 0x0C, 0x18, 0x00,
  0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x01, 0xF3,
  0x03, 0x00, 0x00, 0x3F, 0xF7, 0x0C, 0x00, 0x07, 0xFF, 0x9F, 0xF8, 0x00,
  0xFF, 0xF0, 0x1F, 0xFC, 0x0F, 0xFE, 0x00, 0x00, 0xFF, 0xFF, 0xC0, 0x00,
  0x00, 0xFF, 0xFC, 0x00, 0x00, 0x00, 0xFF, 0x80, 0x00, 0x00, 0x07, 0xFF,
  0x00, 0x00, 0x00, 0x3F, 0xFF, 0x00, 0x01, 0xFF, 0xF3, 0xFF, 0x80, 0x1F,
  0xFF, 0x03, 0xFF, 0xC0, 0x70, 0x60, 0x01, 0xFF, 0xC3, 0x00, 0xC0, 0x01,
  0xFF, 0xEC, 0x03, 0x00, 0x00, 0xFF, 0xF0, 0x0C, 0x00, 0x00, 0x7C, 0xC0,
  0x60, 0x00, 0x00, 0x01, 0x83, 0x00, 0x00, 0x00, 0x01, 0xF0, 0x00, 0x00,
  0x00, 0x00 };

const GFXglyph NotoSansSymbols2_Regular_ClipCut_20pt16bGlyphs[] PROGMEM = {
// bmpOff,   w,   h,xAdv, xOff, yOff      range 0 (0x2702 - 0x2702)
  {     0,  38,  23,  42,    2,  -21 } }; // 0x2702 uni2702  (#0)

const GFXfont NotoSansSymbols2_Regular_ClipCut_20pt16b PROGMEM = {
  (uint8_t  *)NotoSansSymbols2_Regular_ClipCut_20pt16bBitmaps,
  (GFXglyph *)NotoSansSymbols2_Regular_ClipCut_20pt16bGlyphs,
  0x2702, // first
  0x2702, // last
  50   //height
 };

// Approx. 124 bytes
