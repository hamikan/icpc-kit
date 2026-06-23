# Source this file from bash or zsh:
#   source init.sh

if [ -n "${BASH_VERSION:-}" ]; then
  _icpc_init_path="${BASH_SOURCE[0]}"
elif [ -n "${ZSH_VERSION:-}" ]; then
  _icpc_init_path="${(%):-%x}"
else
  _icpc_init_path="$0"
fi

_icpc_init_dir="$(CDPATH= cd -P "$(dirname "$_icpc_init_path")" && pwd)"

export ICPC_KIT="$_icpc_init_dir"
export ICPC_ENV="$ICPC_KIT"

case ":$PATH:" in
  *":$ICPC_KIT/bin:"*) ;;
  *) export PATH="$ICPC_KIT/bin:$PATH" ;;
esac

case ":${CPLUS_INCLUDE_PATH:-}:" in
  *":$ICPC_KIT/ac-library:"*) ;;
  *) export CPLUS_INCLUDE_PATH="$ICPC_KIT/ac-library${CPLUS_INCLUDE_PATH:+:$CPLUS_INCLUDE_PATH}" ;;
esac

if [ -z "${CXX:-}" ]; then
  if command -v g++-15 >/dev/null 2>&1; then
    export CXX=g++-15
  elif command -v g++-14 >/dev/null 2>&1; then
    export CXX=g++-14
  elif command -v g++-13 >/dev/null 2>&1; then
    export CXX=g++-13
  elif [ -x /opt/homebrew/bin/g++ ]; then
    export CXX=/opt/homebrew/bin/g++
  elif [ -x /usr/local/bin/g++ ]; then
    export CXX=/usr/local/bin/g++
  else
    export CXX=g++
  fi
fi

unset _icpc_init_path
unset _icpc_init_dir
