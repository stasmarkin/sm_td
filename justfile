set dotenv-load
set shell := ["bash", "-euo", "pipefail", "-c"]

# QMK checkout directory. Override via SMTD_QMK_DIR in .env or environment.
export SMTD_QMK_DIR := env_var_or_default("SMTD_QMK_DIR", justfile_directory() / "tests/integration/checkouts")

# Pinned QMK versions
QMK_DEFAULT_VERSION := "0.33.5"
QMK_VERSIONS        := "0.33.5"

# All QMK native test suites
QMK_ALL_SUITES := "smtd_qmk_taphold smtd_qmk_taphold_no_action_tapping smtd_full smtd_dynamic smtd_dynamic_fixed smtd_dynamic_clamp smtd_caps_word smtd_external_mods"

# Show available recipes
default:
    @just --list

# Configure local test settings (QMK checkout directory)
configure:
    #!/usr/bin/env bash
    echo "=== sm_td test configuration ==="
    echo ""
    echo "QMK checkout directory (where QMK firmware is downloaded for testing)."
    echo "Current: $SMTD_QMK_DIR"
    echo ""
    read -rp "New path (Enter to keep current): " dir </dev/tty
    if [ -n "$dir" ]; then
        mkdir -p "$dir"
        dir="$(cd "$dir" && pwd)"
    else
        dir="$SMTD_QMK_DIR"
    fi
    if [ -f .env ] && grep -q "^SMTD_QMK_DIR=" .env; then
        sed -i.bak "s|^SMTD_QMK_DIR=.*|SMTD_QMK_DIR=$dir|" .env && rm -f .env.bak
    else
        echo "SMTD_QMK_DIR=$dir" >> .env
    fi
    echo "Saved to .env: SMTD_QMK_DIR=$dir"

# Run tests
#   just test                    — all (python + qmk native)
#   just test python             — python unit tests
#   just test qmk                — all qmk suites, default version
#   just test qmk full           — smtd_full suite
#   just test qmk 0.32.16       — all suites, QMK 0.32.16
#   just test qmk 0.32.16 full  — version + suite
#   just test qmk-matrix        — all pinned versions × all suites
test *args:
    #!/usr/bin/env bash
    args=({{args}})

    _ensure_qmk_dir() {
        if [ -f .env ] && grep -q "^SMTD_QMK_DIR=" .env 2>/dev/null; then
            return
        fi
        echo ""
        echo "=== QMK checkout directory ==="
        echo "Native integration tests need QMK firmware (~2GB per version)."
        echo ""
        local default="$SMTD_QMK_DIR"
        read -rp "Download path [$default]: " dir </dev/tty
        dir="${dir:-$default}"
        mkdir -p "$dir"
        dir="$(cd "$dir" && pwd)"
        echo "SMTD_QMK_DIR=$dir" >> .env
        echo "Saved to .env (change later with: just configure)"
        echo ""
        export SMTD_QMK_DIR="$dir"
    }

    _suite() {
        local s="$1"
        [[ "$s" == smtd_* ]] && echo "$s" || echo "smtd_$s"
    }

    _run_qmk() {
        local version="$1"; shift
        local suites=("$@")
        _ensure_qmk_dir
        sh tests/integration/run.sh "$version" "${suites[@]}"
    }

    if [ ${#args[@]} -eq 0 ]; then
        just test python
        just test qmk
        exit 0
    fi

    case "${args[0]}" in
        python)
            python3 tests/run_tests.py
            ;;

        qmk-matrix)
            for ver in {{QMK_VERSIONS}}; do
                echo ""
                echo "=========================================="
                echo "  QMK $ver"
                echo "=========================================="
                just test qmk "$ver"
            done
            ;;

        qmk)
            version="{{QMK_DEFAULT_VERSION}}"
            suites=()
            for arg in "${args[@]:1}"; do
                if [[ "$arg" =~ ^[0-9]+\.[0-9]+ ]]; then
                    version="$arg"
                else
                    suites+=("$(_suite "$arg")")
                fi
            done
            if [ ${#suites[@]} -eq 0 ]; then
                suites=({{QMK_ALL_SUITES}})
            fi
            _run_qmk "$version" "${suites[@]}"
            ;;

        *)
            echo "Usage: just test [python|qmk|qmk-matrix] [version] [suite...]"
            echo ""
            echo "Targets:"
            echo "  python       Python unit tests (ctypes-based)"
            echo "  qmk          QMK native integration tests (googletest)"
            echo "  qmk-matrix   Run qmk tests against all pinned versions"
            echo ""
            echo "QMK options:"
            echo "  VERSION      QMK tag/commit (default: {{QMK_DEFAULT_VERSION}})"
            echo "  SUITE        Suite name, with or without smtd_ prefix"
            echo ""
            echo "Examples:"
            echo "  just test                    # all"
            echo "  just test python             # python only"
            echo "  just test qmk                # all qmk suites"
            echo "  just test qmk full           # smtd_full suite"
            echo "  just test qmk 0.32.16        # specific QMK version"
            echo "  just test qmk 0.32.16 full   # version + suite"
            echo "  just test qmk-matrix         # version matrix"
            exit 1
            ;;
    esac

# Fetch QMK firmware without running tests
fetch-qmk version=QMK_DEFAULT_VERSION:
    sh tests/integration/fetch.sh "{{version}}"
