//Created with help from https://tchapi.github.io/Adafruit-GFX-Font-Customiser/

const uint8_t HangulBitmaps[] PROGMEM = {
  0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0x18, 0x00, 0x30, 0x00, 0x60, 0x00, 0xC0,
  0x01, 0x80, 0x03, 0x00, 0x06, 0x00, 0x0C, 0x00, 0x18, 0x00, 0x30, 0x00,
  0x60, 0x00, 0xC0, 0x01, 0x80, 0xFF, 0x3F, 0xFF, 0xCF, 0xF0, 0x30, 0x0C,
  0x0C, 0x03, 0x03, 0x00, 0xC0, 0xC0, 0x30, 0x30, 0x0C, 0x0C, 0x03, 0x03,
  0x00, 0xC0, 0xC0, 0x30, 0x30, 0x0C, 0x0C, 0x03, 0x03, 0x00, 0xC0, 0xC0,
  0x30, 0x30, 0x0C, 0x00, 0xC0, 0x01, 0x80, 0x03, 0x00, 0x06, 0x00, 0x0C,
  0x00, 0x18, 0x00, 0x30, 0x00, 0x60, 0x00, 0xC0, 0x01, 0x80, 0x03, 0x00,
  0x06, 0x00, 0x0C, 0x00, 0x1F, 0xFF, 0xFF, 0xFF, 0x80, 0xFF, 0xFF, 0xFF,
  0xFF, 0x00, 0x06, 0x00, 0x0C, 0x00, 0x18, 0x00, 0x30, 0x00, 0x60, 0x00,
  0xC0, 0x01, 0x80, 0x03, 0x00, 0x06, 0x00, 0x0C, 0x00, 0x1F, 0xFF, 0xFF,
  0xFF, 0x80, 0xFF, 0x3F, 0xFF, 0xCF, 0xFC, 0x03, 0x03, 0x00, 0xC0, 0xC0,
  0x30, 0x30, 0x0C, 0x0C, 0x03, 0x03, 0x00, 0xC0, 0xC0, 0x30, 0x30, 0x0C,
  0x0C, 0x03, 0x03, 0x00, 0xC0, 0xC0, 0x30, 0x3F, 0xCF, 0xFF, 0xF3, 0xFC,
  0x00, 0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0x18, 0x00, 0x30, 0x00, 0x60, 0x00,
  0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x01, 0x80, 0x03, 0x00, 0x07, 0x00, 0x0C,
  0x00, 0x1F, 0xFF, 0xFF, 0xFF, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x1E,
  0x00, 0x3C, 0x00, 0x78, 0x00, 0xF0, 0x01, 0xE0, 0x03, 0xC0, 0x07, 0x80,
  0x0F, 0x00, 0x1E, 0x00, 0x3C, 0x00, 0x7F, 0xFF, 0xFF, 0xFF, 0x80, 0xC0,
  0x07, 0x80, 0x0F, 0x00, 0x1E, 0x00, 0x3C, 0x00, 0x78, 0x00, 0xFF, 0xFF,
  0xFF, 0xFF, 0xC0, 0x07, 0x80, 0x0F, 0x00, 0x1E, 0x00, 0x3C, 0x00, 0x7F,
  0xFF, 0xFF, 0xFF, 0x80, 0xC3, 0x30, 0xF0, 0xCC, 0x3C, 0x33, 0x0F, 0x0C,
  0xC3, 0xC3, 0x30, 0xF0, 0xCC, 0x3F, 0xF3, 0xFF, 0xFC, 0xFF, 0xC3, 0x30,
  0xF0, 0xCC, 0x3C, 0x33, 0x0F, 0x0C, 0xC3, 0xC3, 0x30, 0xFF, 0xCF, 0xFF,
  0xF3, 0xFC, 0x00, 0x01, 0x00, 0x02, 0x00, 0x04, 0x00, 0x1C, 0x00, 0x38,
  0x00, 0x70, 0x01, 0xB0, 0x03, 0x60, 0x06, 0xC0, 0x18, 0xC0, 0x31, 0x80,
  0xC1, 0x83, 0x01, 0x8E, 0x03, 0xB8, 0x03, 0x80, 0x08, 0x04, 0x02, 0x01,
  0x00, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x07, 0x03, 0x81, 0xC0, 0xE0,
  0x70, 0x38, 0x36, 0x1B, 0x0D, 0x86, 0xC3, 0x61, 0xB1, 0x8C, 0xC6, 0x63,
  0x31, 0xB4, 0x78, 0x3F, 0x1E, 0x0C, 0x00, 0x07, 0xC0, 0x3F, 0xE0, 0xE0,
  0xE3, 0x00, 0x66, 0x00, 0xD8, 0x00, 0xF0, 0x01, 0xE0, 0x03, 0xC0, 0x07,
  0x80, 0x0D, 0x80, 0x33, 0x00, 0x63, 0x83, 0x83, 0xFE, 0x01, 0xF0, 0x00,
  0xFF, 0xFF, 0xFF, 0xFC, 0x0E, 0x00, 0x1C, 0x00, 0x38, 0x00, 0x70, 0x01,
  0xB0, 0x03, 0x60, 0x06, 0xC0, 0x18, 0xC0, 0x31, 0x80, 0xC1, 0x83, 0x01,
  0x8E, 0x03, 0xB8, 0x03, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x80, 0x40,
  0x20, 0x10, 0x08, 0x04, 0x07, 0x03, 0x81, 0xC0, 0xE0, 0x70, 0x38, 0x36,
  0x1B, 0x0D, 0x86, 0xC3, 0x61, 0xB1, 0x8C, 0xC6, 0x63, 0x31, 0xB0, 0x78,
  0x3C, 0x1E, 0x0C, 0x00, 0x03, 0x80, 0x07, 0x00, 0x0E, 0x07, 0xFF, 0xFF,
  0xFF, 0xE0, 0x70, 0x00, 0xE0, 0x03, 0x60, 0x06, 0xC0, 0x18, 0xC0, 0x31,
  0x80, 0xC1, 0x83, 0x01, 0x8E, 0x03, 0xB8, 0x03, 0x80, 0xFF, 0xFF, 0xFF,
  0xFC, 0x00, 0x18, 0x00, 0x30, 0x00, 0x60, 0x00, 0xC0, 0x01, 0xBF, 0xFF,
  0x7F, 0xFE, 0x00, 0x0C, 0x00, 0x18, 0x00, 0x30, 0x00, 0x60, 0x00, 0xC0,
  0x01, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x06, 0x00, 0x0C, 0x00, 0x18,
  0x00, 0x3F, 0xFF, 0xFF, 0xFF, 0xC0, 0x01, 0x80, 0x03, 0x00, 0x06, 0x00,
  0x0C, 0x00, 0x1F, 0xFF, 0xFF, 0xFF, 0x80, 0xFF, 0xFF, 0xFF, 0xFC, 0x60,
  0xC0, 0xC1, 0x81, 0x83, 0x03, 0x06, 0x06, 0x0C, 0x0C, 0x18, 0x18, 0x30,
  0x30, 0x60, 0x60, 0xC0, 0xC1, 0x81, 0x83, 0x1F, 0xFF, 0xFF, 0xFF, 0x80,
  0x03, 0x00, 0x0C, 0x0F, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x3F, 0x03, 0xFF,
  0x1C, 0x0E, 0xE0, 0x1F, 0x00, 0x3C, 0x00, 0xF8, 0x07, 0x70, 0x38, 0xFF,
  0xC0, 0xFC, 0x00, 0xC6, 0x31, 0x8C, 0x63, 0xFF, 0xC6, 0x31, 0x8C, 0x63,
  0x00, 0xCF, 0x3C, 0xF3, 0xCF, 0x3F, 0xFF, 0xCF, 0x3C, 0xF3, 0xCF, 0x3C,
  0xC0, 0xC6, 0x31, 0x8F, 0xFF, 0x18, 0xC7, 0xFF, 0x8C, 0x63, 0x00, 0xCF,
  0x3C, 0xF3, 0xFF, 0xFC, 0xF3, 0xCF, 0xFF, 0xF3, 0xCF, 0x3C, 0xC0, 0x18,
  0xC6, 0x31, 0x8F, 0xFF, 0x18, 0xC6, 0x31, 0x8C, 0x60, 0x19, 0x8C, 0xC6,
  0x63, 0x31, 0x98, 0xCF, 0xE7, 0xF3, 0x19, 0x8C, 0xC6, 0x63, 0x31, 0x98,
  0xCC, 0x66, 0x00, 0x18, 0xC6, 0x3F, 0xFC, 0x63, 0x1F, 0xFE, 0x31, 0x8C,
  0x60, 0x19, 0x8C, 0xC6, 0x63, 0x3F, 0x9F, 0xCC, 0x66, 0x33, 0x19, 0xFC,
  0xFE, 0x63, 0x31, 0x98, 0xCC, 0x66, 0x00, 0x01, 0x80, 0x01, 0x80, 0x01,
  0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0x0C, 0x30, 0x0C, 0x30, 0x0C, 0x30, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x80, 0x01, 0x80, 0x01,
  0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0x0C, 0x30, 0x0C, 0x30, 0x0C, 0x30, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0x00
};

