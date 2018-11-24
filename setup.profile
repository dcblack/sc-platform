#!/bin/bash
#
#
#$Info: setup.profile - environment settings for SystemC/TLM platform examples. $

# DOCUMENTATION
function help() {
  cat <<EMBEDDED_MARKDOWN
NAME
----
`setup.profile` - environment settings for SystemC/TLM  platform examples

SYNOPSIS
--------

```sh
source setup.profile
```

DESCRIPTION
-----------
Source this file to setup tools

OPTIONS
-------
None

DEPENDENCIES
------------
Assumes the following:

- BASH shell
- boost library
- C++ compiler - Either clang/llvm v9.1 or g++ v5.2 or later
- dirname
- git 2.19 or later
- header - perl script (located in bin/ directory)
- module - bash alias for the modulecmd tcl script that sets up environment variables in BASH.
- perl v5.14 or later
- systemc v2.3.2 or later

COPYRIGHT & LICENSE
-------------------
Copyright (C) 2018 Doulos. All rights reserved.

Licensed under Apache 2.0.

AUTHOR
------
David C Black <david.black@doulos.com>

EMBEDDED_MARKDOWN
}
firstreal() {
  perl -le '@_=split($;,join($;,@ARGV));for(@_){next unless -e $_;print $_;exit 0;}' "$@"
}
real_path() {
  # USAGE: real_path DIR
  perl -M'Cwd(abs_path)' -le '$d=$ARGV[0];print abs_path($d)if -d $d' "$1"
}
has_path() {
  # USAGE: has_path VAR PATH
  arg=$(real_path "$2")
  if [[ "$arg" == "" ]]; then return 1; fi
  perl -M'Cwd(abs_path)' -le '$v=$ARGV[0];$p=$ARGV[1];for$d(split(":",$ENV{$v})){next if !-d $d;exit 0 if$p eq abs_path($d);}exit 1' "$1" "$arg"
}
prepend_path() { # only if 2nd arg does not exist in first
  # USAGE: prepend_path VAR PATH
  arg="$(real_path '$2')"
  has_path "$1" "$2" || \
    eval $(perl -le 'print qq{$ARGV[0]="$ARGV[1]:$ENV{$ARGV[0]}"; export $ARGV[0]}' "$1" "$arg")
}
append_path() { # only if 2nd arg does not exist in first
  # USAGE: append_path VAR PATH
  var="$1"
  arg="$(real_path '$2')"; shift
  has_path "$1" "$2" || \
    eval $(perl -le 'print qq{$ARGV[0]="$ENV{$ARGV[0]}:$ARGV[1]"; export $ARGV[0]}' "$var" "$arg")
}
unique_path() {
  # USAGE: unique_path VAR
  eval $(perl -M'Cwd(abs_path)' -e '$v=$ARGV[0];for$d(split(qr":",$ENV{$v})){next if !-d $d;$e=abs_path($d);if(!exists$e{$e}){$e{$e}=1;push(@e,$e);}};printf qq{%s="%s"\n},$v,join(":",@e)' "$1")
}
remove_path() {
  # USAGE: remove_path VAR PATH
  eval $(perl -M'Cwd(abs_path)' -e '$v=$ARGV[0];$p=abs_path($ARGV[1]);for(split(qr":",$ENV{$v})){$e=abs_path($_);if($p ne $e){$push(@e,$e);}};print "$v=",join(":",@e)' "$1" "$2")
}

export PROJ_ROOT=$(realpath $(dirname $(firstreal ./.git ../.git ../../.git ../../../.git ../../../../.git)))
PROJ_BIN="$PROJ_ROOT/bin"
prepend_path PATH $PROJ_BIN
header -uc platform 1>&2
if [[ "$(uname -s)" == "Darwin" ]]; then
  export CXX=clang++
  export CC=clang
  module add clang
  module add boost
  module add systemc
elif [[ "$(uname -s)" == "Linux" ]]; then
  export CXX=clang++
  export CC=clang
  module add clang
  module add boost
  module add systemc
else
  echo "WARNING: Unsupported OS for setup.profile" 1>&2
fi
export CMAKE_MODULE_PATH=CMakeModules
export SETUP_PROFILE=$(basename $PROJ_ROOT)
module list
