# Tutorial for Compiling C Code to Run on the Web (with Emscripten and WebAssembly)

Simpler and less general than the `shell_minimal.html` file included in `emsdk/emscripten/[version e.g. 1.37.22]/src`.

### Compile C Code to Native Executable

Compile with the following command:

    gcc -o EXAMPLE EXAMPLE.c -O3 -Wall

To run from command line:

    ./EXAMPLE

To run from command line and only see output to standard error (tested in bash):

    ./EXAMPLE 2>&1 >/dev/null

----

### Compile C Code to WebAssembly

    emcc -o EXAMPLE.js EXAMPLE.c -O3 -s WASM=1 -s NO_EXIT_RUNTIME=1 -s TOTAL_MEMORY=[BYTES]

Replace `[BYTES]` with the number of bytes the application plans to
allocate on the heap (e.g. with `malloc()`).
For this template application `[BYTES]` should be `81920000` (78.1MB).
Can also omit the `-s TOTAL_MEMORY` option or replace with the following flag to allow heap growth:

    -s ALLOW_MEMORY_GROWTH=1

The number of bytes must be a multiple of 64K (page size).
(Otherwise, the compiler will give an error stating this same requirement.)

----

### Link WebAssembly with Webpage

1. Placing `EXAMPLE.js` and `EXAMPLE.wasm` in the same directory as
    `EXAMPLE.html`.

2. Inserting the following code to `EXAMPLE.html` (e.g. between the
    `<head>...</head>` tags):

```javascript
<script type="text/javascript">
var Module =
{
  preRun: [],                 // javascript functions to run before main ()
  onRuntimeInitialized: null, // functions run after preRun, before main()
  postRun: [],                // functions run after main ()
  print: (text) => console.log (text),  // function to handle TEXT output
                                        // to stdout by EXAMPLE.c
  printErr: (text) => console.warn (text),  // function handle for stderr
};
</script>
<script async type="text/javascript" src="EXAMPLE.js"></script>
```

3. Start a local HTTP server in the same directory, for instance, with the
   following command:

        python3 -m http.server 8000

4. Open `EXAMPLE.html` in the browser with the URL:

        http://localhost:8000/EXAMPLE.html

(The capitalization matters on Linux.)

----

### Run Exported WebAssembly Functions in Javascript

```javascript
  var result = Module.ccall (
    'my_function',  // name of C function
     null,          // return type
     null,          // argument types
     null           // arguments
  );
```

(Exported functions are those those annotated with the `EXPORT` macro in
`EXAMPLE.c` and `main()`.)

----

### Example of Transferring Memory to WebAssembly Heap in Javascript

```javascript
// Get a TypedArray() in javscript.
var length = 640*480*30*2;
var bytes_per_element = 4;

// Allocate memory on the heap.
var bytes_needed = bytes_per_element * length;
var heapBuffer = Module._malloc (bytes_needed);

// Create TypedArray() with underlying buffer from emscripten heap.
var myTypedArray = new Float32Array (Module.HEAPF32.buffer, heapBuffer, length);

// Initialize array.
for (let i=0; i<length; i++)
  myTypedArray[i] = i;

// Pass a pointer to heap-allocated memory to the function.
var result = Module.ccall(
  'my_function2',
  'number',
  ['number', 'number'],
  [heapBuffer, length]
);

// Free memory from the heap.
Module._free(heapBuffer);
```

See a very similar example in `EXAMPLE.html` in the `my_function2()` javscript
function.

----

### Other Helpful Links:

* [Emscripten Documentation > Connecting C++ with Javascript > Interacting with code](https://kripken.github.io/emscripten-site/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html)

* [Mozilla Developer Network > WebAssembly > Compiling from C/C++ to WebAssembly](https://developer.mozilla.org/en-US/docs/WebAssembly/C_to_wasm)
