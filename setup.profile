#!/bin/bash
#
#
#$Info: setup.profile - environment settings for SystemC/TLM platform examples. $

let ERROR_COUNT=0

# DOCUMENTATION
function about() {
  cat <<EMBEDDED_MARKDOWN
NAME
----
`setup.profile` - environment settings for SystemC/TLM  platform examples

SYNOPSIS
--------

```bash
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
- realpath
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

function firstreal() {
  perl -le '@_=split($;,join($;,@ARGV));for(@_){next unless -e $_;print $_;exit 0;}' "$@"
}

function has_path() {
  # USAGE: has_path VAR PATH
  arg=$(realpath "$2")
  if [[ "$arg" == "" ]]; then return 1; fi
  perl -M'Cwd(abs_path)' -le '$v=$ARGV[0];$p=$ARGV[1];for$d(split(":",$ENV{$v})){next if !-d $d;exit 0 if$p eq abs_path($d);}exit 1' "$1" "$arg"
}

function prepend_path() { # only if 2nd arg does not exist in first
  # USAGE: prepend_path VAR PATH
  arg="$(realpath '$2')"
  has_path "$1" "$2" || \
    eval $(perl -le 'print qq{$ARGV[0]="$ARGV[1]:$ENV{$ARGV[0]}"; export $ARGV[0]}' "$1" "$arg")
}

function append_path() { # only if 2nd arg does not exist in first
  # USAGE: append_path VAR PATH
  var="$1"
  arg="$(realpath '$2')"; shift
  has_path "$1" "$2" || \
    eval $(perl -le 'print qq{$ARGV[0]="$ENV{$ARGV[0]}:$ARGV[1]"; export $ARGV[0]}' "$var" "$arg")
}

function unique_path() {
  # USAGE: unique_path VAR
  eval $(perl -M'Cwd(abs_path)' -e '$v=$ARGV[0];for$d(split(qr":",$ENV{$v})){next if !-d $d;$e=abs_path($d);if(!exists$e{$e}){$e{$e}=1;push(@e,$e);}};printf qq{%s="%s"\n},$v,join(":",@e)' "$1")
}

function remove_path() {
  # USAGE: remove_path VAR PATH
  eval $(perl -M'Cwd(abs_path)' -e '$v=$ARGV[0];$p=abs_path($ARGV[1]);for(split(qr":",$ENV{$v})){$e=abs_path($_);if($p ne $e){push(@e,$e);}};print "$v=",join(":",@e)' "$1" "$2")
}

function validate_tool() {
  if [[ $# != 2 ]]; then
    echo "ERROR: $0 not called properly" 1>&2
    let ++ERROR_COUNT
  elif [[ "$2" == '' ]]; then
    echo "ERROR: $1 is not set" 1>&2
    let ++ERROR_COUNT
  elif [[ "$(which $2)" == '' ]]; then
    echo "ERROR: $1='$2' is not found" 1>&2
    let ++ERROR_COUNT
  elif [[ ! -x "$(which $2)" ]]; then
    echo "ERROR: $1='$2' is not executable" 1>&2
    let ++ERROR_COUNT
  fi
}

function defacto_setup() {
  # Cross fingers - used only if modules not installed
  export CC CXX SYSTEMC
  case $# in
    2) CC="$1"
       CXX="$2"
       SYSTEMC='/apps/systemc'
       ;;
    1) CC="$1" 
       if [[ "$CC" =~ *gcc ]]; then
         CXX="g++"
       elif [[ "$CC" =~ *clang ]]; then
         CXX="clang++"
       else
         echo "ERROR: Unable to determine C++ compiler" 1>&2;
         let ++ERROR_COUNT
       fi
       SYSTEMC='/apps/systemc'
       ;;
    0) CC="gcc" 
       CXX="g++"
       SYSTEMC='/apps/systemc'
       ;;
  esac
  validate_tool CC $CC
  validate_tool CXX $CXX
  export SYSTEMC_HOME="$SYSTEMC"
  # ? BOOST variables ?
}

# Setup Modules 
# - obtained from <https://github.com/cea-hpc/modules>
# - see ABOUT_MODULES.md
# Environment variable is EDA used to locate Modules installation
# - You may need to customize this bit.
if [[ "$EDA" = "" ]]; then
  export EDA=/apps
fi
# Check if environment is already setup. If not, start it.
if [[ "$MODULESHOME" = "" && -x $EDA/Modules/default/init/bash ]]; then
  echo "Setting up modules"
  . /apps/Modules/default/init/bash
fi

export PROJ_ROOT=$(realpath $(dirname $(firstreal ./.git ../.git ../../.git ../../../.git ../../../../.git)))
PROJ_BIN="$PROJ_ROOT/bin"
PROJECT="Example"
if [[ "$action" == '' ]]; then
  action='add'
fi
if [[ "$action" == 'rm' ]]; then
  remove_path PATH $PROJ_BIN
else
  prepend_path PATH $PROJ_BIN
fi
if [[ "$lws" == '' || "$lws" != "$nws" ]]; then
  test "$action" == 'add' && header -uc platform 1>&2
  if [[ "$(uname -s)" == "Darwin" ]]; then
    if [[ "$MODULESHOME" != '' ]]; then
      module $action clang
      module $action boost
      module $action systemc
    else
      defacto_setup clang clang++
    fi
  elif [[ "$(uname -s)" == "Linux" ]]; then
    if [[ "$MODULESHOME" != '' ]]; then
      module $action clang
      module $action boost
      module $action systemc
    else
      defacto_setup gcc g++
    fi
  else
    echo "WARNING: Unsupported OS for setup.profile" 1>&2
  fi
  export CMAKE_MODULE_PATH=CMakeModules
  export SETUP_PROFILE=$(basename $PROJ_ROOT)
  test "$action" == 'add' && module list
fi
