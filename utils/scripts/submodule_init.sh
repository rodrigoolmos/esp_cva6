#!/bin/bash
# Copyright (c) 2011-2025 Columbia University, System Level Design Group
# SPDX-License-Identifier: Apache-2.0

set -e

SCRIPT_PATH=$(realpath $(dirname "$0"))
ESP_ROOT=$(realpath ${SCRIPT_PATH}/../..)

noyes () {
    while true; do
	read -p "$1 [y|n]? n: " yn
	yn=${yn:-n}
	case $yn in
            [Yy]* ) echo "y"; break;;
            [Nn]* ) echo "n"; break;;
            * ) ;;
	esac
    done
}

INSTALL_ARIANE=0
INSTALL_CVA6=0
INSTALL_IBEX=0
INSTALL_ARIANE_LINUX=0
INSTALL_LEON3_LINUX=0
INSTALL_STRATUS_INC=0
INSTALL_MATCHLIB=0
INSTALL_CHISEL=0
INSTALL_NVDLA=0
INSTALL_SPANDEX=0
INSTALL_BASEJUMP=0
INSTALL_ZYNQ=0
INSTALL_EIGEN=0

if [ $(noyes "*** QUESTION : Do you want to install the Ariane core?") == "y" ]; then
    INSTALL_ARIANE=1
fi

if [ $(noyes "*** QUESTION : Do you want to install the CVA6 core?") == "y" ]; then
    INSTALL_CVA6=1
fi

if [ $(noyes "*** QUESTION : Do you want to install the Ibex core?") == "y" ]; then
    INSTALL_IBEX=1
fi

if [ $(noyes "*** QUESTION : Do you want to install Linux for the Ariane core?") == "y" ]; then
    INSTALL_ARIANE_LINUX=1
fi

if [ $(noyes "*** QUESTION : Do you want to install Linux for the Leon3 core?") == "y" ]; then
    INSTALL_LEON3_LINUX=1
fi

if [ $(noyes "*** QUESTION : Do you want to install support for accelerator design with Stratus HLS?") == "y" ]; then
    INSTALL_STRATUS_INC=1
fi

if [ $(noyes "*** QUESTION : Do you want to install support for accelerator design with Matchlib in Catapult HLS?") == "y" ]; then
    INSTALL_MATCHLIB=1
fi

if [ $(noyes "*** QUESTION : Do you want to install support for accelerator design with Chisel?") == "y" ]; then
    INSTALL_CHISEL=1
fi

if [ $(noyes "*** QUESTION : Do you want to install NVDLA?") == "y" ]; then
    INSTALL_NVDLA=1
fi

if [ $(noyes "*** QUESTION : Do you want to install Spandex caches?") == "y" ]; then
    INSTALL_SPANDEX=1
fi

if [ $(noyes "*** QUESTION : Do you want to install Basejump to simulate designs with a DDR controller?") == "y" ]; then
    INSTALL_BASEJUMP=1
fi

if [ $(noyes "*** QUESTION : Do you want to install support for Zynq boards?") == "y" ]; then
    INSTALL_ZYNQ=1
fi

if [ $(noyes "*** QUESTION : Do you want to install Eigen?") == "y" ]; then
    INSTALL_EIGEN=1
fi

echo "*** Installing Required and Selected Submodules ***"
git submodule update --init --recursive rtl/caches/esp-caches

if [ ${INSTALL_ARIANE} == 1 ]; then
    git submodule update --init --recursive rtl/cores/ariane/ariane
    git submodule update --init --recursive soft/ariane/opensbi
    git submodule update --init --recursive soft/ariane/riscv-pk
    git submodule update --init --recursive soft/ariane/riscv-tests
fi

if [ ${INSTALL_CVA6} == 1 ]; then
    git submodule update --init --recursive rtl/cores/cva6/cva6
    git submodule update --init --recursive soft/ariane/opensbi
    git submodule update --init --recursive soft/ariane/riscv-pk
    git submodule update --init --recursive soft/ariane/riscv-tests

    CVA6_PATCH=${ESP_ROOT}/rtl/cores/cva6/cva6_local_changes.patch
    CVA6_SUBMODULE=${ESP_ROOT}/rtl/cores/cva6/cva6
    if [ -f "${CVA6_PATCH}" ]; then
        if git -C "${CVA6_SUBMODULE}" apply --check "${CVA6_PATCH}" >/dev/null 2>&1; then
            echo "*** Applying local CVA6 patch ***"
            git -C "${CVA6_SUBMODULE}" apply "${CVA6_PATCH}"
        else
            echo "*** Skipping CVA6 local patch (already applied or conflicting) ***"
        fi
    fi
fi

if [ ${INSTALL_IBEX} == 1 ]; then
    git submodule update --init --recursive rtl/cores/ibex/ibex
    git submodule update --init --recursive soft/ariane/opensbi
    git submodule update --init --recursive soft/ariane/riscv-pk
    git submodule update --init --recursive soft/ariane/riscv-tests
fi

if [ ${INSTALL_ARIANE_LINUX} == 1 ]; then
    git submodule update --init --recursive soft/ariane/linux
fi

if [ ${INSTALL_LEON3_LINUX} == 1 ]; then
    git submodule update --init --recursive soft/leon3/linux
fi

if [ ${INSTALL_STRATUS_INC} == 1 ]; then
    git submodule update --init --recursive accelerators/stratus_hls/common/inc
fi

if [ ${INSTALL_MATCHLIB} == 1 ]; then
    git submodule update --init --recursive accelerators/catapult_hls/common/matchlib_toolkit
fi

if [ ${INSTALL_CHISEL} == 1 ]; then
    git submodule update --init --recursive accelerators/chisel/hw
fi

if [ ${INSTALL_NVDLA} == 1 ]; then
    git submodule update --init --recursive accelerators/third-party/NV_NVDLA
fi

if [ ${INSTALL_SPANDEX} == 1 ]; then
    git submodule update --init --recursive rtl/caches/spandex-caches
fi

if [ ${INSTALL_BASEJUMP} == 1 ]; then
    git submodule update --init --recursive rtl/peripherals/
fi

if [ ${INSTALL_ZYNQ} == 1 ]; then
    git submodule update --init --recursive utils/zynq
fi

if [ ${INSTALL_EIGEN} == 1 ]; then
    git submodule update --init --recursive soft/common/drivers/common/utils/eigen
fi
