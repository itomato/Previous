#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu
  /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu/Debug/gencpu
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu
  /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu/Release/gencpu
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu
  /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu/MinSizeRel/gencpu
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu
  /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu/RelWithDebInfo/gencpu
fi

