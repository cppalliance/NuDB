#!/usr/bin/env bash

set -euxo pipefail
# The above bash options do the following:

# -e When this option is on, if a simple command fails for any of the reasons
#    listed in Consequences of Shell Errors or returns an exit status value >0,
#    and is not part of the compound list following a while, until, or if
#    keyword, and is not a part of an AND or OR list, and is not a pipeline
#    preceded by the ! reserved word, then the shell shall immediately exit.
# -u The shell shall write a message to standard error when it tries to expand a
#    variable that is not set and immediately exit. An interactive shell shall
#    not exit.
# -x The shell shall write to standard error a trace for each command after it
#    expands the command and before it executes it. It is unspecified
#    whether the command that turns tracing off is traced.
# -o pipefail
#    Pipelines fail on the first command which fails instead of dying later on
#    down the pipeline.

shopt -s globstar

################################## ENVIRONMENT #################################

# If not CI, then set some defaults
if [[ -z ${CI:-} ]]; then
  : ${TRAVIS_BRANCH:=feature}
  : ${CC:=gcc}
  : ${VARIANT:=debug}
  # If running locally we assume we have lcov/valgrind on PATH
fi
: ${WITH_VALGRIND:=0}

MAIN_BRANCH=0
# For builds not triggered by a pull request TRAVIS_BRANCH is the name of the
# branch currently being built; whereas for builds triggered by a pull request
# it is the name of the branch targeted by the pull request (in many cases this
# will be master).
if [[ ${TRAVIS_BRANCH} == master || ${TRAVIS_BRANCH} == develop ]]; then
    MAIN_BRANCH=1
fi

num_jobs=2

echo "using toolset: ${CC}"
echo "using variant: ${VARIANT}"
echo "using PATH: ${PATH}"
echo "using MAIN_BRANCH: ${MAIN_BRANCH}"
echo "using BOOST_ROOT: ${BOOST_ROOT}"

#################################### HELPERS ###################################

function run_tests_with_debugger {
  for x in bin/**/${VARIANT}/**/test-all; do
    scripts/run-with-debugger.sh "${x}"
  done
}

function run_tests {
  for x in bin/**/${VARIANT}/**/test-all; do
    ${x}
  done
}

function run_benchmark {
  for x in bin/**/${VARIANT}/**/bench; do
    ${x} --batch_size=10000 --num_batches=100
  done
}

function run_tests_with_valgrind {
  if ! command -v valgrind >/dev/null 2>&1 ; then
    if [[ ! -z ${VALGRIND_ROOT:-} ]]; then
      export PATH=${VALGRIND_ROOT}/bin:${PATH}
    fi
  fi
  for x in bin/**/${VARIANT}/**/test-all; do
    valgrind \
      --error-exitcode=1 \
      --leak-check=full \
      --track-origins=yes \
      --suppressions=scripts/valgrind.supp \
      --gen-suppressions=all \
      "${x}"
  done
}

: ${ADDRESS_MODEL:=64}
function build_bjam {
  echo "using address-model: ${ADDRESS_MODEL}"
  ${BOOST_ROOT}/bjam toolset=${CC} \
    variant=${VARIANT} \
    address-model=${ADDRESS_MODEL} \
    -j${num_jobs}
}

: ${CMAKE_ROOT:="${HOME}/cmake"}
: ${CMAKE_EXTRA_OPTS:=""}
function build_cmake {
  exeperms=$(test $(uname) = "Linux" && echo "/111" || echo "+111")
  mkdir -p build
  pushd build > /dev/null
  gen=$(command -v ninja >/dev/null && echo "Ninja" || echo "Unix Makefiles")
  eval cmake -DVARIANT=${VARIANT} -G"${gen}" -DCMAKE_VERBOSE_MAKEFILE=ON ${CMAKE_EXTRA_OPTS} ..
  cmake --build . -j ${num_jobs} --verbose
  mkdir -p ../bin/${VARIANT}
  find . -perm ${exeperms} -type f -exec cp {} ../bin/${VARIANT}/. \;
  popd > /dev/null
}

##################################### BUILD ####################################

if [[ ${BUILD_SYSTEM:-} == cmake ]]; then
  build_cmake
else
  build_bjam
fi

##################################### TESTS ####################################

if [[ ${VARIANT} == coverage ]]; then
  if ! command -v lcov >/dev/null 2>&1 ; then
    if [[ ! -z ${LCOV_ROOT:-} ]]; then
      export PATH=${LCOV_ROOT}/usr/bin:${PATH}
    fi
  fi

  find . -name "*.gcda" | xargs rm -f
  rm *.info -f
  # Create baseline coverage data file
  lcov --no-external -c -i -d . -o baseline.info > /dev/null
fi

# Perform test
if [[ ${WITH_VALGRIND} -ge 1 ]]; then
  run_tests_with_valgrind
else
  run_tests
fi

if [[ ${VARIANT} == coverage ]]; then
  # Create test coverage data file
  lcov --no-external -c -d . -o testrun.info > /dev/null
  # Combine baseline and test coverage data
  lcov -a baseline.info -a testrun.info -o lcov-all.info > /dev/null
  # Extract only include/*, and don\'t report on examples or test
  lcov -e "lcov-all.info" "${PWD}/include/nudb/*" -o lcov.info > /dev/null
  lcov --remove "lcov.info" "${PWD}/include/nudb/_experimental/*" -o lcov.info > /dev/null
  ~/.local/bin/codecov -X gcov -f lcov.info
  # cat lcov.info | node_modules/.bin/coveralls
  # Clean up these stragglers so BOOST_ROOT cache is clean
  find ${BOOST_ROOT}/bin.v2 -name "*.gcda" | xargs rm -f
else
  # only run benchmark for non-coverage builds
  run_benchmark
fi

