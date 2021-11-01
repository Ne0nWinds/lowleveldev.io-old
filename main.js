import compiler from "./compiler.js"

const editor = document.getElementById("editor");

let timeoutId = 0;
editor.oninput = () => {
	clearTimeout(timeoutId);
	timeoutId = setTimeout(() => compile(editor.value), 150);
}

const test_cases = [
	['0', 0],
	['42', 42],
	['5+20-4', 21],
	[' 12     + 34   - 5', 41],
	['5+6*7', 47],
	['-10+20', 10],
	['- -10', 10],
	['- - +10', 10],
	['27 == 27', 1],
	['1 != 32', 0],
	['2 * 50   >=    200 / 2   ', 1],
	['2 > 1', 1],
	['(2 > 1) * 8 \n', 8],
	['- 1 == - 1', 1],
	['0 >= - 1', 1],
	['-1 > -129', 1],
];

async function compile(value) {
	const encoder = new TextEncoder('utf-8');
	const addr = compiler.get_mem_addr();

	const view = new Uint8Array(compiler.memory.buffer, addr, value.length + 1);
	encoder.encodeInto(value, view);
	view[value.length] = 0;

	const len = compiler.compile();
	if (len == 0) {
		console.log("== Compilation Failed == ");
		return;
	}
	const view2 = new Uint8Array(compiler.memory.buffer, compiler.get_compiled_code(), len);
	console.log(view2);

	const { instance } = await WebAssembly.instantiate(view2);
	let returnValue = instance.exports.main();
	console.log("== Compilation Successful == ");
	console.log(returnValue);
	return returnValue;
}

void async function() {
	for (let i = 0; i < test_cases.length; ++i) {
		if (await compile(test_cases[i][0]) != test_cases[i][1]) {
			console.log("== Failed Test Case ==\n'%s' should return %d", test_cases[i][0], test_cases[i][1]);
			break;
		}
	}
	console.clear();
	console.log("== All Test Cases Passed ==");
}();
