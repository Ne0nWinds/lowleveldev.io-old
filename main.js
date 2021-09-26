import compiler from "./compiler.js"

const editor = document.getElementById("editor");

let timeoutId = 0;
editor.onkeydown = () => {
	clearTimeout(timeoutId);
	timeoutId = setTimeout(compile, 150);
}

async function compile() {
	const { value } = editor;
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

	const { instance } = await WebAssembly.instantiate(view2);
	console.log(instance.exports.main());
	console.log("== Compilation Successful == ");
}
