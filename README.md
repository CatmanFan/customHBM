# customHBM
Wii HOME Menu (HBM) library

Originally a fork of [libhomemenu](https://github.com/TheProjecter/libhomemenu) by TheProjecter, this is aiming to be a recode continuing from where the previous library left at the time its development became inactive. Like the original, it is intended to be graphics-library independent, however it may behave differently based on the graphics backend used (libwiisprite, GRRLIB, native, etc.).

It contains derived portions of code from the original libhomemenu (license unknown, original source code is attributed above) translated into C++, however the majority of code has been rewritten from scratch. The project also uses portions of code from [Tantric/dborth](https://github.com/dborth)'s [libwiigui](https://github.com/dborth/libwiigui) ([WiiBrew page](http://wiibrew.org/wiki/Libwiigui)), which is licensed under GNU GPL, as well as derived code of [GRRLIB](https://github.com/GRRLIB/GRRLIB)'s (licensed under MIT) TTF drawing functions. I have chosen to license this project under GNU GPL as it is also the same license used in Tantric/dborth's software (Snes9XGX, FCEUGX, VBAGX, WiiMC, ...) as well as in [WiiSXRX](https://github.com/niuus/WiiSXRX) (as GPL-3.0) and [Not64](https://github.com/extremscorner/not64) (as GPL-2.0).

To install this library, you will need **[libromfs-ogc](https://github.com/NateXS/libromfs-ogc)**. (Confirmed, Japanese font will not work without it)

## Credits
- This library uses the original Nintendo Wii Menu sounds (ripped using Dolphin to account for the reverb filter) and fonts (designed by Fontworks, DynaFont, AsiaSoft Corp, DXKorea and [Noto](https://github.com/notofonts/tifinagh)). It also makes use of [PNGU](https://wiibrew.org/wiki/PNGU) for rendering the images included, and FreeTypeGX to render the aforementioned fonts.

## Languages

This menu is available in the following languages (some localizations have been adapted from existing fan translations of Wii software):

| Name                 | Local Name          | Author(s) / Proofreader(s)                                                                                   |
|----------------------|---------------------|--------------------------------------------------------------------------------------------------------------|
| Japanese             | 日本語               | *Nintendo*                                                                                                   |
| English              | -                   | *Nintendo*                                                                                                   |
| German               | Deutsch             | *Nintendo*                                                                                                   |
| French               | Français            | *Nintendo*                                                                                                   |
| Spanish              | Español             | *Nintendo*                                                                                                   |
| Italian              | Italiano            | *Nintendo*                                                                                                   |
| Dutch                | Nederlands          | *Nintendo*                                                                                                   |
| Korean               | 한국어                | *Nintendo*                                                                                                   |
| Simplified Chinese   | 简体中文             | *Nintendo / iQue*                                                                                            |
| Traditional Chinese  | 繁体中文             | *[TVGZone](https://www.bilibili.com/video/BV1D24y1D7S7/)*                                                    |
| Portuguese           | Português (PT)      | *Mr. Lechkar* (based on official vWii/Wii U manuals and 3DS software)                                        |
| Brazilian Portuguese | Português do Brasil | *[Angel333119, SergioF (RIP) & Alan L. Carlos](https://www.romhacking.net.br/index.php?topic=1632.0)*        |
| Russian              | Русский             | *[CaXaP](https://vk.com/ttydrus)* (version used in NSMBWii translation)                                      |
| Ukrainian            | Українська          | *Mr. Lechkar*                                                                                                |
| Polish               | Polski              | *[Graj Po Polsku](https://grajpopolsku.pl/forum/viewtopic.php?t=1590)* (version used in NSMBWii translation) |
| Swedish              | Svenska             | *Mr. Lechkar* (based on official vWii/Wii U manuals)                                                         |
| Danish               | Dansk               | *Mr. Lechkar* (based on official vWii/Wii U manuals)                                                         |
| Norwegian            | Norsk               | *Mr. Lechkar* (based on official vWii/Wii U manuals)                                                         |
| Finnish              | Suomi               | *Mr. Lechkar* (based on official vWii/Wii U manuals)                                                         |
| Greenlandic          | Kalaallisut         | *Mr. Lechkar*                                                                                                |
| Greek                | Ελληνικά            | *Mr. Lechkar* (based on official vWii/Wii U manuals)                                                         |
| Turkish              | Türkçe              | *Mr. Lechkar* (based on official Wii U manual)                                                               |
| Catalan              | Català              | *[Wiicat](https://sites.google.com/view/wiicat/ca/)* (version used in Mario Kart Wii translation)            |
| Galician             | Galego              | *Mr. Lechkar*                                                                                                |
| Basque               | Euskara             | *Mr. Lechkar*                                                                                                |
| Welsh                | Cymraeg             | *Mr. Lechkar*                                                                                                |
| Okinawan             | 沖縄口               | *Mr. Lechkar*                                                                                                |

## License
Owing to the proprietary licenses accorded by the original fonts, any commercial usage (i.e. with payments or for profit, such as sale) of this library is **prohibited** without consent from the font manufacturers for explicit commercial use. The fonts may be used separately (i.e. outside of the library) **for personal use only**.