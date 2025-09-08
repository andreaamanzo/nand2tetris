#!/usr/bin/env sh

# Enhanced JackCompiler launcher:
#  - Recursively compiles .jack files in a directory
#  - Supports -o OUTDIR to collect .vm outputs in a separate folder
#
# Usage:
#   $(basename "$0")                      # compile current dir (recursive)
#   $(basename "$0") DIR                  # compile DIR (recursive)
#   $(basename "$0") FILE.jack            # compile single file
#   $(basename "$0") -o OUTDIR [PATH]     # compile and place .vm into OUTDIR
#
# Author: adapted from nand2tetris JackCompiler.sh

unset CDPATH

script="`test -L "$0" && readlink -n "$0" || echo "$0"`"
launcher_dir="$PWD"
cd "`dirname "$script"`" || exit 1

# --- parse args (-h/--help, -o OUTDIR, PATH) ---
outdir=""
patharg=""
while [ $# -gt 0 ]; do
  case "$1" in
    -h|--help)
      echo "Usage:"
      echo "  $(basename "$0")                      Compiles all .jack files in current dir (recursive)."
      echo "  $(basename "$0") DIRECTORY            Compiles all .jack files in DIRECTORY (recursive)."
      echo "  $(basename "$0") FILE.jack            Compiles FILE.jack to FILE.vm."
      echo "  $(basename "$0") -o OUTDIR [PATH]     Compiles and moves .vm under OUTDIR, preserving tree."
      exit 0
      ;;
    -o)
      shift
      [ -z "$1" ] && { echo "Error: -o requires an argument"; exit 2; }
      outdir="$1"
      shift
      ;;
    --)
      shift
      break
      ;;
    -*)
      echo "Unknown option: $1"
      exit 2
      ;;
    *)
      patharg="$1"
      shift
      ;;
  esac
done

# default to current working dir from where user invoked the script
if [ -z "$patharg" ]; then
  arg1="$launcher_dir"
else
  case "$patharg" in
    /*) arg1="$patharg" ;;
    *)  arg1="$launcher_dir/$patharg" ;;
  esac
fi

# normalize OUTDIR to absolute path if provided
if [ -n "$outdir" ]; then
  case "$outdir" in
    /*) ;;
    *)  outdir="$launcher_dir/$outdir" ;;
  esac
  mkdir -p "$outdir" || { echo "Error: cannot create OUTDIR '$outdir'"; exit 3; }
fi

classpath="${CLASSPATH}:bin/classes:bin/lib/Hack.jar:bin/lib/Compilers.jar"
jackc() {
  java -classpath "$classpath" Hack.Compiler.JackCompiler "$1"
}

move_vm_if_needed() {
  # $1 = source .jack absolute path
  [ -z "$outdir" ] && return 0
  srcjack="$1"
  base="${srcjack%*.jack}"
  vm_src="$base.vm"
  if [ -f "$vm_src" ]; then
    case "$arg1" in
      */) root="$arg1" ;;
      *)  root="$arg1/" ;;
    esac
    rel="${srcjack#$root}"       # relative path (e.g. sub/dir/Foo.jack)
    rel_dir=`dirname "$rel"`     # e.g. sub/dir
    dest_dir="$outdir/$rel_dir"
    mkdir -p "$dest_dir" || return 1
    mv "$vm_src" "$dest_dir/" || return 1
    echo "→ Moved: $vm_src -> $dest_dir/"
  fi
}

echo "Compiling '$arg1'"

if [ -f "$arg1" ]; then
  # Single file
  jackc "$arg1" || exit $?
  move_vm_if_needed "$arg1"
  exit 0
fi

if [ -d "$arg1" ]; then
  # Directory: recurse and compile each .jack
  find "$arg1" -type f -name '*.jack' | while IFS= read -r jackfile; do
    echo " - $jackfile"
    jackc "$jackfile" || exit $?
    move_vm_if_needed "$jackfile" || exit $?
  done
  exit 0
fi

echo "Error: '$arg1' is not a file or directory."
exit 4
