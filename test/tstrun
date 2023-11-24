#!/bin/bash

##  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
##  SPDX-License-Identifier: MIT

usage () {
  echo "Usage:"
  echo "   tstrun [options] [wildcard] [tags]"
  echo ""
  echo "OPTIONS"
  echo "  -h | --help                 this help"
  echo "  -l | --list                 prints the list of available tests"
  echo "  -c | --color                toggles coloured messages on/off"
  echo "  -d | --test-directory dir   cd to the directory dir with tests"
  echo "  -w | --wildcard '*x[yz]'    specify a file pattern to match the tests to execute"
  echo "  -o | --output filename      the name of the generated logfile"
  echo ""
  echo "WILDCARD"
  echo "  A filter to select which tests to run ('*' by default). Note that it MUST be"
  echo "  single quoted to prevent shell expansion. The initial 't_' is implied."
  echo ""
  echo "TAGS"
  echo "  [+/-]tagname to turn the tag on/off"
  exit 1
}

WILDCARD="*"
COLOR=""
OUTFILE="test.log"
TAGS=""

COLOR_TOGGLE=1
DEFAULT_OPTIONS=0

toggle_color () {
    if [ $COLOR_TOGGLE -eq 0 ]; then
      COLOR_TOGGLE=1
      ## First character is \033 (ESC)
      COLOR_RED="[1;31m"
      COLOR_GREEN="[0;32m"
      COLOR_YELLOW="[0;33m"
      COLOR_NONE="[0m"
    else
      COLOR_TOGGLE=0
      COLOR_RED=""
      COLOR_GREEN=""
      COLOR_YELLOW=""
      COLOR_NONE=""
    fi
}

toggle_color # Set default to colors off

if [ "$TSTOPTIONS" != "" ] ; then
  set -- $TSTOPTIONS "@@" "$@"
  DEFAULT_OPTIONS=1
fi

while [ "$#" -gt 0 ]; do
  case $1 in 

   -h | --help) 
      usage ;;

   -l | --list)
      find . -name "t_$WILDCARD" -print | sed -e '/\.c$/d' -e '/\.cpp$/d' -e '/\.o$/d' | while read -r f ; do $f --l ; done
      exit 0 ;;

   -w | --wildcard)
      shift
      if [ "$#" -eq 0 ]; then echo "ERROR: Missing wildcard"; usage ; fi
      WILDCARD=$1
      shift ;;

   -d | --test-directory)
      shift
      if [ "$#" -eq 0 ]; then echo "ERROR: Missing directory"; usage ; fi
      cd $1
      shift ;;

   @@)
      DEFAULT_OPTIONS=0
      shift
      ;;

   -c | --color)
      toggle_color
      if [ $DEFAULT_OPTIONS -eq 0 ] ; then COLOR="--c" ; fi
      shift
      ;;

    -o | --output)
      shift;
      if [ "$#" -eq 0 ]; then echo "ERROR: Missing output file name"; usage ; fi
      OUTFILE=$1;
      shift ;;

    --*) echo "Invalid option $1"; usage ;;

    +*|-*) 
        if [ $DEFAULT_OPTIONS -eq 0 ] ; then break ; fi
        TAGS="$TAGS $1" #collect tags that may be in default options
        shift
        ;;

    *)  WILDCARD=$1
        shift
        break;
        ;;
  esac
done

echo "output: $OUTFILE ($WILDCARD)"
echo "STIME `date`" > $OUTFILE
find . -name "t_$WILDCARD" -print | sed -e '/\.c$/d' -e '/\.cpp$/d' -e '/\.o$/d' | while read -r f ; do $f $COLOR $TAGS $* ; done 2>> $OUTFILE

FAIL=`grep '^[0-9 ]*FAIL' $OUTFILE | wc -l`
PASS=`grep '^[0-9 ]*PASS' $OUTFILE | wc -l`
SKIP=`grep '^[0-9 ]*SKIP' $OUTFILE | wc -l`

echo "TOTAL $COLOR_RED$FAIL FAIL$COLOR_NONE | $COLOR_GREEN$PASS PASS$COLOR_NONE | $COLOR_YELLOW$SKIP SKIP$COLOR_NONE" >> $OUTFILE

echo "ETIME `date`" >> $OUTFILE
