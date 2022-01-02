#!/bin/bash
set -e
set -o pipefail

mypath=$(dirname $(readlink -f $0))
url="https://raw.githubusercontent.com/cee-studio/cee-utils/master"

wget --no-cache $url/scripts/get-cee-utils.sh -O ${mypath}/get-cee-utils.sh
chmod +x ${mypath}/get-cee-utils.sh

list="README.md
debug.h
cee-sqlite3.h
cee-sqlite3.c
json-actor-boxed.c
json-actor-boxed.h
json-actor.c
json-actor.h
json-parser.c
json-printf.c
json-scanf.c
json-scanf.h
json-struct.c
log.c
log.h
logconf.c
logconf.h
ntl.c
ntl.h
cee-utils.c
cee-utils.h
third-party/jsmn.h
third-party/cJSON.c
third-party/cJSON.h
third-party/json-string.c
third-party/url-encode.h
third-party/cee-data-sizes.h
third-party/greatest.h
third-party/utf8.h
third-party/utarray.h
third-party/uthash.h
third-party/utlist.h
third-party/utringbuffer.h
third-party/utstack.h
third-party/utstring.h
third-party/HttpStatusCodes_C.h
"

mkdir -p $mypath/../cee-utils
pushd $mypath/../cee-utils
for i in $list; do
    echo "getting $i"
    echo "$url/$i"
    wget --no-cache $url/$i -O $i
done
popd
