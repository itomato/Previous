#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu
  /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu/Debug/build68k < /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu/table68k >cpudefs.c
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu
  /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu/Release/build68k < /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu/table68k >cpudefs.c
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu
  /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu/MinSizeRel/build68k < /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu/table68k >cpudefs.c
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu
  /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu/RelWithDebInfo/build68k < /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu/table68k >cpudefs.c
fi

