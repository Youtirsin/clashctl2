#!/bin/bash

# this script sets up clashctl, it will do:
# 1. install requirements and build clashctl
# 2. backup clash config if exists because of previous installation and using
# 3. cp `clashctl` the folder to $HOME and `clashctl` the executable to $HOME/clashctl

requirements=("cmake" "curl" "make" "g++")

config_file_path="$HOME/clashctl/config/config.yaml"

function info()
{
  echo "[INFO] $@"
}

function error()
{
  echo "[ERROR] $@" >&2
}

function fail()
{
  error $@
  exit 1
}

function check_for_file()
{
  if [ ! -e "$1" ]; then
    error "missing file: $1"
    return 0
  fi
  return 1
}

function check_installed()
{
  local exe_name=$1
  if which "$exe_name" >/dev/null 2>&1; then
    info "$exe_name has been installed."
    return 1
  else
    info "$exe_name has not been installed."
    return 0
  fi
}

function check_requirements_needed()
{
  requirements_needed=()
  for r in "${requirements[@]}"; do
    check_installed $r
    if [[ $? -eq 0 ]]; then
      requirements_needed+=("$r")
    fi
  done
}

function ensure_requirements_installed()
{
  check_requirements_needed
  if [ ${#requirements_needed[@]} -eq 0 ]; then
    info "all requirements have been installed."
    return
  fi

  to_be_installed=$(IFS=" "; echo "${requirements_needed[*]}")
  info "installing $to_be_installed"
  sudo apt install -y $to_be_installed

  check_requirements_needed
  if [ ${#requirements_needed[@]} -eq 0 ]; then
    info "all requirements have been installed."
    return
  fi

  fail "failed to install all requirements."
}

function build()
{
  ensure_requirements_installed

  rm -rf build
  mkdir build
  cd build && cmake .. && cmake --build .
  cd ..
}

function maybe_backup_clash_config()
{
  check_for_file $config_file_path
  if [[ $? -eq 0 ]]; then
    return 0
  fi
  info "backing up config file."
  rm -rf config_backup
  mkdir config_backup
  cp $config_file_path config_backup/config.yaml
  return 1
}

function recover_clash_config()
{
  info "recovering config file."
  cp config_backup/config.yaml $config_file_path
}

function setup()
{
  build
  maybe_backup_clash_config
  local config_backed_up=$?

  info "copying clashctl folder."
  cp -r clashctl $HOME/
  info "copying clashctl executable."
  cp build/clashctl $HOME/clashctl/

  if [[ $config_backed_up -eq 1 ]]; then
    recover_clash_config
  fi

  info "clashctl has been installed."
}

setup
