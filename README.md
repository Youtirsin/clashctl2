# clashctl2
 a cli tool for clash

## Install

### Prerequisites

- cmake
- curl
- make
- g++ (C++ 17 support required)

```bash
# clashctl will be install to ~
make install

# uninstall will remove ~/clashctl
make uninstall

# you can `git pull` the new code and update in place
make reinstall
```



## Usage

```bash
# update clash config
~/clashctl/clashctl update <url>

# start clash
~/clashctl/clashctl start

# once started, use the shortcuts to apply or de-apply proxy for your terminal
~/clashctl/set_proxy
~/clashctl/unset_proxy

# see more from help
~/clashctl/clashctl help
```

