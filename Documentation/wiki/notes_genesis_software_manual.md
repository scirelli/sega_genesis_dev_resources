# Notes Genesis Software Manual

## Genesis Technical Overview
### Genesis:
* 68000 @ 8mHz
    * Main CPU 
    * 1 MByte (8 Mbit) ROM Area
    * 64 KByte RAM Area
* VDP (Video Display Processor)
    * Dedicated video display processor
        * Controls playfield and sprites
        * Capable of DMA
        * Horizontal and vertical interrupts
* Z80 @4mHz
    * Controls PSG (Programmable Sound Generator) & FM Chip
    * 8 KBytes of dedicated Sound RAM
 
### Video
* Playfield and Sprites are character-based
* Display Area (visual):
    * 40 Chars wide x 28 chars high  40x28
        * Each char is 8x8 pixels
        * Pixel resolution = 320x224
    * 3 Planes:
        * 2 Scrolling playfields
        * 1 Sprite plane
        * Definable priorities between planes
    * Playfields:
        * 6 Different sizes
        * 1 Playfield can have a "fixed" window
        * Playfield map
            * Each char position take 2 bytes, that includes
                * Char name (10bits); points to char definition
                * Horizontal flip
                * Vertical flip
                * Color palette (2bits); index into CRAM
                * Priority
                * Scrolling
                    * 1 Pixel scrolling resolution
                    * Horizontal:
                        * Whole playfield as unit
                        * Each character line
                        * Each scan line
                    * Vertical
                        * Whole playfield as unit
                        * 2 Char wide columns
                            * "2 Char wide column" is a vertical strip of the screen that is 16 pixels wide (2 tiles × 8 pixels) and spans from the top of the screen to the bottom.
                              When the manual lists this under "Vertical" scrolling, it means you can slice the background into these 16-pixel-wide vertical strips, and scroll each strip up or down independently of the others.
                              Whole playfield as unit: You write one vertical scroll value, and the entire background moves up or down together.
                              2 Char wide columns: You write an array of values into the VSRAM (Vertical Scroll RAM). If your screen is 320 pixels wide, you have 20 independent columns (320 / 16 = 20). You can tell Column 0 to scroll up 5 pixels, Column 1 to scroll down 10 pixels, Column 2 to stay still, etc.
                              Why this exists:
                              This hardware feature is how classic Genesis games achieve complex visual tricks without taxing the 68000 CPU. By scrolling these columns independently at different speeds, developers created pseudo-3D floors (like in Street Fighter II), rippling water effects, or rotating cylinders (like the barrels in Sonic the Hedgehog 3).
            ```text
            | ------------------- Name ---------------------- | HF | VF | - CP -- | -P |-S- |
            ┏━━━━┯━━━━┯━━━━┯━━━━┯━━━━┯━━━━┯━━━━┯━━━━┯━━━━┯━━━━┯━━━━┯━━━━┯━━━━┯━━━━┯━━━━┯━━━━┓
            ┃  0 │ 0  │ 0  │ 0  │ 0  │ 0  │ 0  │ 0  │ 0  │ 0  │ 0  │ 0  │ 0  │ 0  │ 0  │ 0  ┃
            ┠────┼────┼────┼────┼────┼────┼────┼────┼────┼────┼────┼────┼────┼────┼────┼────┨
            ┃    │    │    │    │    │    │    │    │    │    │    │    │    │    │    │    ┃
            ┗━━━━┷━━━━┷━━━━┷━━━━┷━━━━┷━━━━┷━━━━┷━━━━┷━━━━┷━━━━┷━━━━┷━━━━┷━━━━┷━━━━┷━━━━┷━━━━┛
            ```




##  Gloassary
* VSRAM: Vertical Scroll RAM
* Character: 8x8 pixels
* Position (character): 2 bytes (word)
