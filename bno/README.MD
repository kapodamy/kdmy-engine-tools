# Binary Notation Object

BNO encoder and decoder sourcecode.

---

## About

The BNO file contains the JSON or XML contents in binary form, this avoids parsing and allocating resources.
A BNO is loaded directly in the RAM without any parsing, only a integrity check is required. Traversing between entries/nodes is done by seeking and does not require string allocation.
This format is similar to [BSON](https://wikipedia.org/wiki/BSON "BSON - Wikipedia") and [Binary XML](https://wikipedia.org/wiki/Binary_XML "Binary XML - Wikipedia") but simpler.

__Note:__ is recommended use \*.jbno for JSON files and \*.bno for XML files.

### Usage

```text
BNO encoder/decoder v0.1 by kapodamy
Encodes XML and JSON files into BNO (binary object notation) format and viceversa.

Usage:
    Encode:
        bno.exe [encode options...] <input xml file> <output bno file>
        bno.exe <input json file> <output jbno file>
    Decode:
        bno.exe <input jbno file> <output json file>
        bno.exe <input bno file> <output xml file>
Encode XML options:
 -w, --whitelist <tag names>         Nodes to allow whitespaced text, each tag name is separated by ':' character
 -b, --blacklist <tag names>         Nodes to disallow whitespaced text, each tag name is separated by ':' character
 -n, --no-spaces                     Reject all whitespaced text nodes
```

---

## Building

### debug

- x86 `make build-x86-debug-windows`
- x64 `make build-x64-debug-windows`

### release

- x86 `make build-x86-release-windows`
- x64 `make build-x64-release-windows`

---

### Building for 32-bits

__Only x64 DLLs are included__. For x86 replace the DLLs in `/lib` folder and headers in `/include` with the proper files.
