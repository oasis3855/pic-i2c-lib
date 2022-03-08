## I2C接続EEPROMを読み書き (PIC 12F1822)<!-- omit in toc -->

[Home](https://oasis3855.github.io/webpage/) > [Software](https://oasis3855.github.io/webpage/software/index.html) > [Software Download](https://oasis3855.github.io/webpage/software/software-download.html) > [pic-i2c-lib](../README.md) > ***i2c-eeprom*** (this page)

<br />
<br />

Last Updated : Mar. 2013

- [ソフトウエアのダウンロード](#ソフトウエアのダウンロード)
- [概要](#概要)
- [バージョン情報](#バージョン情報)
- [ライセンス](#ライセンス)

<br />
<br />

## ソフトウエアのダウンロード

- ![download icon](../readme_pics/soft-ico-download-darkmode.gif) [このGitHubリポジトリを参照する（ソースコード）](./)

※ ```i2c-lib.c``` , ```serial-lib.c``` は I2Cバス, シリアルバス通信用の共通ライブラリ

## 概要

I2C接続のEEPROMを読み書きするソフトウエア

**基礎データ**

- Microchip I2C EEPROM 24LC32, 24LC64, 24LC128, 24LC256, 24LC512用
- Device Address : 0x50〜0x57 

書き込み後のウエイト時間はポーリングを利用するのではなく、想定される最大時間を満足する値としている。最大時間は「AN1028 : Recommended Usage of Microchip I2CTM Serial EEPROM Devices」の5ページ目に示されているI2Cバスクロック100kHz時の最大値を用いている。

## バージョン情報

- Version 2013/02/03

## ライセンス

このスクリプトは [GNU General Public License v3ライセンスで公開する](https://gpl.mhatta.org/gpl.ja.html) フリーソフトウエア

※ 配布ソースコード等に書かれているライセンス条項は撤回し、上述GNU General Public License v3ライセンスにて配布する。
