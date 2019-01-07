About Modules
=============

This is a shell independent tool for managing environment variables. It uses
TCL scripts to do its work.

The original may be browsed at <https://github.com/cea-hpc/modules>

In order to install Modules for use the following files need the following
commands installed.

Using with csh/tcsh
-------------------

/etc/csh.login needs the following lines after "limit coredumpsize..."

```csh
if ( -x /apps/Modules/default/init/sh ) then
  source /apps/Modules/default/init/csh
endif
```

Using with bash
-------------------

/etc/bash.bashrc needs the following lines after "umask 022...fi"

```sh
if [ -x /apps/Modules/default/init/bash ]; then
  . /apps/Modules/default/init/bash
fi
```

Using with sh
-------------

/etc/profile needs the following lines

```sh
if [ -x /apps/Modules/default/init/sh ]; then
  . /apps/Modules/default/init/sh
fi
```

## For more information

See the original documentation, which may be browsed at
<https://github.com/cea-hpc/modules>

#### The end