const GFXglyph HangulGlyphs[] PROGMEM = {
  {     0,  15,  15,  17,    1,  -15 },   // 0x1100 Kiyeok
  {    29,  18,  15,  20,    1,  -15 },   // 0x1101 Ssangkiyeok
  {    64,  15,  15,  17,    1,  -15 },   // 0x1102 Nieun
  {    93,  15,  15,  17,    1,  -15 },   // 0x1103 Tikeut
  {   122,  18,  15,  20,    1,  -15 },   // 0x1104 Ssangtikeut
  {   157,  15,  15,  17,    1,  -15 },   // 0x1105 Rieul
  {   186,  15,  15,  17,    1,  -15 },   // 0x1106 Mieum
  {   215,  15,  15,  17,    1,  -15 },   // 0x1107 Pieup
  {   244,  18,  15,  20,    1,  -15 },   // 0x1108 Ssangpieup
  {   279,  15,  15,  17,    1,  -15 },   // 0x1109 Sios
  {   308,  18,  15,  20,    1,  -15 },   // 0x110A Ssangsios
  {   343,  15,  15,  17,    1,  -15 },   // 0x110B Ieung
  {   372,  15,  15,  17,    1,  -15 },   // 0x110C Cieuc
  {   401,  18,  15,  20,    1,  -15 },   // 0x110D Ssangcieuc
  {   436,  15,  15,  17,    1,  -15 },   // 0x110E Chieuch
  {   465,  15,  15,  17,    1,  -15 },   // 0x110F Khieukh
  {   494,  15,  15,  17,    1,  -15 },   // 0x1110 Thieuth
  {   523,  15,  15,  17,    1,  -15 },   // 0x1111 Phieuph
  {   552,  14,  15,  16,    1,  -15 },   // 0x1112 Hieuh
  {     0,   0,   0,   0,    0,    0 },   // 0x1113  [[ No need for any composed forms ...
  {     0,   0,   0,   0,    0,    0 },   // 0x1114
  {     0,   0,   0,   0,    0,    0 },   // 0x1115
  {     0,   0,   0,   0,    0,    0 },   // 0x1116
  {     0,   0,   0,   0,    0,    0 },   // 0x1117
  {     0,   0,   0,   0,    0,    0 },   // 0x1118
  {     0,   0,   0,   0,    0,    0 },   // 0x1119
  {     0,   0,   0,   0,    0,    0 },   // 0x111A
  {     0,   0,   0,   0,    0,    0 },   // 0x111B
  {     0,   0,   0,   0,    0,    0 },   // 0x111C
  {     0,   0,   0,   0,    0,    0 },   // 0x111D
  {     0,   0,   0,   0,    0,    0 },   // 0x111E
  {     0,   0,   0,   0,    0,    0 },   // 0x111F
  {     0,   0,   0,   0,    0,    0 },   // 0x1120
  {     0,   0,   0,   0,    0,    0 },   // 0x1121
  {     0,   0,   0,   0,    0,    0 },   // 0x1122
  {     0,   0,   0,   0,    0,    0 },   // 0x1123
  {     0,   0,   0,   0,    0,    0 },   // 0x1124
  {     0,   0,   0,   0,    0,    0 },   // 0x1125
  {     0,   0,   0,   0,    0,    0 },   // 0x1126
  {     0,   0,   0,   0,    0,    0 },   // 0x1127
  {     0,   0,   0,   0,    0,    0 },   // 0x1128
  {     0,   0,   0,   0,    0,    0 },   // 0x1129
  {     0,   0,   0,   0,    0,    0 },   // 0x112A
  {     0,   0,   0,   0,    0,    0 },   // 0x112B
  {     0,   0,   0,   0,    0,    0 },   // 0x112C
  {     0,   0,   0,   0,    0,    0 },   // 0x112D
  {     0,   0,   0,   0,    0,    0 },   // 0x112E
  {     0,   0,   0,   0,    0,    0 },   // 0x112F
  {     0,   0,   0,   0,    0,    0 },   // 0x1130
  {     0,   0,   0,   0,    0,    0 },   // 0x1131
  {     0,   0,   0,   0,    0,    0 },   // 0x1132
  {     0,   0,   0,   0,    0,    0 },   // 0x1133
  {     0,   0,   0,   0,    0,    0 },   // 0x1134
  {     0,   0,   0,   0,    0,    0 },   // 0x1135
  {     0,   0,   0,   0,    0,    0 },   // 0x1136
  {     0,   0,   0,   0,    0,    0 },   // 0x1137
  {     0,   0,   0,   0,    0,    0 },   // 0x1138
  {     0,   0,   0,   0,    0,    0 },   // 0x1139
  {     0,   0,   0,   0,    0,    0 },   // 0x113A
  {     0,   0,   0,   0,    0,    0 },   // 0x113B
  {     0,   0,   0,   0,    0,    0 },   // 0x113C
  {     0,   0,   0,   0,    0,    0 },   // 0x113D
  {     0,   0,   0,   0,    0,    0 },   // 0x113E
  {     0,   0,   0,   0,    0,    0 },   // 0x113F
  {     0,   0,   0,   0,    0,    0 },   // 0x1140
  {     0,   0,   0,   0,    0,    0 },   // 0x1141
  {     0,   0,   0,   0,    0,    0 },   // 0x1142
  {     0,   0,   0,   0,    0,    0 },   // 0x1143
  {     0,   0,   0,   0,    0,    0 },   // 0x1144
  {     0,   0,   0,   0,    0,    0 },   // 0x1145
  {     0,   0,   0,   0,    0,    0 },   // 0x1146
  {     0,   0,   0,   0,    0,    0 },   // 0x1147
  {     0,   0,   0,   0,    0,    0 },   // 0x1148
  {     0,   0,   0,   0,    0,    0 },   // 0x1149
  {     0,   0,   0,   0,    0,    0 },   // 0x114A
  {     0,   0,   0,   0,    0,    0 },   // 0x114B
  {     0,   0,   0,   0,    0,    0 },   // 0x114C
  {     0,   0,   0,   0,    0,    0 },   // 0x114D
  {     0,   0,   0,   0,    0,    0 },   // 0x114E
  {     0,   0,   0,   0,    0,    0 },   // 0x114F
  {     0,   0,   0,   0,    0,    0 },   // 0x1150
  {     0,   0,   0,   0,    0,    0 },   // 0x1151
  {     0,   0,   0,   0,    0,    0 },   // 0x1152
  {     0,   0,   0,   0,    0,    0 },   // 0x1153
  {     0,   0,   0,   0,    0,    0 },   // 0x1154
  {     0,   0,   0,   0,    0,    0 },   // 0x1155
  {     0,   0,   0,   0,    0,    0 },   // 0x1156
  {     0,   0,   0,   0,    0,    0 },   // 0x1157
  {     0,   0,   0,   0,    0,    0 },   // 0x1158
  {     0,   0,   0,   0,    0,    0 },   // 0x1159
  {     0,   0,   0,   0,    0,    0 },   // 0x115A
  {     0,   0,   0,   0,    0,    0 },   // 0x115B
  {     0,   0,   0,   0,    0,    0 },   // 0x115C
  {     0,   0,   0,   0,    0,    0 },   // 0x115D
  {     0,   0,   0,   0,    0,    0 },   // 0x115E
  {     0,   0,   0,   0,    0,    0 },   // 0x115F
  {     0,   0,   0,   0,    0,    0 },   // 0x1160 ... ]
  {   579,   5,  15,  14,    8,  -15 },   // 0x1161 A
  {   589,   6,  15,  15,    8,  -15 },   // 0x1162 Ae
  {   601,   5,  15,  14,    8,  -15 },   // 0x1163 Ya
  {   611,   6,  15,  15,    8,  -15 },   // 0x1164 Yae
  {   623,   5,  15,  14,    8,  -15 },   // 0x1165 Eo
  {   633,   9,  15,  18,    8,  -15 },   // 0x1166 E
  {   651,   5,  15,  14,    8,  -15 },   // 0x1167 Yeo
  {   661,   9,  15,  18,    8,  -15 },   // 0x1168 Ye
  {   679,  16,   5,  18,    1,   -5 },   // 0x1169 O
  {     0,   0,   0,   0,    0,    0 },   // 0x116A (composed)
  {     0,   0,   0,   0,    0,    0 },   // 0x116B (composed)
  {     0,   0,   0,   0,    0,    0 },   // 0x116C (composed)
  {   689,  16,   5,  18,    1,   -5 },   // 0x116D Yo
  {   699,  16,   5,  18,    1,   -5 },   // 0x116E U
  {     0,   0,   0,   0,    0,    0 },   // 0x116F (composed)
  {     0,   0,   0,   0,    0,    0 },   // 0x1170 (composed)
  {     0,   0,   0,   0,    0,    0 },   // 0x1171 (composed)
  {   709,  16,   5,  18,    1,   -5 },   // 0x1172 Yu
  {   719,  16,   2,  18,    1,   -2 },   // 0x1173 Eu
  {     0,   0,   0,   0,    0,    0 },   // 0x1174 (composed)
  {   723,   2,  15,  13,   10,  -15 }    // 0x1175 I
};

