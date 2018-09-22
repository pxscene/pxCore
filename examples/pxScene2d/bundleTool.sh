#!/bin/sh
entryfile=""
outputfile="index.js"
enableJar=0
parsinginput=0
inputpassed=0

while getopts ":e:o:j:h" opt; do
  case $opt in
    e) entryfile="$OPTARG"; inputpassed=1;
    ;;
    o) outputfile="$OPTARG"
    ;;
    j) enableJar="$OPTARG"
    ;;
    h) echo "Usage : ./tool.sh -e <entry filename> -o <output filename(optional)> -j 1/0(enable/disable jar)" >&2; exit 1;
    ;;
    \?) echo "Invalid option -$OPTARG"; echo "Usage : ./tool.sh -e <entry filename> -o <output filename(optional)> -j 1/0(enable/disable jar)" >&2; >&2; exit 1;
    ;;
  esac
done

if [ "$inputpassed" -eq 0 ]; then
  echo "Entry file not provided"
  exit 1;
fi

webpack -o "dist/$outputfile"

#sed -i 's/function(module, exports, __webpack_require__) {/function(module, exports, __webpack_require__) {\n px.registerCode(__webpack_require__.m)\n/g' dist/$outputfile


linenumbers=`grep -rn "^module.exports = __webpack_require__" dist/$outputfile|awk -F: '{print $1}'`
delcmd="sed -i '"
for number in $linenumbers
do
delcmd=$delcmd" $number d;"
done
delcmd=$delcmd"' dist/$outputfile"
eval "$delcmd"

linenumbers=`grep -rn "^__webpack_require__(" dist/$outputfile|grep -v "./pack/$entryfile"|awk -F: '{print $1}'`
delcmd="sed -i '"
for number in $linenumbers
do
delcmd=$delcmd" $number d;"
done
delcmd=$delcmd"' dist/$outputfile"
eval "$delcmd"
#sed -i "$number"d dist/$outputfile

sed -i 's/\.\/pack\///g' dist/$outputfile
sed -i "s/__webpack_require__(\/\*! $entryfile \*\/\\\"$entryfile\\\");/px.registerCode(__webpack_require__.m)\n__webpack_require__(\/\*! $entryfile \*\/\\\"$entryfile\\\");/g" dist/$outputfile
#this needs to be done for only parts created for spark code
sed -i 's/function(module, exports)/function(module=px.module, exports=px.exports)/g' dist/$outputfile

mv dist/pack/* dist/.
rm -rf dist/pack
cp FreeSans.ttf dist/.

if [ "$enableJar" -eq 1 ]; then
  echo "{\"main\" : \"$outputfile\"}" > dist/package.json
  cd dist
  jar cvf bundle.jar *
  echo "Output bundle.jar generated within dist !!!!!!!!!!!!!"
else
  echo "Output file $outputfile generated within dist !!!!!!!!!!!!!"
fi

