#!/bin/bash
#
#
#$Info: setup.profile - environment settings for SystemC/TLM platform examples. $

# DOCUMENTATION
function help() {
  perldoc $0
  cat >/dev/null <<.
=pod

=head1 NAME

setup.profile - environment settings for SystemC/TLM  platform examples

=head1 SYNOPSIS

    source setup.profile

=head1 DESCRIPTION

Source this file to setup tools

=head1 OPTIONS

None

=head1 DEPENDENCIES

Assumes the following:

=over

=item BASH shell

=item boost library

=item clang/llvm

=item g++ v5.2 or later

=item git 2.19 or later

=item header - perl script

=item module - bash alias

Invokes the modulecmd tcl script that sets
up environment variables in BASH.

=item perl v5.14 or later

=item systemc v2.3.2 or later

=back

=head1 COPYRIGHT & LICENSE

Copyright (C) 2018 Doulos. All rights reserved.

Licensed under Apache 2.0.

=head1 AUTHOR

David C Black <david.black@doulos.com>

=cut

.
}
firstreal() {
  perl -le '@_=split($;,join($;,@ARGV));for(@_){next unless -e $_;print $_;exit 0;}' "$@"
}
realpath() {
  perl -M'Cwd(abs_path)' -le 'print abs_path("@ARGV")' "$@"
}
prepend_path() { # only if 2nd arg does not exist in first
  # USAGE: prepend_path PATH ./bin
  var="$1"
  arg="$(realpath '$2')"
  echo $(perl -e 'print $ENV{"@ARGV"},"\n";' "$var") | grep -q -s "$arg"
  if [ "$?" -eq 1 ] ; then
      eval $(perl -e 'print qq{$ARGV[0]="$ARGV[1]:$ENV{$ARGV[0]}"; export $ARGV[0]\n}' "$var" "$arg")
  fi
}
append_path() { # only if 2nd arg does not exist in first
  # USAGE: append_path PATH ./bin
  var="$1"
  arg="$(realpath '$2')"; shift
  echo $(perl -e 'print $ENV{"@ARGV"},"\n";' "$var") | grep -q -s "$arg"
  if [ "$?" -eq 1 ] ; then
      eval $(perl -e 'print qq{$ARGV[0]="$ENV{$ARGV[0]}:$ARGV[1]"; export $ARGV[0]\n}' "$var" "$arg")
  fi
}

root=$(dirname $(firstreal ./.git ../.git ../../.git ../../../.git ../../../../.git))
prepend_path PATH "$(realpath $root/bin)"
header Sandbox 1>&2
if [[ "$(uname -s)" == "Darwin" ]]; then
  export CXX=clang++
  export CPP=clang
  module add clang
  module add boost
  module add systemc
elif [[ "$(uname -s)" == "Linux" ]]; then
  export CXX=g++
  export CPP=gcc
  module add devtools
  module add boost
  module add systemc
else
  echo "WARNING: Unsupported OS for setup.profile" 1>&2
fi
export CMAKE_MODULE_PATH=CMakeModules
module list
