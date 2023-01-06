# train_ctl_sw

# code

* [trainctl](https://github.com/banyanshade-software/train_ctl_sw/tree/master/trainctl) : **main code is here** (directory includes some documentation too)
* mac : train_throttle command and instrument application (uses trainctl)
* stm32f407VETx, stm32f103_ui, stm32f103_switcher, stm32f103_dispatchboard : project directory for several boards board (uses trainctl, compile here)

# project

Target is to have a full Z-scale layout control, both in automatic mode and manual "safe" mode.

We do not use DCC (digital train control) because on Z-scale DCC decoders do not fit all locomotives and are expensives

We use "traditional" analog control, where track is divided in blocks, and we can control power sent to any block.

**Note** in the code we use the word _canton_ instead of block (french word for block system) to avoid confusion
between so-called cantons and logical sub-blocks (see trainctl)

Choice had been made to have one power board (including PWM and volt control) for each canton

## related project

[train_ctl_hw](https://github.com/banyanshade-software/train_ctl_hw) contains boards schematic and PCBs,
thus boards have several not yet resolved issues in this repositiory.

Schematics are defined using [KiCad](https://www.kicad.org)

[train_ctl_cad](https://github.com/banyanshade-software/train_ctl_cad) contains laser cut train layout (which
is definetly usable only for my own layout) as well as some laser cut houses and building that are ready to be 
used (card board laser cut, 1mm width)

hop

## License

All this is GPLv3 (well, see LICENSE file)
