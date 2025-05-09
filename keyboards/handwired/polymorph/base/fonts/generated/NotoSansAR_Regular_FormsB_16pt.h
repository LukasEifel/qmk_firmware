// fontconvert -f~/repos/noto-sans-arabic/static/NotoSansArabic/NotoSansArabic-Regular.ttf -s16 -r40 -v _FormsB_ 0xfef5 0xfef5 0xfefb 0xfefb 
// Visualize your font via https://tchapi.github.io/Adafruit-GFX-Font-Customiser

const uint8_t NotoSansArabic_Regular_FormsB_16pt16bBitmaps[] PROGMEM = {
  0x78, 0x00, 0x7F, 0xE3, 0x21, 0xE1, 0xC0, 0x00, 0xE0, 0x00, 0x70, 0xC0,
  0x38, 0xF0, 0x1C, 0x3C, 0x0E, 0x0F, 0x07, 0x03, 0xE3, 0x80, 0x7B, 0xC0,
  0x1F, 0xC0, 0x07, 0xE0, 0x01, 0xF0, 0x00, 0x78, 0x00, 0x7E, 0x00, 0x7F,
  0x00, 0x7B, 0xC0, 0x78, 0xE0, 0x78, 0x70, 0x78, 0x78, 0x7F, 0xF8, 0x3F,
  0xF8, 0x00, 0x38, 0x00, 0x38, 0x00, 0x70, 0x00, 0xE4, 0x01, 0xCC, 0x03,
  0xBE, 0x07, 0x1E, 0x0E, 0x1E, 0x1C, 0x1F, 0x38, 0x1F, 0xE0, 0x0F, 0xC0,
  0x0F, 0x80, 0x0F, 0x00, 0x3F, 0x00, 0xFE, 0x01, 0xDE, 0x07, 0x1C, 0x3C,
  0x38, 0xF0, 0xF3, 0xFF, 0xC7, 0xFF, 0x00 };

const GFXglyph NotoSansArabic_Regular_FormsB_16pt16bGlyphs[] PROGMEM = {
// bmpOff,   w,   h,xAdv, xOff, yOff      range 0 (0xfef5 - 0xfef5)
  {     0,  17,  23,  21,   -1,  -22 },   // 0xFEF5 uniFEF5  (#0)
  {     0,   0,   0,   0,    0,    0 },   // 0xFEF6 (skip)
  {     0,   0,   0,   0,    0,    0 },   // 0xFEF7 (skip)
  {     0,   0,   0,   0,    0,    0 },   // 0xFEF8 (skip)
  {     0,   0,   0,   0,    0,    0 },   // 0xFEF9 (skip)
  {     0,   0,   0,   0,    0,    0 },   // 0xFEFA (skip)
// bmpOff,   w,   h,xAdv, xOff, yOff      range 1 (0xfefb - 0xfefb)
  {    49,  15,  22,  18,    1,  -21 } }; // 0xFEFB uniFEFB  (#1)

const GFXfont NotoSansArabic_Regular_FormsB_16pt16b PROGMEM = {
  (uint8_t  *)NotoSansArabic_Regular_FormsB_16pt16bBitmaps,
  (GFXglyph *)NotoSansArabic_Regular_FormsB_16pt16bGlyphs,
  0xFEF5, // first
  0xFEFB, // last
  40   //height
 };

// Approx. 147 bytes
