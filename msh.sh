#!/bin/bash

set -e

# ---- Helper: go to minishell root ----
go_to_root() {
    while [ "$(basename "$(pwd)")" != "minishell" ]; do
        if [ "$(pwd)" = "/" ]; then
            echo "Reached filesystem root — minishell not found."
            exit 1
        fi
        cd ..
    done
}

# ---- Build functions ----
build_parser() {
    (cd src/parser && make "$@")
}

demo_parser() {
    (cd src/parser && make demo "$@")
}

clean_parser() {
    (cd src/parser && make clean "$@")
}

build_main() {
    (cd src && make "$@")
}

clean_main() {
    (cd src && make clean "$@")
}

build() {
    (make "$@")
}

run_parser() {
    ./build/bin/parser/parserdemo
}

run_minishell() {
    ./build/bin/minishell
}

# ---- Dispatch ----
cmd="$1"
shift || true

case "$cmd" in
    build_parser)   build_parser "$@" ;;
    demo_parser)    demo_parser "$@" ;;
    clean_parser)   clean_parser "$@" ;;
    build_main)     build_main "$@" ;;
    build)          build "$@" ;;
    clean_main)     clean_main "$@" ;;
    run_parser)     run_parser "$@" ;;
    run_minishell)  run_minishell "$@" ;;
    *)
        echo "Unknown command: $cmd"
        exit 1
        ;;
esac

# Always return to root at the end
go_to_root