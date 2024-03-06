#!/usr/bin/env bash
if [[ $# -ne 2 ]]; then
    echo "Usage: $0 <check|fix> folder"
    exit 1
fi
task="${1}"; shift
dir="${1}"; shift
fileDir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

function check_syntax()
{
    # If run-clang-format is not installed, clone it
    if [ ! -f  run-clang-format/run-clang-format.py ]; then

      git clone https://github.com/Sarcasm/run-clang-format.git
      if [ ! $? -eq 0 ]; then
        echo "Error installing run-clang-format."
        exit 1
      fi
    fi

    python3 run-clang-format/run-clang-format.py --recursive ${dir} --extensions "h,hpp,c,cpp"
    if [ ! $? -eq 0 ]; then
      echo "Error: C++ Code formatting in file ${d} is not normalized."
      echo "Solution: Please run '$fileDir/style_cxx.sh fix' to fix it."
      exit -1
    fi
}

function fix_syntax()
{
     src_files=`find ${dir} -type f -name "*.hpp" -o -name "*.h" -o -name "*.c" -o -name "*.cpp"`
     echo $src_files | xargs -n6 -P2 clang-format -style=file -i "$@"
     if [ ! $? -eq 0 ]; then
        echo "Error running run-clang-format."
        exit 1
     fi
}

##############################################
### Testing/fixing C++ Code Style
##############################################
command -v clang-format >/dev/null
if [ ! $? -eq 0 ]; then
    echo "Error: please install clang-format on your system."
    exit -1
fi
 
if [[ "${task}" == 'check' ]]; then
    check_syntax
else
    fix_syntax
fi

exit 0