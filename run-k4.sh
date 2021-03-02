#!/bin/bash
##############################################################################
# Run script for CEPCSW:
# - run a simple job
#
# Usage:
# $ ./run.sh Examples/options/helloalg.py
# or:
# $ 
#
# Author: Tao Lin <lintao@ihep.ac.cn>
##############################################################################

function info:() {
    echo "INFO: $*" 1>&2
}

function error:() {
    echo "ERROR: $*" 1>&2
}

function build-dir() {
    local blddir=build

    if [ -n "${bldtool}" ]; then
        blddir=${blddir}.${bldtool}
    fi

    # If detect the extra env var, append it to the build dir
    if [ -n "${k4_version}" ]; then
        blddir=${blddir}.${k4_version}
    fi
    if [ -n "${k4_platform}" ]; then
        blddir=${blddir}.${k4_platform}
    fi

    echo $blddir
}


function run-job() {
    local blddir=$(build-dir)

    $blddir/run gaudirun.py $*
}

##############################################################################
# Parse the command line options
##############################################################################

# The current default platform
k4_platform=x86_64-linux-gcc9-opt
k4_version=master
bldtool=${CEPCSW_BLDTOOL} # make, ninja # set in env var


run-job $* || exit -1
