#!/usr/bin/env bash

trap "exit" SIGHUP SIGINT SIGTERM

set -o errexit
set -o pipefail
set -o nounset

FILE="circuits/cb/conditional_swap_shuffle1.wir.GC_duplo"
PROFILE="const"
NUM_CIRCUITS="1"
PARALLEL_EXEC="1,1,1"
IP_CONST="127.0.0.1"
PORT_CONST="28001"

INTERACTIVE="0"
TEST="0"

# Reading arguments
while [[ $# -gt 1 ]]
do
    key="$1"

    case $key in
        -f|--file)
        FILE="$2"
        shift
        ;;
        --profile)
        PROFILE="$2"
        shift # past argument
        ;;
        -n|--num_circuits)
        NUM_CIRCUITS="$2"
        shift # past argument
        ;;
        -e|--parallel_executions)
        PARALLEL_EXEC="$2"
        shift # past argument
        ;;
        -ip_const|--ip_constructor)
        IP_CONST="$2"
        shift # past argument
        ;;
        -p_const|--port_constructor)
        PORT_CONST="$2"
        shift # past argument
        ;;
        -i|--interactive)
        INTERACTIVE="$2"
        shift
        ;;
        -t|--test)
        TEST="$2"
        shift
        ;;
        *)
        echo "UNKNOWN OPTION: ${key}"
        ;;
    esac

    shift # past argument or value
done

echo "${TEST}"

if [ "${TEST}" -eq "1" ];
then
    for i in 1 10 100 500
    do
        for j in 1 2 3 4 5 6 7 8 9 10
        do
            FILE="circuits/cb/conditional_swap_shuffle${i}.wir.GC_duplo"

            echo "${i}: ${j}: ${FILE}"

            if [[ "${PROFILE}" = "const" ]]; then
                ./build/release/PokerConstructor -f "${FILE}" -n "${i}" -e "${PARALLEL_EXEC}" -ip_const "${IP_CONST}" -p_const "${PORT_CONST}" -i 0
            elif [[ "${PROFILE}" = "eval" ]]; then
                ./build/release/PokerEvaluator  -f "${FILE}" -n "${i}" -e "${PARALLEL_EXEC}" -ip_const "${IP_CONST}" -p_const "${PORT_CONST}" -i 0
            fi
        done
    done
else
    if [[ "${PROFILE}" = "const" ]]; then
        ./build/release/PokerConstructor -f "${FILE}" -n "${NUM_CIRCUITS}" -e "${PARALLEL_EXEC}" -ip_const "${IP_CONST}" -p_const "${PORT_CONST}" -i "${INTERACTIVE}"
    elif [[ "${PROFILE}" = "eval" ]]; then
        ./build/release/PokerEvaluator  -f "${FILE}" -n "${NUM_CIRCUITS}" -e "${PARALLEL_EXEC}" -ip_const "${IP_CONST}" -p_const "${PORT_CONST}" -i "${INTERACTIVE}"
    fi
fi

echo "${PROFILE} ms"
cat "${PROFILE}_ms.log"
echo ""


echo "${PROFILE} kb"
cat "${PROFILE}_kb.log"