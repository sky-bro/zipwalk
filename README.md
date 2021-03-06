# zipwalk

![cmake status](https://github.com/sky-bro/zipwalk/actions/workflows/cmake.yaml/badge.svg)

A CLI tool to parse damaged zip files.

## Build and install

```shell
mkdir build
cd build
# default value of ${CMAKE_INSTALL_PREFIX} is "/usr/local"
cmake ../src # cmake -DCMAKE_INSTALL_PREFIX=~/.local/ ../src/
make # compile
make install # install zipwalk to ${CMAKE_INSTALL_PREFIX}/bin/zipwalk
```

> to uninstall, just delete the file: `rm zipwalk`

### Arch User

```shell
yay -Sy zipwalk
```

## Run

* `zipwalk`: `./zipwalk -f ../samples/123.zip`
* `test-my_inflate`: `./test/test-my_inflate ../samples/123.deflate 123.txt`

## getopt

* ./zipwalk
* `-t` `--target_directory`, where to put the extracted files
* `-f` `--file` zipfile path
* `-v` `--version` print version number
* `-h` `--help` print usage
* `-s` `--save-file` save files
* `-e` `--extension` parse file as a zip/gzip/tar, ignore the suffix of filename

## Thinking / TODO

* [x] in local file header, filename: folders? files? -- check whether ends with '/' or not
* date & time
* [x] base folder
* [ ] zip32/zip64
* [x] getbyte, getword, getdword, getqword(little endian ?)
* human readable? -h
* [x] mkdir, recursively
* [x] file / folder
* other compression method
  * [x] stored
  * [x] deflate
* [ ] summary info
* [ ] encrypted ?
* [ ] .gz/.tar.gz files
  * determine format of a file by its filename
  * or one more argument `-e` `--extension` zip, tar, gzip
  * `.tar.gz` `.tgz` `.gz`: do un-gzip, save file to `.tar`
  * `.tar`: do un-tar

## Refs

* [wiki: ZIP (file format)](https://en.wikipedia.org/wiki/ZIP_(file_format))
* [ZIP压缩算法详细分析及解压实例解释](https://www.cnblogs.com/esingchan/p/3958962.html) 详细，从哈夫曼到deflate
* [zip文件结构](https://www.jianshu.com/p/717373dd3188)
* [zip文件解析](https://www.jianshu.com/p/e42b94967503)
* [rfc1951](http://tools.ietf.org/html/rfc1951) (*deflate* format)
