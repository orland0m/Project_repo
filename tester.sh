#!/usr/local/cs/bin/bash

rm -r Project_repo
git clone git@github.com:orland0m/Project_repo.git &&
cd Project_repo &&
g++ tester.cc cache.cc connection-handler.cc http-headers.cc \
    http-request.cc http-response.cc web-request.cc -o _testingphase &&
./_testingphase