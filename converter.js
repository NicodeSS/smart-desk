const segmentMap = {
  63: "0", // 0b00111111
  6: "1", // 0b00000110
  91: "2", // 0b01011011
  79: "3", // 0b01001111
  102: "4", // 0b01100110
  109: "5", // 0b01101101, S
  125: "6", // 0b01111101
  7: "7", // 0b00000111
  127: "8", // 0b01111111
  111: "9", // 0b01101111
  119: "A", // 0b01110111, R
  124: "b", // 0b01111100
  57: "C", // 0b00111001
  88: "c", // 0b01011000
  94: "d", // 0b01011110
  121: "E", // 0b01111001
  123: "e", // 0b01111011
  113: "F", // 0b01110001
  118: "H", // 0b01110110
  116: "h", // 0b01110100
  48: "I", // 0b00110000, I(left)
  14: "J", // 0b00001110
  56: "L", // 0b00111000
  84: "n", // 0b01010100
  92: "o", // 0b01011100
  115: "P", // 0b01110011, p
  103: "q", // 0b01100111
  80: "r", // 0b01010000
  120: "t", // 0b01111000
  62: "U", // 0b00111110
  28: "u", // 0b00011100
  110: "y", // 0b01101110
  64: "-", // 0b01000000
  72: "=", // 0b01001000
  0: " ", // 0b00000000, Blank
};
const res = [];
for (let i = 0; i < 128; i++) {
  res.push(`'${segmentMap[i] ?? "?"}'`);
}
console.log(`const segmentMap = {${res.join(",")}};`);
