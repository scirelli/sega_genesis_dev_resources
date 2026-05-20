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
        * 1 Playfield canhave a "fixed" window
        * Playfield map
            * Each char position take 2 bytes, that includes
                * Char name (10bits); points to char definition
                * Horizontal flip
                * Vertical flip
                * Color palette (2bits); index into CRAM
                * Priority
                * Scrolling
                    * 1 Pixel scrolling resolution
            ```text
            | ------------------- Name ---------------------- | HF | VF | - CP -- | -P |-S- |
            ┏━━━━┯━━━━┯━━━━┯━━━━┯━━━━┯━━━━┯━━━━┯━━━━┯━━━━┯━━━━┯━━━━┯━━━━┯━━━━┯━━━━┯━━━━┯━━━━┓
            ┃  0 │ 0  │ 0  │ 0  │ 0  │ 0  │ 0  │ 0  │ 0  │ 0  │ 0  │ 0  │ 0  │ 0  │ 0  │ 0  ┃
            ┠────┼────┼────┼────┼────┼────┼────┼────┼────┼────┼────┼────┼────┼────┼────┼────┨
            ┃    │    │    │    │    │    │    │    │    │    │    │    │    │    │    │    ┃
            ┗━━━━┷━━━━┷━━━━┷━━━━┷━━━━┷━━━━┷━━━━┷━━━━┷━━━━┷━━━━┷━━━━┷━━━━┷━━━━┷━━━━┷━━━━┷━━━━┛
            ```