const GFXfont HangulJamoFont PROGMEM = {
  (uint8_t  *)HangulBitmaps,
  (GFXglyph *)HangulGlyphs,
  0x1100, 0x1175,
  1
};

#define HANGEUL_KIYEOK          u"\x1100"
#define HANGEUL_SSANGKIYEOK     u"\x1101"
#define HANGEUL_NIEUN           u"\x1102"
#define HANGEUL_TIKEUT          u"\x1103"
#define HANGEUL_SSANGTIKEUT     u"\x1104"
#define HANGEUL_RIEUL           u"\x1105"
#define HANGEUL_MIEUM           u"\x1106"
#define HANGEUL_PIEUP           u"\x1107"
#define HANGEUL_SSANGPIEUP      u"\x1108"
#define HANGEUL_SIOS            u"\x1109"
#define HANGEUL_SSANGSIOS       u"\x110A"
#define HANGEUL_IEUNG           u"\x110B"
#define HANGEUL_CIEUC           u"\x110C"
#define HANGEUL_SSANGCIEUC      u"\x110D"
#define HANGEUL_CHIEUCH         u"\x110E"
#define HANGEUL_KHIEUKH         u"\x110F"
#define HANGEUL_THIEUTH         u"\x1110"
#define HANGEUL_PHIEUPH         u"\x1111"
#define HANGEUL_HIEUH           u"\x1112"
#define HANGEUL_A               u"\x1161"
#define HANGEUL_AE              u"\x1162"
#define HANGEUL_YA              u"\x1163"
#define HANGEUL_YAE             u"\x1164"
#define HANGEUL_EO              u"\x1165"
#define HANGEUL_E               u"\x1166"
#define HANGEUL_YEO             u"\x1167"
#define HANGEUL_YE              u"\x1168"
#define HANGEUL_O               u"\x1169"
#define HANGEUL_YO              u"\x116D"
#define HANGEUL_U               u"\x116E"
#define HANGEUL_YU              u"\x1172"
#define HANGEUL_EU              u"\x1173"
#define HANGEUL_I               u"\x1175"
