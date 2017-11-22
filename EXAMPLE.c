/*
  COMPILE TO NATIVE EXECUTABLE USING:

    gcc -o EXAMPLE EXAMPLE.c -O3 -Wall

  Run from command line:

    ./EXAMPLE

  Run from command line and only see output to stderr (tested in bash):

    ./EXAMPLE 2>&1 >/dev/null

  -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

  COMPILE TO WEBASSEMBLY WITH:

    emcc -o EXAMPLE.js EXAMPLE.c -O3 -s WASM=1 -s NO_EXIT_RUNTIME=1 \
    -s TOTAL_MEMORY=81920000

  Replace 81920000 (78.1M) with the number of bytes the application plans to
  allocate on the heap (e.g. with malloc()).
  Can also omit the option or replace with the following flag to allow heap
  growth:

    -s ALLOW_MEMORY_GROWTH=1

  The number of bytes must be a multiple of 64K (page size).
  (Otherwise, the compiler will give an error stating this same requirement.)

  -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

  LINK WEBASSEMBLY WITH EXAMPLE.HTML BY:

  (1) Placing EXAMPLE.js and EXAMPLE.wasm in the same directory as
      EXAMPLE.html.

  (2) Inserting the following code to EXAMPLE.html (e.g. between the
      <head>...</head> tags):

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

  (3) Start a local HTTP server in the same directory, for instance, with the
      following command:

    python3 -m http.server 8000

  (4) Open EXAMPLE.html in the browser with the URL:

    http://localhost:8000/EXAMPLE.html

      (The capitalization matters on Linux.)

  -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

  RUN EXPORTED FUNCTIONS IN JAVASCRIPT:

    var result = Module.ccall (
      'my_function',  // name of C function
       null,          // return type
       null,          // argument types
       null           // arguments
    );

  (Exported functions are those those annotated with the "EXPORT" macro in
  EXAMPLE.c and main().)

  -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

  EXAMPLE OF TRANSFERRING MEMORY TO THE HEAP IN JAVASCRIPT:

    // Get a TypedArray() in javscript.
    var size = 640*480*30;
    var myTypedArray = new Float32Array(size);

    // Allocate memory on the heap.
    var bytes_needed = myTypedArray.BYTES_PER_ELEMENT * myTypedArray.length;
    var heapBuffer = Module._malloc (bytes_needed);

    // Initialize array.
    for (let i=0; i<size; i++)
      myTypedArray[i] = i;

    // Copy the TypedArray to heap-allocated memory.
    // Note: Pointers are implemented as indices to bytes in the heap.
    // The set() function indexes this same heap but views the heap as
    // an array of the prefixed data (in this case Float32).
    // Hence we adjust the heapBuffer "pointer" to compensate for this
    // different "view" of the heap by dividing by
    // myTypedArray.BYTES_PER_ELEMENT.

    // See another explanation at:
    //   https://github.com/kripken/emscripten/issues/4003
    var adjusted_pointer = heapBuffer/myTypedArray.BYTES_PER_ELEMENT;
    Module.HEAPF32.set(myTypedArray, adjusted_pointer);

    // Pass a pointer to heap-allocated memory to the function.
    var result = Module.ccall(
      'my_function2',
      'number',
      ['number', 'number'],
      [heapBuffer, size]
    );

    // Free memory from the heap.
    Module._free(heapBuffer);

  See a very similar example in EXAMPLE.html in the my_function2() javscript
  function.

  -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

  HELPFUL LINKS:

  Emscripten Documentation: Connecting C++ with Javascript: Interacting with code:
    https://kripken.github.io/emscripten-site/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html

  Mozilla Developer Network: WebAssembly: Compiling from C/C++ to WebAssembly:
    https://developer.mozilla.org/en-US/docs/WebAssembly/C_to_wasm
*/


#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define EXPORT
#endif

#ifdef __cplusplus
#define DEMANGLE extern "C" {
#define END_DEMANGLE }
#else
#define DEMANGLE
#define END_DEMANGLE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


DEMANGLE
/* Prints a message to stderr. */
EXPORT
void my_function () {
  fprintf(stderr, "my_function() called. outputting to stderr.\n");
}


/* Calculates average, subject to floating point rounding errors. */
EXPORT
float my_function2 (float* array, int length) {
  printf("Entered my_function\n");
  clock_t start;
  double elapsed;
  start = clock();
  float result = 0;

  for (int i=0; i<length; i++)
  {
    if (i%(length/5)==0)
      printf("Result so far: %.4f\n", result);
    result += array[i] / length;
  }

  elapsed = ((double) clock() - start) / CLOCKS_PER_SEC;
  printf("Leaving my_function -- elapsed: %.2f sec\n", elapsed);
  return result;
}
END_DEMANGLE


/* Automatically exported. */
int main (int argc, char** argv) {
  printf ("Hello World!\n");

  my_function ();

  int size = 640*480*30*2;
  float* array = malloc (sizeof(float) * size);
  for (int i=0; i<size; i++)
    array[i] = i;
  float result = my_function2 (array, size);
  printf ("Result: %.2f\n", result);
  free (array);
}
