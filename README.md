# lowleveldev.io
This is the future home of a low level programming tutorial series using a custom-built WebAssembly C compiler

## How to build (Windows):
- Open powershell in project directory and `cd .\compiler\`
- Run `.\compile.ps1 -debug $true`
- Open a second powershell window in project directory
- Run `python -m http.server`
- Open [localhost:8000](http://localhost:8000/) in a browser
- The code typed into the textbox will be compiled into WebAssembly + output to the js dev tools console

## Main Project Goals
- Encourage people to become interested in low-level programming
- Be a free, practical resource (alternative?) for learning Computer Science

## Project Notes
- The main inspiration for this site is [FreeCodeCamp](https://www.freecodecamp.org/), but instead of web technologies, it's about low-level programming skills using C. The final project will be very similar with a structured curriculum, test cases on code, small articles / YouTube videos for background information, etc.
- The repository contains a (soon-to-be) C compiler that outputs WebAssembly. This custom compiler will be for compiling user code to WebAssembly and running test cases against it (similar to the FreeCodeCamp courses). Hosting our own compiler in WebAssembly has a lot of advantages. A site like Codecademy has to maintain VMs for C / C++ compilers, which is expensive, causes extra latency when compiling code, and has lots of security risks. This project will be front-end only (other than maybe saving progress through a login). Thus, avoiding the pitfalls of compilers in VMs
- Speed is another priority, because being able to see the output of your code quickly is really important for learning / experimentation. On a modern desktop / laptop, the compiler should be able to compile to WebAssembly / run test cases as fast as the user can type. On a mobile device / older laptop, the compiler should take no longer than one second. This will be accomplished by having our compiler written in C, compiling directly to binary WebAssembly, being CPU-cache efficient, using little / no dynamic memory allocation, and optimizing with SIMD (when possible). Other things that introduce more complexity but will be considered if I can't hit the goal with other methods are compiling only changes / multiple threads
- Another aspect of speed is page loading time. My goal is to make it a reasonable experience on a mobile phone / tablet, allowing anyone to have access to it. The first advantage is that most of the code is written in WebAssembly. I'm using clang with a WebAssembly backend. Unlike something like emscripten, this does not generate a huge amount of boilerplate / glue code and should keep the download size quite small for the `.wasm` files. I also want to take advantage of modern browser APIs (when available) like PWA service workers to optionally download the curriculum for offline use and very quickly load the site in bad network conditions. Other than that, I plan to use js minification using [Google Closure Compiler](https://developers.google.com/closure/) and WASM minfication using [wasm-opt](https://github.com/WebAssembly/binaryen)

## Other Projects
- Main inspiration [FreeCodeCamp](https://www.freecodecamp.org/)
- Compiler Design similar to [chibicc](https://github.com/rui314/chibicc)
- [Open Source Computer Science Degree](https://github.com/ossu/computer-science)
