# clashctl2
 a cli tool for clash.

## Install

### Prerequisites

all requirements will be installed when installing with `bash setup.sh`.

- cmake
- curl
- make
- g++ (C++ 17 support required)

```bash
# clashctl will be install to ~
bash setup.sh

# uninstall
# just remove the folder
rm -r $HOME/clashctl

# update
git pull
bash setup.sh
```

## Usage

```bash
# update clash config
~/clashctl/clashctl update <url>

# show options and select
~/clashctl/clashctl

# or type commands
# start clash
~/clashctl/clashctl start

# after started, use the shortcuts to set or unset proxy for your terminal
~/clashctl/set_proxy
~/clashctl/unset_proxy

# stop clash
~/clashctl/clashctl stop

# see more from help
~/clashctl/clashctl help
```
