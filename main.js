import compiler from "./compiler.js"

const editor = document.getElementById("editor");

let timeoutId = 0;
editor.oninput = () => {
	clearTimeout(timeoutId);
	timeoutId = setTimeout(() => compile(editor.value), 150);
}

const test_cases = [
	['0;', 0],
	['42;', 42],
	['5+20-4;', 21],
	[' 12     + 34   - 5;', 41],
	['5+6*7;', 47],
	['-10+20;', 10],
	['- -10;', 10],
	['- - +10;', 10],
	['27 == 27;', 1],
	['1 != 32;', 1],
	['2 * 50   >=    200 / 2   ;', 1],
	['2 > 1;', 1],
	['(2 > 1) * 8 \n;', 8],
	['- 1 == - 1;', 1],
	['0 >= - 1;', 1],
	['-1 > -129;', 1],
	['1; 2;', 2],
];

async function compile(value) {
	const start = Date.now();
	const encoder = new TextEncoder('utf-8');
	const addr = compiler.get_mem_addr();

	const view = new Uint8Array(compiler.memory.buffer, addr, value.length + 1);
	encoder.encodeInto(value, view);
	view[value.length] = 0;

	const len = compiler.compile();
	console.log("Compilation to WebAssembly -- %dms", Date.now() - start);
	if (len == 0) {
		console.log("== Compilation Failed == ");
		return;
	}
	const view2 = new Uint8Array(compiler.memory.buffer, compiler.get_compiled_code(), len);
	console.log(view2);

	const { instance } = await WebAssembly.instantiate(view2);
	console.log("WebAssembly to x86 -- %dms", Date.now() - start);
	let returnValue = instance.exports.main();
	console.log("WebAssembly Runtime -- %dms", Date.now() - start);
	console.log("== Compilation Successful == ");
	console.log(returnValue);
	return returnValue;
}

void async function() {
	let i;
	for (i = 0; i < test_cases.length; ++i) {
		let compile_result = null;
		try {
			compile_result = await compile(test_cases[i][0]);
		} catch (e) {
			console.log("== Failed Test Case ==\nCompiler generated invalid code");
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
