7650 // PC keyboard interface constants
7651 
7652 #define KBSTATP         0x64    // kbd controller status port(I)
7653 #define KBS_DIB         0x01    // kbd data in buffer
7654 #define KBDATAP         0x60    // kbd data port(I)
7655 
7656 #define NO              0
7657 
7658 #define SHIFT           (1<<0)
7659 #define CTL             (1<<1)
7660 #define ALT             (1<<2)
7661 
7662 #define CAPSLOCK        (1<<3)
7663 #define NUMLOCK         (1<<4)
7664 #define SCROLLLOCK      (1<<5)
7665 
7666 #define E0ESC           (1<<6)
7667 
7668 // Special keycodes
7669 #define KEY_HOME        0xE0
7670 #define KEY_END         0xE1
7671 #define KEY_UP          0xE2
7672 #define KEY_DN          0xE3
7673 #define KEY_LF          0xE4
7674 #define KEY_RT          0xE5
7675 #define KEY_PGUP        0xE6
7676 #define KEY_PGDN        0xE7
7677 #define KEY_INS         0xE8
7678 #define KEY_DEL         0xE9
7679 
7680 // C('A') == Control-A
7681 #define C(x) (x - '@')
7682 
7683 static uchar shiftcode[256] =
7684 {
7685   [0x1D] CTL,
7686   [0x2A] SHIFT,
7687   [0x36] SHIFT,
7688   [0x38] ALT,
7689   [0x9D] CTL,
7690   [0xB8] ALT
7691 };
7692 
7693 static uchar togglecode[256] =
7694 {
7695   [0x3A] CAPSLOCK,
7696   [0x45] NUMLOCK,
7697   [0x46] SCROLLLOCK
7698 };
7699 
7700 static uchar normalmap[256] =
7701 {
7702   NO,   0x1B, '1',  '2',  '3',  '4',  '5',  '6',  // 0x00
7703   '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',
7704   'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  // 0x10
7705   'o',  'p',  '[',  ']',  '\n', NO,   'a',  's',
7706   'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',  // 0x20
7707   '\'', '`',  NO,   '\\', 'z',  'x',  'c',  'v',
7708   'b',  'n',  'm',  ',',  '.',  '/',  NO,   '*',  // 0x30
7709   NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
7710   NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
7711   '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
7712   '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
7713   [0x9C] '\n',      // KP_Enter
7714   [0xB5] '/',       // KP_Div
7715   [0xC8] KEY_UP,    [0xD0] KEY_DN,
7716   [0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
7717   [0xCB] KEY_LF,    [0xCD] KEY_RT,
7718   [0x97] KEY_HOME,  [0xCF] KEY_END,
7719   [0xD2] KEY_INS,   [0xD3] KEY_DEL
7720 };
7721 
7722 static uchar shiftmap[256] =
7723 {
7724   NO,   033,  '!',  '@',  '#',  '$',  '%',  '^',  // 0x00
7725   '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
7726   'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  // 0x10
7727   'O',  'P',  '{',  '}',  '\n', NO,   'A',  'S',
7728   'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  // 0x20
7729   '"',  '~',  NO,   '|',  'Z',  'X',  'C',  'V',
7730   'B',  'N',  'M',  '<',  '>',  '?',  NO,   '*',  // 0x30
7731   NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
7732   NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
7733   '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
7734   '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
7735   [0x9C] '\n',      // KP_Enter
7736   [0xB5] '/',       // KP_Div
7737   [0xC8] KEY_UP,    [0xD0] KEY_DN,
7738   [0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
7739   [0xCB] KEY_LF,    [0xCD] KEY_RT,
7740   [0x97] KEY_HOME,  [0xCF] KEY_END,
7741   [0xD2] KEY_INS,   [0xD3] KEY_DEL
7742 };
7743 
7744 
7745 
7746 
7747 
7748 
7749 
7750 static uchar ctlmap[256] =
7751 {
7752   NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
7753   NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
7754   C('Q'),  C('W'),  C('E'),  C('R'),  C('T'),  C('Y'),  C('U'),  C('I'),
7755   C('O'),  C('P'),  NO,      NO,      '\r',    NO,      C('A'),  C('S'),
7756   C('D'),  C('F'),  C('G'),  C('H'),  C('J'),  C('K'),  C('L'),  NO,
7757   NO,      NO,      NO,      C('\\'), C('Z'),  C('X'),  C('C'),  C('V'),
7758   C('B'),  C('N'),  C('M'),  NO,      NO,      C('/'),  NO,      NO,
7759   [0x9C] '\r',      // KP_Enter
7760   [0xB5] C('/'),    // KP_Div
7761   [0xC8] KEY_UP,    [0xD0] KEY_DN,
7762   [0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
7763   [0xCB] KEY_LF,    [0xCD] KEY_RT,
7764   [0x97] KEY_HOME,  [0xCF] KEY_END,
7765   [0xD2] KEY_INS,   [0xD3] KEY_DEL
7766 };
7767 
7768 
7769 
7770 
7771 
7772 
7773 
7774 
7775 
7776 
7777 
7778 
7779 
7780 
7781 
7782 
7783 
7784 
7785 
7786 
7787 
7788 
7789 
7790 
7791 
7792 
7793 
7794 
7795 
7796 
7797 
7798 
7799 
