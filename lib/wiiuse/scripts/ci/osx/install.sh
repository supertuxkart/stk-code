#!/bin/bash
set -ev
unset -f cd # See https://github.com/travis-ci/travis-ci/issues/8703

brew update
brew ls --versions cmake && brew upgrade cmake || brew install cmake # Install or Update cmake (if required)
