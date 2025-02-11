// fontconvert -f~/repos/Noto_Sans_Symbols_2/NotoSansSymbols2-Regular.ttf -s20 -r50 -v _Calc_ -n0x11000 0x1F5A9 0x1F5AB 
// Visualize your font via https://tchapi.github.io/Adafruit-GFX-Font-Customiser

const uint8_t NotoSansSymbols2_Regular_Calc_20pt16bBitmaps[] PROGMEM = {
  0x7F, 0xFF, 0xEF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x00,
  0x3C, 0x00, 0x03, 0xC0, 0x00, 0x3C, 0x00, 0x07, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xC4, 0x44, 0x3C, 0x44, 0x47, 0xFF, 0xFF, 0xFC, 0x44, 0x43, 0xC4,
  0x44, 0x3F, 0xFF, 0xFF, 0xC4, 0x44, 0x3C, 0x44, 0x43, 0xC4, 0x44, 0x3F,
  0xFF, 0xFF, 0xC4, 0x44, 0x3C, 0x44, 0x43, 0xC4, 0x44, 0x3F, 0xFF, 0xC3,
  0xC4, 0x44, 0x3C, 0x44, 0x5B, 0xC4, 0x44, 0x7F, 0xFF, 0xFF, 0x7F, 0xFF,
  0xE0, 0xFF, 0xFF, 0xFF, 0xFE, 0x3F, 0xFF, 0xFF, 0xF8, 0xFF, 0xFF, 0xFF,
  0xE3, 0xFF, 0xFF, 0xFF, 0xBF, 0xFF, 0xFF, 0xFE, 0x3F, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x1F, 0xFF, 0xFF, 0xF0,
  0x3F, 0xFF, 0xFF, 0x80, 0x7F, 0xFF, 0xFE, 0x01, 0xFF, 0xFF, 0xF8, 0x87,
  0xFF, 0xFF, 0xE0, 0x1F, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xFF, 0x87, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x00, 0xFF, 0xFE, 0x00, 0x03, 0xFF,
  0xF8, 0x01, 0xCF, 0xFF, 0xE0, 0x07, 0x3F, 0xFF, 0x80, 0x1C, 0xFF, 0xFE,
  0x00, 0x73, 0xFF, 0xF8, 0x01, 0xCF, 0xFF, 0xE0, 0x07, 0x3F, 0xFF, 0x80,
  0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x7F, 0xFF, 0xFF, 0xFB, 0x18, 0x00,
  0x0C, 0x3C, 0x60, 0x00, 0x30, 0xF1, 0x80, 0x00, 0xC3, 0xC6, 0x3F, 0xE3,
  0x0F, 0x18, 0x00, 0x0C, 0x3C, 0x63, 0xF0, 0x30, 0xF1, 0x80, 0x00, 0xC3,
  0xC6, 0x3F, 0xC3, 0x0F, 0x18, 0x00, 0x0C, 0x3C, 0x60, 0x00, 0x30, 0xF1,
  0x80, 0x00, 0xC3, 0xC7, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x3C, 0x00,
  0x00, 0x00, 0xF0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x0F, 0x00, 0x00,
  0x00, 0x3C, 0x00, 0x00, 0x00, 0xF0, 0x07, 0xFF, 0x83, 0xC0, 0x11, 0xFE,
  0x0F, 0x00, 0x47, 0xF8, 0x3C, 0x01, 0x1F, 0xE0, 0xF0, 0x04, 0x7F, 0x83,
  0xE0, 0x11, 0xFE, 0x0D, 0x80, 0x47, 0xF8, 0x33, 0x01, 0x1F, 0xE0, 0xC6,
  0x04, 0x7F, 0x83, 0x0F, 0xFF, 0xFF, 0xF8 };

const GFXglyph NotoSansSymbols2_Regular_Calc_20pt16bGlyphs[] PROGMEM = {
// bmpOff,   w,   h,xAdv, xOff, yOff      range 0 (0x1f5a9 - 0x1f5ab)
  {     0,  20,  29,  24,    2,  -28 },   // 0x1F5A9 u1F5A9  (#0)
  {    73,  30,  30,  33,    2,  -28 },   // 0x1F5AA u1F5AA  (#1)
  {   186,  30,  29,  33,    2,  -27 } }; // 0x1F5AB u1F5AB  (#2)

const GFXfont NotoSansSymbols2_Regular_Calc_20pt16b PROGMEM = {
  (uint8_t  *)NotoSansSymbols2_Regular_Calc_20pt16bBitmaps,
  (GFXglyph *)NotoSansSymbols2_Regular_Calc_20pt16bGlyphs,
  0xE5A9, // first
  0xE5AB, // last
  50   //height
 };

// Approx. 323 bytes
