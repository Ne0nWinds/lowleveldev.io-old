import compiler from "./compiler.js"

const editor = document.getElementById("editor");

let timeoutId = 0;
editor.oninput = () => {
	clearTimeout(timeoutId);
	// compile(editor.value);
	timeoutId = setTimeout(() => compile(editor.value), 650);
}

const test_cases = [
	['{ return 0; }', 0],
	['{ return 42; }', 42],
	['{ return 5+20-4; }', 21],
	['   {  return 12     + 34   - 5; }', 41],
	['{ return 5+6*7; }', 47],
	['{ return -10+20; }', 10],
	['{ return - -10; }', 10],
	['{ return - - +10; }', 10],
	['{ return 27 == 27; }', 1],
	['{ return 1 != 32; }', 1],
	['{ return 2 * 50   >=    200 / 2   ; }', 1],
	['{ return 2 > 1; }', 1],
	['{ return (2 > 1) * 8 \n\n; }', 8],
	['{ return - 1 == - 1; }', 1],
	['{ return 0 >= - 1; }', 1],
	['{ return -1 > -129; }', 1],
	['{ 1; return 2; }', 2],
	['{ a = 3; return a; }', 3],
	['{ a = 15; b = 25 * 3; return b - a + 1; }', 61],
	['{ test_var = 14; return test_var; }', 14],
	['{ var1 = 30; var2 = 32; return var2 - var1; }', 2],
	['{ var1 = 30; var2 = 32; return -(var2 - var1) + 10; }', 8],
	['{ return 1; 2; 3; }', 1],
	['{ 1; return 2; 3; }', 2],
	['{ 1; 2; return 3; }', 3],
	['{ a = 16; b = 2; return (a * b) - 8; 1024 - 67; }', 24],
	['{ {1; {2;} return 3; }}', 3],
	['{ a = 1; }', 0]
];

const encoder = new TextEncoder('utf-8');
const view = new Uint8Array(compiler.memory.buffer, compiler.get_mem_addr(), 1024);

async function compile(value) {
	const start = performance.now();
	encoder.encodeInto(value, view);
	view[value.length] = 0;

	const len = compiler.compile();
	if (len == 0) {
		console.log("== Compilation Failed == ");
		return;
	}

	const view2 = new Uint8Array(compiler.memory.buffer, compiler.get_compiled_code(), len);
	const compilationToWebAssembly = performance.now();
	console.log(view2);

	const { instance } = await WebAssembly.instantiate(view2);
	const webAssemblyToX86 = performance.now();
	let returnValue = instance.exports.main();
	const webAssemblyRuntime = performance.now();
	console.log("Compilation to WebAssembly -- %.3fms", compilationToWebAssembly - start);
	console.log("WebAssembly to x86 -- %.3fms", webAssemblyToX86 - compilationToWebAssembly);
	console.log("WebAssembly Runtime -- %.3fms", webAssemblyRuntime - webAssemblyToX86);
	console.log("Total Time -- %.3fms", webAssemblyRuntime - start);
	console.log("== Compilation Successful == ");
	console.log(returnValue);
	return returnValue;
}

void async function() {
	let i = 0;
	for (; i < test_cases.length; ++i) {
		let compile_result = null;
		try {
			compile_result = await compile(test_cases[i][0]);
		} catch (e) {
			console.log("== Failed Test Case ==\nCompiler generated invalid code or threw an exception");
			console.log(e);
		}
		if (compile_result != test_cases[i][1]) {
			console.log("== Failed Test Case ==\n'%s' should return %d", test_cases[i][0], test_cases[i][1]);
			break;
		}
	}
	if (i == test_cases.length) {
		console.clear();
		console.log("== All Test Cases Passed ==");
	}
}();
