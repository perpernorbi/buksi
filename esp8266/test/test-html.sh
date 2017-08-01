#!/bin/bash

THIS_SCRIPT_DIR="$(dirname "${BASH_SOURCE[0]}")"
HTML_SOURCE_DIR="${THIS_SCRIPT_DIR}/../html"
CGI_SOURCE_DIR="${THIS_SCRIPT_DIR}/cgi-src"
HTML_TARGET_DIR="${THIS_SCRIPT_DIR}/html"
SED_TEMPLATE="${SED_TEMPLATE-"${THIS_SCRIPT_DIR}/template.sed"}"

rm -rf "${HTML_TARGET_DIR}"
mkdir -p "${HTML_TARGET_DIR}"

cp -r "${HTML_SOURCE_DIR}/"* "${HTML_TARGET_DIR}"
cp -r "${CGI_SOURCE_DIR}/"* "${HTML_TARGET_DIR}"
find "${HTML_TARGET_DIR}" -name "*.tpl" -exec sed -f "${SED_TEMPLATE}" -i {} \;
find "${HTML_TARGET_DIR}" -name "*.js" -exec sed -f "${SED_TEMPLATE}" -i {} \;
find "${HTML_TARGET_DIR}" -name "*.tpl" -exec rename 's/tpl$/html/' {} \;


pushd "${HTML_TARGET_DIR}"
python -m SimpleHTTPServer 8080 &
SERVER_PID=$!
trap "kill ${SERVER_PID}" SIGINT SIGTERM
popd

inotifywait --format %w%f ${HTML_SOURCE_DIR} -r -m -e close_write,move | while read source; do 
    target="${HTML_TARGET_DIR}${source#${HTML_SOURCE_DIR}}"
    if [[ "${source}" =~ .*\.tpl ]]; then
        echo Rendering template ${source} to ${target%.*}.html
        sed -f "${SED_TEMPLATE}" ${source} > ${target%.*}.html
    else if [[ "${source}" =~ .*\.js ]]; then
            echo Rendering javascript ${source} to ${target}
            sed -f "${SED_TEMPLATE}" ${source} > ${target}
        else
            echo Copying ${source} to ${target}
            cp ${source} ${target}
        fi
    fi
done
