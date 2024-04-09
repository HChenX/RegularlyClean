<div align="center">
<h1>RegularlyClean</h1>

![stars](https://img.shields.io/github/stars/HChenX/RegularlyClean?style=flat)
![downloads](https://img.shields.io/github/downloads/HChenX/RegularlyClean/total)
![Github repo size](https://img.shields.io/github/repo-size/HChenX/RegularlyClean)
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/HChenX/RegularlyClean)](https://github.com/HChenX/RegularlyClean/releases)
[![GitHub Release Date](https://img.shields.io/github/release-date/HChenX/RegularlyClean)](https://github.com/HChenX/RegularlyClean/releases)
![last commit](https://img.shields.io/github/last-commit/HChenX/RegularlyClean?style=flat)
![language](https://img.shields.io/badge/language-java-purple)

[//]: # (<p><b><a href="README-en.md">English</a> | <a href="README.md">简体中文</a></b></p>)
<p>是一个可以定时自动清理垃圾文件的模块！</p>
</div>

# ✨模块介绍

- 使用 crond 进行定时的自动垃圾文件清理模块。
- 拥有超自由的自定义功能，随你所想！
- 使用 c 语言高效代码实现部分功能。

# 💡模块说明

- 首先最重要的是模块基于：[Petit-Abba](https://github.com/Petit-Abba)
  的 [black_and_white_list](https://github.com/Petit-Abba/black_and_white_list) 模块。
- 本模块是基于其模块的超深度自定义拓展完善而来。
- 于原模块区别如下：

* 比如：
  * 1.支持自动安全模式，`/*`将会自动被忽略，需要替换成`/&*`使用，避免手滑勿清。
  * 2.支持大文件跳过，比如扫描到大于“20”MB的文件会被自动忽略，防止可能的勿清个人文件。（可选）
  * 3.支持在ROOT管理器直接控制模块是否运行，禁用模块停止运行、反之恢复。
  * 4.支持在自定义APP界面自动触发清理，比如进入MT自动清理一遍，主打眼不见心不烦。
  * 5.完全支持MAGISK和KSU等ROOT管理器。
  * 6.超强自定义清理时间定制。
  * 7.等更多精彩等你发掘···

# 🙏致谢名单

- 本模块代码参考了如下项目，对此表示由衷的感谢：

|         项目名称         |                                    项目链接                                    |
|:--------------------:|:--------------------------------------------------------------------------:|
| black_and_white_list | [black_and_white_list](https://github.com/Petit-Abba/black_and_white_list) |
|         翻译提供         |                                    提供者                                     |
|         简体中文         |                                  焕晨HChen                                   |

# 📢项目声明

- 任何对本项目的使用必须注明作者，抄袭是不可接受的！

# 🌏免责声明

- 使用本模块即代表愿意承担一切后果。
- 任何由本项目衍生出的项目本项目不承担任何责任。

# 🎉结尾

- 感谢您愿意使用本模块！Enjoy your day! ♥️
