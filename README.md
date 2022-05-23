# CCF C++ App Template

Template repository for CCF C++ applications.

## Build

First, install CCF:

```
$ wget https://github.com/microsoft/CCF/releases/download/ccf-2.0.0/ccf_2.0.0_amd64.deb
$ sudo dpkg -i ccf_2.0.0_amd64.deb
$ cat /opt/ccf/share/VERSION_LONG
ccf-2.0.0
```

Then, in the local checkout:

```bash
$ mkdir build
$ cd build
$ cmake -GNinja ..
$ ninja
```
